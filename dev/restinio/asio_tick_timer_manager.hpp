/*
	restinio
*/

/*!
	Timer factory implementation using asio timers.
*/

#pragma once

#include <memory>
#include <chrono>
#include <unordered_map>

#include <asio.hpp>

#include <restinio/timer_common.hpp>

namespace restinio
{

//
// asio_tick_timer_manager_t
//

//! Timer factory implementation using asio timers.
template < typename Executor = asio::strand< asio::executor > >
class asio_tick_timer_manager_t
	:	public std::enable_shared_from_this< asio_tick_timer_manager_t< Executor > >
{
	public:
		struct factory_t
		{
			const std::chrono::steady_clock::duration m_tick;
			const std::chrono::steady_clock::duration m_check_period;
			const std::size_t m_initial_buckets;

			factory_t(
				std::chrono::steady_clock::duration tick = std::chrono::seconds( 1 ),
				std::chrono::steady_clock::duration check_period = std::chrono::seconds( 1 ),
				std::size_t initial_buckets = 64 )
				:	m_tick{ check_period }
				,	m_check_period{ check_period }
				,	m_initial_buckets{ initial_buckets }
			{}

			auto
			create( asio::io_context & io_context ) const
			{
				return
					std::make_shared< asio_tick_timer_manager_t >(
						io_context,
						m_tick,
						m_check_period,
						m_initial_buckets );
			}
		};

		asio_tick_timer_manager_t(
			asio::io_context & io_context,
			std::chrono::steady_clock::duration tick,
			std::chrono::steady_clock::duration check_period,
			std::size_t initial_buckets )
			:	m_tick_timer{ io_context }
			,	m_strand{ io_context.get_executor() }
			,	m_tick{ std::move( tick ) }
			,	m_check_period{ std::move( check_period ) }
			,	m_timers{ initial_buckets }
		{}

		//! Timer guard for async operations.
		class timer_guard_t final
		{
			public:
				timer_guard_t( std::shared_ptr< asio_tick_timer_manager_t > timer_context )
					:	m_timer_manager{ std::move( timer_context ) }
				{}

				// Guard operation.
				void
				schedule_timeout_check_invocation(
					tcp_connection_ctx_weak_handle_t weak_handle )
				{
					m_timer_manager->schedule_timer( this, std::move( weak_handle ) );
				}

				// Cancel timeout guard if any.
				void
				cancel()
				{
					m_timer_manager->cancel_timer( this );
				}

			private:
				std::shared_ptr< asio_tick_timer_manager_t > m_timer_manager;
			//! \}
		};

		// Create guard for connection.
		timer_guard_t
		create_timer_guard()
		{
			return timer_guard_t( this->shared_from_this() );
		}

		void
		start()
		{
			asio::dispatch(
				m_strand,
				[ ctx = this->shared_from_this() ]{
					ctx->m_timers.clear();
					ctx->schedule_next_tick();
				} );
		}

		void
		stop()
		{
			asio::dispatch(
				m_strand,
				[ ctx = this->shared_from_this() ]{
					ctx->m_tick_timer.cancel();
				} );
		}

		//! Schedule or reschedule a given timer with specified callback.
		void
		schedule_timer(
			void * timer_id,
			tcp_connection_ctx_weak_handle_t tcp_connection_ctx )
		{
			asio::dispatch(
				m_strand,
				[ ctx = this->shared_from_this(),
					tcp_connection_ctx = std::move( tcp_connection_ctx ),
					timer_id ]{

					ctx->schedule_impl( timer_id,std::move( tcp_connection_ctx ) );
				} );
		}

		//! Cancel a given timer.
		void
		cancel_timer( void * timer_id )
		{
			// It is important to use post here:
			// beacause a chain of dispatch calls when expired timer is hit
			// causes cancel_impl() to execute which invalidates
			// iterator on expired timer table entry in
			// check_expired_timers() loop.
			asio::dispatch(
				m_strand,
				[ timer_id, ctx = this->shared_from_this() ]{
					ctx->cancel_impl( timer_id );
				} );
		}

	private:
		void
		schedule_next_tick()
		{
			m_tick_timer.expires_after( m_tick );
			m_tick_timer.async_wait(
				asio::bind_executor(
					m_strand,
					[ ctx = this->shared_from_this() ]( const auto & ec ){
						if( !ec )
						{
							ctx->check_expired_timers();
							ctx->schedule_next_tick();
						}
						else
						{
							ctx->m_timers.clear();
						}
					} ) );
		}

		void
		schedule_impl(
			void * timer_id,
			tcp_connection_ctx_weak_handle_t tcp_connection_ctx )
		{
			auto it = m_timers.find( timer_id );
			if( m_timers.end() != it )
			{
				it->second.m_expired_after = std::chrono::steady_clock::now() + m_check_period;
			}
			else
			{
				typename timer_table_t::value_type
					entry{
						timer_id,
						timer_data_t{
							m_check_period,
							std::move( tcp_connection_ctx ) } };

				m_timers.insert( std::move( entry ) );
			}
		}

		void
		cancel_impl( void * timer_id )
		{
			m_timers.erase( timer_id );
		}

		//! Checks expired timers, execute them and remove them from table.
		void
		check_expired_timers()
		{
			auto it = m_timers.begin();
			const auto it_end = m_timers.end();

			const auto now = std::chrono::steady_clock::now();

			while( it_end != it )
			{
				auto & d = it->second;

				if( d.m_expired_after < now )
				{
					auto tcp_connection_ctx = std::move( d.m_tcp_connection_ctx );
					it = m_timers.erase( it );

					if( auto h = tcp_connection_ctx.lock() )
						h->check_timeout();
				}
				else
				{
					++it;
				}
			}
		}

		asio::steady_timer m_tick_timer;
		Executor m_strand;

		//! Tick duration.
		const std::chrono::steady_clock::duration m_tick;
		const std::chrono::steady_clock::duration m_check_period;

		struct timer_data_t
		{
			timer_data_t(
				std::chrono::steady_clock::duration timeout_from_now,
				tcp_connection_ctx_weak_handle_t tcp_connection_ctx )
				:	m_expired_after{ std::chrono::steady_clock::now() + timeout_from_now }
				,	m_tcp_connection_ctx{ std::move( tcp_connection_ctx ) }
			{}

			timer_data_t( const timer_data_t & ) = delete;
			const timer_data_t & operator = ( const timer_data_t & ) = delete;

			timer_data_t( timer_data_t && ) = default;
			timer_data_t & operator = ( timer_data_t && ) = delete;

			std::chrono::steady_clock::time_point m_expired_after;
			tcp_connection_ctx_weak_handle_t m_tcp_connection_ctx;
		};

		using timer_table_t = std::unordered_map< void *, timer_data_t >;
		timer_table_t m_timers;
};

using mt_asio_tick_timer_manager_t = asio_tick_timer_manager_t< asio::strand< asio::executor > >;
using st_asio_tick_timer_manager_t = asio_tick_timer_manager_t< asio::executor >;

} /* namespace restinio */