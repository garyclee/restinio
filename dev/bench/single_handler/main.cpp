/*
	restinio bench single handler.
*/

#include <iostream>
#include <fstream>

#include <args.hxx>

#include <restinio/all.hpp>

#include <bench/common/cfg.hpp>

//
// app_args_t
//

struct app_args_t
{
	bool m_help{ false };
	std::string m_config_file;
	std::size_t m_asio_pool_size{ 1 };

	app_args_t( int argc, const char * argv[] )
	{
		args::ArgumentParser parser( "Single handler RESTinio benchmark", "" );
		args::HelpFlag help( parser, "Help", "Usage example", { 'h', "help" } );

		args::ValueFlag< std::string > config_file(
				parser, "config_file", "Configuration file",
				{ 'c', "config" } );
		args::ValueFlag< std::size_t > asio_pool_size(
				parser, "asio_pool_size",
				"Size of worker thread pool for Asio",
				{ "asio-pool-size" } );

		try
		{
			parser.ParseCLI( argc, argv );
		}
		catch( const args::Help & )
		{
			m_help = true;
			std::cout << parser;
		}

		if( config_file )
			m_config_file = args::get( config_file );
		else if( !m_help )
		{
			std::cout << parser;
			throw std::runtime_error( "Config name must be specified" );
		}

		if( asio_pool_size )
			m_asio_pool_size = args::get( asio_pool_size );
	}
};

const std::string RESP_BODY{ "Hello world!" };

struct req_handler_t
{
	auto
	operator () (
		restinio::http_request_handle_t req,
		restinio::connection_handle_t conn ) const
	{
		if( restinio::http_method_get() == req->m_header.method() &&
			req->m_header.request_target() == "/" )
		{
			restinio::response_builder_t{ req->m_header, std::move( conn ) }
				.append_header( "Server", "RESTinio Benchmark server" )
				.append_header_date_field()
				.append_header( "Content-Type", "text/plain; charset=utf-8" )
				.set_body( RESP_BODY )
				.done();

			return restinio::request_accepted();
		}

		return restinio::request_rejected();
	}
};

int
main(int argc, const char *argv[] )
{
	try
	{
		const app_args_t args{ argc, argv };

		if( !args.m_help )
		{
			std::cout << "pool size: " << args.m_asio_pool_size << std::endl;

			using traits_t =
				restinio::traits_t<
					restinio::asio_timer_factory_t,
					restinio::null_logger_t,
					req_handler_t >;
			using http_server_t =
				restinio::http_server_t< traits_t >;
			using server_settings_t =
				restinio::server_settings_t< traits_t >;

			server_settings_t settings{};

			{
				std::ifstream fin{ args.m_config_file, std::ios::binary };
				if( !fin )
				{
					throw std::runtime_error{ "unable to open config: " + args.m_config_file };
				}
				json_dto::from_stream( fin, settings );
				fin.close();
			}

			http_server_t http_server{
				restinio::create_child_io_service( args.m_asio_pool_size ),
				std::move( settings ) };

			http_server.open();
			// Wait for quit command.
			std::cout << "Type \"quit\" or \"q\" to quit." << std::endl;

			std::string cmd;
			do
			{
				std::cin >> cmd;
			} while( cmd != "quit" && cmd != "q" );

			http_server.close();
		}
	}
	catch( const std::exception & ex )
	{
		std::cerr << "Error: " << ex.what() << std::endl;
		return 1;
	}

	return 0;
}