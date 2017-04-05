/*
	restinio
*/

/*!
	A base class for connection handle.
*/

#pragma once

#include <memory>

namespace restinio
{

//
// connection_base_t
//

class connection_base_t
	:	public std::enable_shared_from_this< connection_base_t >
{
		friend class response_builder_t;

	public:
		connection_base_t(std::uint64_t id )
			:	m_connection_id{ id }
		{}

		//! Get connection id.
		std::uint64_t
		connection_id() const
		{
			return m_connection_id;
		}

	protected:
		virtual void
		write_response_message(
			//! Response header.
			http_response_header_t http_header,
			//! Body.
			std::string body ) = 0;

	private:
		//! Id of a connection.
		const std::uint64_t m_connection_id;
};

using connection_handle_t =
	std::shared_ptr< connection_base_t >;

} /* namespace restinio */