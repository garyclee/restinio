# *RESTinio*

[TOC]

# What Is It?
*RESTinio* is a header-only library for creating REST applications in c++.
It helps to create http server that can handle requests asynchronously.
Currently it is in beta state and represents our solution for the problem of
being able to handle request asynchronously with additional features.

RESTinio is distributed BSD-3-CLAUSE license. General support is provided via
Issues section on Bitbucket. If you need commercial support then contact "info
at stiffstream dot com" for more information.

## Why creating yet another library of that kind?

Well, there are lots of libraries and frameworks
of all sorts of complexity and maturity for
building REST service in C++.
Isn't it really a [NIH syndrom](https://en.wikipedia.org/wiki/Not_invented_here)?

We've used some of already available libraries and have tried lots of them.
And we have found that pretty much of them lack an ability
to handle requests asynchronously.
Usually library design forces user to set the response
inside a handler call. So when handling needs some interactions
with async API such design results in blocking of a caller thread.
And that hurts when the rest of the application is built on async bases.

In addition to async handling feature we though it would be nice
for such library to keep track of what is going on with connections and
control timeouts on operations of reading request from socket,
handling request and writing response to socket.
And it would also be nice to have request handler router
(like in [express](https://expressjs.com/)).
And a header-only design is a plus.

And it happens that under such conditions you don't have a lot of options.
So we have come up with *RESTinio*...

# Obtain And Build

## Prerequisites

To use *RESTinio* it is necessary to have:

* Reasonably modern C++14 compiler (VC++14.0, GCC 5.4 or above, clang 3.8 or above);
* [asio](http://think-async.com/Asio) from [git repo](https://github.com/chriskohlhoff/asio.git), commit `f5c570826d2ebf50eb38c44039181946a473148b`;
* [nodejs/http-parser](https://github.com/nodejs/http-parser) 2.7.1;
* [fmtlib](http://fmtlib.net/latest/index.html) 4.0.0.
* Optional: [SObjectizer](https://sourceforge.net/projects/sobjectizer/) 5.5.19.3;

For building samples, benchmarks and tests:

* [Mxx_ru](https://sourceforge.net/projects/mxxru/) 1.6.13 or above;
* [rapidjson](https://github.com/miloyip/rapidjson) 1.1.0;
* [json_dto](https://bitbucket.org/sobjectizerteam/json_dto-0.1) 0.1.2.1 or above;
* [args](https://github.com/Taywee/args) 6.0.4;
* [CATCH](https://github.com/philsquared/Catch) 1.9.6.

## Obtaining

There are two ways of obtaining *RESTinio*.

* Getting from
[repository](https://bitbucket.org/sobjectizerteam/restinio-0.3).
In this case external dependencies must be obtained with Mxx_ru externals tool.
* Getting
[archive](https://bitbucket.org/sobjectizerteam/restinio-0.2/downloads/restinio-0.2.1-full.tar.bz2).
Archive includes source code for all external dependencies.

### Cloning of hg repository

```
hg clone https://bitbucket.org/sobjectizerteam/restinio-0.3
```

And then:
```
cd restinio-0.3
mxxruexternals
```
to download and extract *RESTinio*'s dependencies.

### MxxRu::externals recipe

See MxxRu::externals recipes for *RESTinio*
[here](./doc/MxxRu_externals_recipe.md).

### Getting archive

```
wget https://bitbucket.org/sobjectizerteam/restinio-0.3/downloads/restinio-0.3.0-full.tar.bz2
tar xjvf restinio-0.3.0-full.tar.bz2
cd restinio-0.3.0-full
```

## Build

### CMake

Building with CMake currently is provided for samples, tests and benches
not depending on SObjectizer.
To build them run the following commands:
```
hg clone https://bitbucket.org/sobjectizerteam/restinio-0.3
cd restinio-0.3
mxxruexternals
cd dev
mkdir cmake_build
cd cmake_build
cmake -DCMAKE_INSTALL_PREFIX=target -DCMAKE_BUILD_TYPE=Release ..
make
make install
```

Or, if getting sources from archive:
```
wget https://bitbucket.org/sobjectizerteam/restinio-0.3/downloads/restinio-0.3.0-full.tar.bz2
tar xjvf restinio-0.3.0-full.tar.bz2
cd restinio-0.3.0-full/dev
mkdir cmake_build
cd cmake_build
cmake -DCMAKE_INSTALL_PREFIX=target -DCMAKE_BUILD_TYPE=Release ..
make
make install
```

### Mxx_ru
While *RESTinio* is header-only library, samples, tests and benches require a build.

Compiling with Mxx_ru:
```
hg clone https://bitbucket.org/sobjectizerteam/restinio-0.3
cd restinio-0.3
mxxruexternals
cd dev
ruby build.rb
```

For release or debug builds use the following commands:
```
ruby build.rb --mxx-cpp-release
ruby build.rb --mxx-cpp-debug
```

*NOTE.* It might be necessary to set up `MXX_RU_CPP_TOOLSET` environment variable,
see Mxx_ru documentation for further details.

### Dependencies default settings

External libraries used by *RESTinio* have the following default settings:

* A standalone version of *asio* is used and a chrono library is used,
so `ASIO_STANDALONE` and `ASIO_HAS_STD_CHRONO` defines are necessary. Also
`ASIO_DISABLE_STD_STRING_VIEW` is defined, btcause it is a C++17 feature and
not all compilers support it yet;
* a less strict version of *nodejs/http-parser* is used, so the following
definition `HTTP_PARSER_STRICT=0` is applied;
* *fmtlib* is used as a header-only library, hence a `FMT_HEADER_ONLY`
define is necessary;
* for *RapidJSON* two definitions are necessary: `RAPIDJSON_HAS_STDSTRING` and
`RAPIDJSON_HAS_CXX11_RVALUE_REFS`.

# Getting started

To start using *RESTinio* make sure that all dependencies are available.
The tricky one is [nodejs/http-parser](https://github.com/nodejs/http-parser),
because it is a to be compiled unit, which can be built as a static library and
linked to your target or can be a part of your target.
And it is worth to mention that *asio* that is used by *RESTinio* is the one from
github [repository](https://github.com/chriskohlhoff/asio.git).

To start using *RESTinio* simply include `<restinio/all.hpp>` header.
It includes all necessary headers of the library.

It easy to learn how to use *RESTinio* by example.

## Minimalistic hello world

Here is a minimal hello world http server
([see full example](./dev/sample/hello_world_minimal/main.cpp)):
~~~~~
::c++
#include <iostream>
#include <restinio/all.hpp>

int main()
{
  restinio::run(
    restinio::on_this_thread()
      .port(8080)
      .address("localhost")
      .request_handler([](auto req) {
        return req->create_response().set_body("Hello, World!").done();
      }));

  return 0;
}
~~~~~

Here a helper function `restinio::run()` is used to create and run the server.
It is the easiest way to start the server,
it hides some boilerplate code for simple common cases.
*RESTinio* considers the following two typical cases:

* run server on current thread;
* run server on thread pool.

See details [here](./dev/restinio/http_server_run.hpp).

Each `restinio::run()` function creates http server instance with specified settings
and runs it. And for also it subscribes to breakflag signals to stop the server
when ctrl+c is hit.

*RESTinio* does its networking stuff with
[asio](https://github.com/chriskohlhoff/asio.git) library, so to run server
it must have an `asio::io_context` instance to run on.
Each `restinio::run()` function creates an instance of `asio::io_context`
and then runs it (via `asio::io_context::run()` function) on a current thread or
on a thread pool.

`restinio::run()` functions receive server settings as an argument.
Server settings is a [fluent interface](https://en.wikipedia.org/wiki/Fluent_interface)
class for various server options. Most of the settings have reasonable default values.
And one setting is especially important: it is `request_handler`.
It is a function-object that handles http requests.
In the sample above it is lambda that serves all request with "Hello, World!" response.
Other two option in the sample specify to run server on localhost
and listen for connections on port 8080.

## Enhance request handler

Lets see a more complicated request handler and look at what it does.

Let's use the following function as a request handler:
~~~~~
::c++
restinio::request_handling_status_t handler(restinio::request_handle_t req);
{
  if( restinio::http_method_get() == req->header().method() &&
      req->header().request_target() == "/" )
  {
    req->create_response()
      .append_header( restinio::http_field::server, "RESTinio hello world server" )
      .append_header_date_field()
      .append_header( restinio::http_field::content_type, "text/plain; charset=utf-8" )
      .set_body( "Hello world!")
      .done();
    return restinio::request_accepted();
  }
  return restinio::request_rejected();
}

int main()
{
  restinio::run(
    restinio::on_this_thread()
      .port(8080).address("localhost")
      .request_handler(handler));

  return 0;
}
~~~~~

Function has a single parameter that holds a handle on actual request - `restinio::request_t`
that has the following API (details are omitted):
~~~~~
::c++
class request_t // Base class omitted.
{
  public:
    const http_request_header_t & header() const;

    const std::string & body() const;

    template <typename Output = restinio_controlled_output_t>
    auto create_response(
      std::uint16_t status_code = 200,
      std::string reason_phrase = "OK" );
};
~~~~~

* `request_t::header()` gives access to `http_request_header_t` object that describes
http header of a given request;
* `request_t::body()` gives access to body of a given request;
* `request_t::create_response()` creates an object for cunstructing and sending request.

First two functions are rather straightforward,
so for them there is no much sense to go into more details.
A more intriguing function is `request_t::create_response()` -
it is a template function that is customized with `Output` type.
For now it would be enough to use the default customization.
`request_t::create_response()` has two parameter *status code* (http response code)
and *reason phrase* (http response reason phrase).
By default these params are set for ` HTTP/1.1 200 OK` response.
If called for the first time `request_t::create_response()`
returns an instance of type `response_builder_t<Output>`,
it is an object for constructing and sending response on a given request.
To avoid the possibility of creating multiple responses on a single request
`request_t::create_response()` will return an object only for the first call
all further call would throw.

For setting response in the sample above we use some functions  of a response builder:

* `append_header(field, value)` - for setting header fields;
* `append_header_date_field()` - for setting `Date` field value with current timmestamp;
* `set_body()` - for setting response body;

All of mentioned functions return the reference to a response builder that gives
some syntactic sugar and allows to set response in a nice way.

For sending response to peer a `done()` function is used.
It stops constructing response
(no setters will have an effect on the response) and initiates sending
a response via underlying TCP connection.

A one thing left to mention is what request handler returns.
Any request handler must return a value of `request_handling_status_t` enum:
~~~~~
::c++
enum class request_handling_status_t
{
  accepted,
  rejected
};
~~~~~

There are two helper functions:
`restinio::request_accepted()` and `restinio::request_rejected()`
for refering an itemes of enum. Both of them are used in the sample.

See also a full ([sample](./dev/sample/hello_world_basic/main.cpp)).

## Enhance request handler even more

Here we will add basic routing for incoming requests.
*RESTinio* has [express](https://expressjs.com/)-like request handler router
(see [express router](#markdown-header-express-router) for more deteils).

The signature of a handlers that can be put in router
has an additional parameter -- a container with parameters extracted from URI.

Lets see how we can use express router in a sample request handle:
~~~~~
::c++
using router_t = restinio::router::express_router_t;

auto create_server_handler()
{
  auto router = std::make_unique< router_t >();

  router->http_get(
    "/",
    []( auto req, auto ){
        init_resp( req->create_response() )
          .append_header( restinio::http_field::content_type, "text/plain; charset=utf-8" )
          .set_body( "Hello world!")
          .done();

        return restinio::request_accepted();
    } );

  router->http_get(
    "/json",
    []( auto req, auto ){
        init_resp( req->create_response() )
          .append_header( restinio::http_field::content_type, "text/json; charset=utf-8" )
          .set_body( R"-({"message" : "Hello world!"})-")
          .done();

        return restinio::request_accepted();
    } );

  router->http_get(
    "/html",
    []( auto req, auto ){
        init_resp( req->create_response() )
            .append_header( restinio::http_field::content_type, "text/html; charset=utf-8" )
            .set_body(
              "<html>\r\n"
              "  <head><title>Hello from RESTinio!</title></head>\r\n"
              "  <body>\r\n"
              "    <center><h1>Hello world</h1></center>\r\n"
              "  </body>\r\n"
              "</html>\r\n" )
            .done();

        return restinio::request_accepted();
    } );


  return router;
}

int main()
{
	using traits_t =
		restinio::traits_t<
			restinio::asio_timer_factory_t,
			restinio::single_threaded_ostream_logger_t,
			router_t >;

	restinio::run(
		restinio::on_this_thread<traits_t>()
			.port( 8080 )
			.address( "localhost" )
			.request_handler( create_request_handler() ) );
}
~~~~~

Function `create_server_handler()` creates an instance of
`restinio::router::express_router_t` with three endpoints:

* '/' - default path: reply with text hello message;
* '/json': reply with json hello message;
* '/html': reply with html hello message.

Note in the sample above we do not use route parameters.

Considering implementation of `create_server_handler()` above,
we can notice that it return a unique pointer on a router class.
And it is not function object. So how *RESTinio* can use it?
To receive an accurate answer one should read
[basic idea](#markdown-header-basic-idea) section first.

A brief and non accurate answer will be that
"RESTinio" is customizable for concrete types of handler,
and if it knows a concrete type of a handler it can receive
it wrapped in unique pointer. To set this (and not only this) customization
a traits calss is used.
In samples in prevous sections a default traits were used, so they were hidden.
In this sample we explicitly define a traits to use:
~~~~~
::c++
using router_t = restinio::router::express_router_t;

using traits_t =
  restinio::traits_t<
    restinio::asio_timer_factory_t,
    restinio::single_threaded_ostream_logger_t,
    router_t >;
~~~~~

Here we use a helper class `restinio::traits_t`
that has some customizations defined by default.
Request handler type is on the third place.

And to run the server we need to point the traits we are using:
~~~~~
::c++
restinio::run(
  restinio::on_this_thread<traits_t>() // Use custom traits.
    .port( 8080 )
    .address( "localhost" )
    .request_handler( create_request_handler() );
~~~~~

Function `restinio::on_this_thread<Traits>()` involves a creation of a special type
and from this type `restinio::run()` function deduces the trats for its server.

See also a full ([sample](./dev/sample/hello_world_basic/main.cpp)).

# Basic idea

When describing  *RESTinio* http server there are three abstractions
vital for understanding of how to use it.

* `http_server_t<Traits>` - a class representing http server.
* `server_settings_t<Traits>` - a class representing server settings.
* `Traits` - a customization type for previous abstractions,
that makes it possible to tune concrete server implementaion.

Not less of importance is the fact that *RESTinio* runs on stand-alone
version of [asio](https://github.com/chriskohlhoff/asio.git).
We picked a tag from master branch to be closer to
[Networking TS](https://github.com/chriskohlhoff/networking-ts-impl)
that is aimed to be part of a standard.
An instance of `asio::io_context` is used to run the server.
*RESTinio* can use its own privately created `asio::io_context` or
it can make use of external io_context instance.
The last case requires some assumptions to be fulfilled
(see [using external io_context](#markdown-header-using-external-io_context)).

## Traits

Main class `http_server_t` is a template class parameterized with a single
template parameter: `Traits`.
Traits class must specify a set of types used inside *RESTinio*, they are:
~~~~~
::c++
timer_factory_t;
logger_t;
request_handler_t;
strand_t;
stream_socket_t;
~~~~~

There is a helper classes for working with traits:

~~~~~
::c++
template <
    typename Timer_Factory,
    typename Logger,
    typename Request_Handler = default_request_handler_t,
    typename Strand = asio::strand< asio::executor >,
    typename Socket = asio::ip::tcp::socket >
struct traits_t; // Implementation omitted.

template <
    typename Timer_Factory,
    typename Logger,
    typename Request_Handler = default_request_handler_t >
using single_thread_traits_t =
  traits_t< Timer_Factory, Logger, Request_Handler, noop_strand_t >; // Implementation omitted.
~~~~~

Refer to [Traits](#markdown-header-traits_1) and [restinio/traits.hpp](./dev/restinio/traits.hpp) for details.

## Class *http_server_t<Traits>*

Class `http_server_t<Traits>` is a template class parameterized with a single template parameter: `Traits`.
Its meaning is directly depicted in its name, `http_server_t<Traits>`
represents http-server.
It is handy to consider `http_server_t<Traits>` class as a root class
for the rest of *RESTinio* ecosystem running behind it, because pretty much all of them are
also template types parameterized with the same `Traits` parameter.

Class `http_server_t<Traits>` has two constructors (simplified to omit verbose template stuff):
~~~~~
::c++
http_server_t(
  io_context_holder_t io_context,
  server_settings_t<Traits> settings );

template < typename Configurator >
http_server_t(
  io_context_holder_t io_context,
  Configurator && configurator );
~~~~~

The first is the main one. It obtains `io_context` as an `asio::io_context` back-end
and server settings with the bunch of params.

The second constructor can simplify setting of parameters via generic lambda like this:
~~~~~
::c++
http_server_t< my_traits_t > http_server{
  restinio::own_io_context(),
  []( auto & settings ){ // Omit concrete name of settings type.
    settings
      .port( 8080 )
      .read_next_http_message_timelimit( std::chrono::seconds( 1 ) )
      .handle_request_timeout( std::chrono::milliseconds( 3900 ) )
      .write_http_response_timelimit( std::chrono::milliseconds( 100 ) )
      .logger( /* logger params */ )
      .request_handler( /* request handler params */ );
  } };
~~~~~

But in the end it delegates construction to the first custructor.

*RESTinio* runs its logic on `asio::io_context`, but its internal logic
is separated from maintaining io_context directly, hence allowing to
run restinio on external (specified by user) instance of `asio::io_context`
(see [using external io context](#markdown-header-using-external-io_context)).
So there is a special class for wrapping io_context instance and pass it to
`http_server_t` constructor: `io_context_holder_t`.
To create the such holder use on of the following functions:

* `restinio::own_io_context()` -- create and use its own instance of io_context;
* `restinio::external_io_context()` -- use external instance of io_context.

### Running server

To run server there are open()/close() methods :
~~~~~
::c++
class http_server_t
{
  // ...
  public:
    template <
        typename Server_Open_Ok_CB,
        typename Server_Open_Error_CB >
    void
    open_async(
      Server_Open_Ok_CB && open_ok_cb,
      Server_Open_Error_CB && open_err_cb )

    void
    open_sync();

    template <
        typename Server_Close_Ok_CB,
        typename Server_Close_Error_CB >
    void
    close_async(
      Server_Close_Ok_CB && close_ok_cb,
      Server_Close_Error_CB && close_err_cb );

    void
    close_sync();
    //...
}
~~~~~

There are sync methods for starting/stoping server and async.
To choose the right method it is necessary to understand
that *RESTinio* doesn't start and run io_context that it runs on.
So user is responsible for running io_context.
Sync versions of `open()/close()` methods assume they are called on
the context of a running io_context. For example:
~~~~~
::c++
// Create and initialize object.
restinio::http_server_t< my_traits_t > server{
  restinio::own_io_context(),
  [&]( auto & settings ){
    //
    settings
      .port( args.port() )
      // .set_more_params( ... )
      .request_handler(
        []( restinio::request_handle_t req ){
            // Handle request.
        } );
  } };

// Post initial action to asio event loop.
asio::post( server.io_context(),
  [&] {
    // Starting the server in a sync way.
    server.open_sync();
  } );

// Running server.
server.io_context().run();
~~~~~

Async versions of `open()/close()` methods can be used from any thread.
But it is not guaranteed that server is already  started when method finishes.
When using async_open() user provides two callbacks, the first one is called if server starts
successfully, and the second one is for handling error.
For example:
~~~~~
::c++
asio::io_context io_ctx;
restinio::http_server_t< my_traits_t > server{
    restinio::external_io_context(io_ctx),
    [&]( auto & settings ) { ... } };

// Launch thread on which server will work.
std::thread server_thread{ [&] {
    io_ctx.run();
  } };

// Start server in async way. Actual start will be performed
// on the context of server_thread.
server.open_async(
    // Ok callback. Nothing to do.
    []{},
    // Error callback. Rethrow an exception.
    []( auto ex_ptr ) {
      std::rethrow_exception( ex_ptr );
    } );
...
// Wait while server_thread finishes its work.
server_thread.join();
~~~~~

Refer to ([restinio/http_server.hpp](./dev/restinio/http_server.hpp)) for details.

## Class *server_settings_t<Traits>*

Class `server_settings_t<Traits>` serves to pass settings to `http_server_t<Traits>`.
It is defined in [restinio/settings.hpp](./dev/restinio/settings.hpp);

For each parameter a setter/getter pair is provided.
While setting most of parameters is pretty straightforward,
there are some parameters with a bit tricky setter/getter semantics.
They are request_handler, timer_factory, logger, acceptor_options_setter,
socket_options_setter and cleanup_func.

For example setter for request_handler looks like this:
~~~~~
::c++
template< typename... PARAMS >
server_settings_t &
request_handler( PARAMS &&... params );
~~~~~

When called an instance of `std::unique_ptr<Traits::request_handler_t>`
will be created with specified `params`.
If no constructor with such parameters is available, then compilation error will occur.
If `request_handler_t` has a default constructor then it is not
mandatory to call setter -- the default constructed instance will be used.
But there is an exception for `std::function` type,
because even though it has a default constructor
it will be useless when constructed in such a way.

Request handler is constructed as unique_ptr, then getter
returns unique_ptr value with ownership, so while manipulating
`server_settings_t` object don't use it.

The same applies to timer_factory,logger parameters, acceptor_options_setter,
socket_options_setter and cleanup_func.

When `http_server_t` instance is created all settings are checked to be properly instantiated.

Refer to [server settings](#markdown-header-server-settings) and [restinio/settings.hpp](./dev/restinio/settings.hpp) for details.

# Traits

## List of types that must be defined be *Traits*

* `timer_factory_t` defines the logic of how timeouts are managed;
* `logger_t` defines logger that is used by *RESTinio* to track its inner logic;
* `request_handler_t` defines a function-like type to be used as request handler;
* `strand_t` - defines a class that is used by connection as a wrapper
for its callback-handlers running on `asio::io_context` thread(s)
in order to guarantee serialized callbacks invocation
(see [asio doc](https://chriskohlhoff.github.io/networking-ts-doc/doc/networking_ts/reference/strand.html)).
Actually there are two options for the strand type:
`asio::strand< asio::executor >` and `asio::executor`.
The first is a real strand that guarantees serialized invocation and
the second one is simply a default executor to eliminate unnecessary overhead
when running `asio::io context` on a single thread;
* `stream_socket_t` is a customization point that tells restinio
what type of socket used for connections. This parameter allows restinio
to support TLS connection (see [TLS support](#markdown-header-tls-support.md)).

## timer_factory_t
`timer_factory_t` - defines a timeout controller logic.
It must define a nested type `timer_guard_t` with the following interface:

~~~~~
::c++
class timer_guard_t
{
  public:
    // Set new timeout guard.
    template <
        typename Executor,
        typename Callback_Func >
    void
    schedule_operation_timeout_callback(
      const Executor & executor,
      std::chrono::steady_clock::duration timeout,
      Callback_Func && f );

    // Cancel timeout guard if any.
    void
    cancel();
};
~~~~~

The first method starts guarding timeout of a specified duration,
and if it occurs some how the specified callback must be posted on
`asio::io_context` executor.

The second method must cancel execution of the previously scheduled timer.

An instance of `std::shared_ptr< timer_guard_t >` is stored in each connection
managed by *RESTinio* and to create it `timer_factory_t` must define
the following method:

~~~~~
::c++
class timer_factory_t
{
  public:
    // ...

    using timer_guard_instance_t = std::shared_ptr< timer_guard_t >;

    // Create guard for connection.
    timer_guard_instance_t
    create_timer_guard( asio::io_context & );
    // ...
};
~~~~~

*RESTinio* comes with a set of ready-to-use `timer_factory_t` implementation:

* `null_timer_factory_t` -- noop timer guards, they produce timer guards
that do nothing (when no control needed).
See [restinio/null_timer_factory.hpp](./dev/restinio/null_timer_factory.hpp);
* `asio_timer_factory_t` -- timer guards implemented with asio timers.
See [restinio/asio_timer_factory.hpp](./dev/restinio/asio_timer_factory.hpp);
* `so5::so_timer_factory_t` -- timer guards implemented with *SObjectizer* timers.
See [restinio/so5/so_timer_factory.hpp](./dev/restinio/so5/so_timer_factory.hpp)
Note that `restinio/so5/so_timer_factory.hpp` header file is not included
by `restinio/all.hpp`, so it needs to be included separately.

## logger_t

`logger_t` - defines a logger implementation.
It must support the following interface:
~~~~~
::c++
class null_logger_t
{
  public:
    template< typename Msg_Builder >
    void trace( Msg_Builder && mb );

    template< typename Msg_Builder >
    void info( Msg_Builder && mb );

    template< typename Msg_Builder >
    void warn( Msg_Builder && mb );

    template< typename Msg_Builder >
    void error( Msg_Builder && mb );
};
~~~~~

`Msg_Builder` is lambda that returns a message to log out.
This approach allows compiler to optimize logging when it is possible,
see [null_logger_t](./dev/restinio/loggers.hpp).

For implementation example see [ostream_logger_t](./dev/restinio/ostream_logger.hpp).

## request_handler_t

`request_handler_t` - is a key type for request handling process.
It must be a function-object with the following invocation interface:
~~~~~
::c++
restinio::request_handling_status_t
handler( restinio::request_handle_t req );
~~~~~

The `req` parameter defines request data and stores some data necessary for creating responses.
Parameter is passed by value and thus can be passed to another processing flow
(that is where an async handling becomes possible).

Handler must return handling status via `request_handling_status_t` enum.
If handler handles request it must return `accepted`.
If handler refuses to handle request it must return `rejected`.
There are two helper functions:
`restinio::request_accepted()` and `restinio::request_rejected()`
for refering an itemes of enum.

## strand_t

`strand_t` provides serialized callback invocation for events of a specific connection.
There are two option for `strand_t`: `asio::strand< asio::executor >` or `asio::executor`.

By default `asio::strand< asio::executor >` is used,
it guarantees serialized chain of callback invocation.
But if `asio::io_context` runs on a single thread there is no need
to use `asio::strand` because there is no way to run callbacks in parallel.
So in such cases it is enough to use `asio::executor` directly and
eliminate overhead of `asio::strand`.

## stream_socket_t

`stream_socket_t` allows to customize underlying socket type,
so it possible to create https server using identical interface (see [TLS support](#markdown-header-tls-support)).

# Response builder

Lets consider that we are at the point when response
on a particular request is ready to be created and send.
The key here is to use a given connection handle and
[response_builder_t](./dev/restinio/message_builders.hpp) that is created by
this connection handle:

Basic example of default response builder:
~~~~~
::c++
// Construct response builder.
auto resp = req->create_response(); // 200 OK

// Set header fields:
resp.append_header( restinio::http_field::server, "RESTinio server" );
resp.append_header_date_field();
resp.append_header( restinio::http_field::content_type, "text/plain; charset=utf-8" );

// Set body:
resp.set_body( "Hello world!" );

// Response is ready, send it:
resp.done();
~~~~~

Currently there are three types of response builders.
Each builder type is a specialization of a template class `response_builder_t< Output >`
with a specific output-type:

* Output `restinio_controlled_output_t`. Simple standard response builder.
* Output `user_controlled_output_t`. User controlled response output builder.
* Output `chunked_output_t`. Chunked transfer encoding output builder.

## RESTinio controlled output response builder

Requires user to set header and body.
Content length is automatically calculated.
Once the data is ready, the user calls done() method
and the resulting response is scheduled for sending.

~~~~~
::c++
 handler =
  []( auto req ) {
    if( restinio::http_method_get() == req->header().method() &&
      req->header().request_target() == "/" )
    {
      req->create_response()
        .append_header( restinio::http_field::server, "RESTinio hello world server" )
        .append_header_date_field()
        .append_header( restinio::http_field::content_type, "text/plain; charset=utf-8" )
        .set_body( "Hello world!")
        .done();

      return restinio::request_accepted();
    }

    return restinio::request_rejected();
  };
~~~~~

## User controlled output response builder

This type of output allows user
to send body divided into parts.
But it is up to user to set the correct
Content-Length field.

~~~~~
::c++
 handler =
  []( restinio::request_handle_t req ){
    using output_type_t = restinio::user_controlled_output_t;
    auto resp = req->create_response< output_type_t >();

    resp.append_header( restinio::http_field::server, "RESTinio" )
      .append_header_date_field()
      .append_header( restinio::http_field::content_type, "text/plain; charset=utf-8" )
      .set_content_length( req->body().size() );

    resp.flush(); // Send only header

    for( const char c : req->body() )
    {
      resp.append_body( std::string{ 1, c } );
      if( '\n' == c )
      {
        resp.flush();
      }
    }

    return resp.done();
  }
~~~~~

## Chunked transfer encoding output builder

This type of output sets transfer-encoding to chunked
and expects user to set body using chunks of data.

~~~~~
::c++
 handler =
  []( restinio::request_handle_t req ){
    using output_type_t = restinio::chunked_output_t;
    auto resp = req->create_response< output_type_t >();

    resp.append_header( restinio::http_field::server, "RESTinio" )
      .append_header_date_field()
      .append_header( restinio::http_field::content_type, "text/plain; charset=utf-8" );

    resp.flush(); // Send only header


    for( const char c : req->body() )
    {
      resp.append_chunk( std::string{ 1, c } );
      if( '\n' == c )
      {
        resp.flush();
      }
    }

    return resp.done();
  }
~~~~~

# Server settings

## Traits independent settings
|      settings        | type | description |
|----------------------|------|-------------|
| port | `std::uint16_t` | Server port. |
| protocol | `asio::ip::tcp` | Protocol ipv4/ipv6 used for server endpoint. |
| address | `std::string` | Addres for server endpoint. Can be set to a concrete ip address when running on maschine with two or more network cards. Also supports values `localhost`, `ip6-localhost`. |
| buffer_size | `std::size_t` | It limits a size of chunk that can be read from socket in a single read operattion (when receiving http request). |
| read_next_http_message_timelimit | `std::chrono::steady_clock::duration` | A period for holding connection before completely receiving new http-request. Starts counting since connection is establised or a previous request was responsed. |
| write_http_response_timelimit | `std::chrono::steady_clock::duration` | A period of time wait for response to be written to socket. |
| handle_request_timeout | `std::chrono::steady_clock::duration` | A period of time that is given for a handler to create response. |
| max_pipelined_requests | `std::size_t` | Max pipelined requests able to receive on single connection. |
| acceptor_options_setter | `acceptor_options_setter_t` | Acceptor options setter. |
| socket_options_setter | `socket_options_setter_t` | Socket options setter. |
| concurrent_accepts_count | `std::size_t` | Max number of running concurrent accepts. When running server on N threads then up to N accepts can be handled concurrently. |
| separate_accept_and_create_connect | `bool` | For the cases when a lot of connection can be fired by clients in a short time interval, it is vital to accept connections and initiate new accept operations as quick as possible. So creating connection instance that involves allocations and initialization can be done in a context that is independent to acceptors one. |
| cleanup_func | function-object | [Cleanup function](#markdown-header-сleanup-function). |

## Traits dependent settings

Consider the following aliases
~~~~~
::c++
using request_handler_t = typename Traits::request_handler_t;
~~~~~

In the following table `Params &&...` stands for variadic template parameters
in the following sense:

~~~~~
::c++
template< typename... Params >
auto &
request_handler( Params &&... params )
{
  return set_unique_instance(
      m_request_handler,
      std::forward< Params >( params )... );
}
~~~~~

|      settings        | type | description |
|----------------------|------|-------------|
| request_handler | `std::unique_ptr< request_handler_t >` `Params &&...` | Request handler. |
| timer_factory | `Params &&...` | Timers factory (see [timer](#markdown-header-timer_factory_t)). |
| logger | `Params &&...` | Logger (see [logger](#markdown-header-logger_t)). |

# Cleanup function

## Purpose

One of corner cases of *RESTinio* usage is user's resources cleanup on server's shutdown.
A user can easily create a cyclical dependency between dynamically created objects
which will prevent deallocation of these objects at the end of the server's work.

For example lets consider a case when all request are stored into some thread-safe
storage to be processed later. It could looks like:

~~~~~
::c++
void launch_server(thread_safe_request_queue & pending_requests)
{
  restinio::run(
    restinio::on_this_thread().port(8080).address("localhost")
      .request_handler([&](auto req) {
        pending_requests.push_back(std::move(req));
        return restinio::request_accepted();
      }));
}
~~~~~

This simple code contains a serious defect:
when the server finished its work it will be destroyed just before return from `restinio::run`.
But some `request_handle_t` will live inside `pending_requests` container.
It means that some internal RESTinio objects
(like connections objects and associated socket objects) will be alive,
because there are `request_handle_t` which holds references to them.

A user can add a cleanup of `pending_requests` just after return from `restinio::run` like this:
~~~~~
::c++
void launch_server(thread_safe_request_queue & pending_requests)
{
  restinio::run(
    restinio::on_this_thread().port(8080).address("localhost")
      .request_handler([&](auto req) {
        pending_requests.push_back(std::move(req));
        return restinio::request_accepted();
      }));
  pending_requests.clean();
}
~~~~~
But there are some other problems.
The main of them is: when `request_handle_t` objects from `pending_requests` will be destroyed then
calls to already destroyed `restinio::http_server_t` and corresponding
`asio::io_context` objects can be made.
This can lead to various hard to detect bugs.

It means that user should clean up its resources at the moment of the server's shutdown
just before return from `restinio::run`.
To do that user can set up `cleanup_func` which will be called by the server during shutdown procedure.

By using `cleanup_func` the code above can be rewritten that way:
~~~~~
::c++
void launch_server(thread_safe_request_queue & pending_requests)
{
  restinio::run(
    restinio::on_this_thread().port(8080).address("localhost")
      .request_handler([&](auto req) {
        pending_requests.push_back(std::move(req));
        return restinio::request_accepted();
      })
      .cleanup_func([&]{
        pending_requests.clean();
      }));
}
~~~~~

Note that `cleanup_func` is a part of `server_settings_t`.
It means that it can be used even when RESTinio server is ran via
`restino::http_server_t` instance. For example:
~~~~~
::c++
restinio::http_server_t<> server{
  restinio::own_io_context(),
  [&](auto & settings) {
    settings.port(8080).address("localhost")
      .request_handler(...)
      .cleanup_func([&]{
        pending_requests.clean();
      });
  }};
~~~~~

## When cleanup_func is called

If a user sets `cleanup_func` in `server_settings_t`
then this cleanup function will be called if the server is started and:

* `restinio::http_server_t::close_sync` is called directly or indirectly
(as result of `close_async` call);
* destructor for `restinio::http_server_t` is called when the server wasn't closed manually.
In that case `close_sync` is called inside `http_server_t`'s destructor.

Technically speaking the `cleanup_func` is called inside `close_sync` just after the acceptor will be closed.

**Important notes:** The topic of cleaning up connections,
requests and associated resources is a hard one.
During the work on RESTinio v.0.3 we have found one solution which seems to be working.
But it could be not the best one.
So if you have some troubles with it or have some ideas on this topic please let us know.
We are working hard on this topic and will be glad to hear any feedback from you.

## What is this?

One of corner cases of RESTinio usage is user's resources cleanup on
server's shutdown. A user can easily create a cyclical dependency between
dynamically created objects which will prevent deallocation of these
objects at the end of the server's work.

For example lets consider a case when all request are stored into some
thread-safe storage to be processed later. It could looks like:

~~~~~{.cpp}
void launch_server(thread_safe_request_queue & pending_requests)
{
  restinio::run(
    restinio::on_this_thread().port(8080).address("localhost")
      .request_handler([&](auto req) {
        pending_requests.push_back(std::move(req));
        return restinio::request_accepted();
      }));
}
~~~~~

This simple code contains a serious defect: when the server finished its
work it will be destroyed just before return from `restinio::run`. But some
`request_handle_t` will live inside `pending_requests` container. It means
that some internal RESTinio objects (like connections objects and associated
socket objects) will be alive, because there are `request_handle_t` which
holds references to them.

A user can add a cleanup of `pending_requests` just after return from
`restinio::run` like this:
~~~~~{.cpp}
void launch_server(thread_safe_request_queue & pending_requests)
{
  restinio::run(
    restinio::on_this_thread().port(8080).address("localhost")
      .request_handler([&](auto req) {
        pending_requests.push_back(std::move(req));
        return restinio::request_accepted();
      }));
  pending_requests.clean();
}
~~~~~
But there are some other problems. The main of them is: when `request_handle_t`
objects from `pending_requests` will be destroyed then calls to already
destroyed `restinio::http_server_t` and corresponding `asio::io_context`
objects can be made. This can lead to various hard to detect bugs.

It means that user should clean up its resources at the moment of the
server's shutdown just before return from `restinio::run`. To do that user
can set up `cleanup_func` which will be called by the server during
shutdown procedure.

By using `cleanup_func` the code above can be rewritten that way:
~~~~~{.cpp}
void launch_server(thread_safe_request_queue & pending_requests)
{
  restinio::run(
    restinio::on_this_thread().port(8080).address("localhost")
      .request_handler([&](auto req) {
        pending_requests.push_back(std::move(req));
        return restinio::request_accepted();
      })
      .cleanup_func([&]{
        pending_requests.clean();
      }));
}
~~~~~

Note that `cleanup_func` is a part of `server_settings_t`. It means
that it can be used even when RESTinio server is ran via
`restino::http_server_t` instance. For example:
~~~~~{.cpp}
restinio::http_server_t<> server{
  restinio::own_io_context(),
  [&](auto & settings) {
    settings.port(8080).address("localhost")
      .request_handler(...)
      .cleanup_func([&]{
        pending_requests.clean();
      });
  }};
~~~~~

## When cleanup_func is called

If a user sets `cleanup_func` in `server_settings_t` then this cleanup
function will be called if the server is started and:

* `restinio::http_server_t::close_sync` is called directly or indirectly (as result of `close_async` call);
* destructor for `restinio::http_server_t` is called when the server wasn't closed manually.
In that case `close_sync` is called inside `http_server_t`'s destructor.

Technically speaking the `cleanup_func` is called inside `close_sync` just after
the acceptor will be closed.

## Important notes

The topic of cleaning up connections, requests and associated resources is a
hard one. During the work on RESTinio v.0.3 we have found one solution which
seems to be working. But it could be not the best one. So if you have some
troubles with it or have some ideas on this topic please let us know. We are
working hard on this topic and will be glad to hear any feedback from you.

# Express router

One of the reasons to create *RESTinio* was an ability to have
[express](https://expressjs.com/)-like request handler router.

Since v 0.2.1 *RESTinio* has a router based on idea borrowed
from [express](https://expressjs.com/) - a JavaScript framework.

Routers acts as a request handler (it means it is a function-object
that can be called as a request handler).
But router aggregates several handlers and picks one or none of them to handle the request.
The choice of the handler to execute depends on request target and HTTP method.
If router finds no handler matching request then it rejects it.
There is a difference between ordinary restinio request handler
and the one that is used with experss router and is bound to concrete endpoint.
The signature of a handlers that can be put in router
has an additional parameter -- a container with parameters extracted from URI.

Express router is defined by `express_router_t` class.
Its implementation is inspired by
[express-router](https://expressjs.com/en/starter/basic-routing.html).
It allows to define route path with injection
of parameters that become available for handlers.
For example the following code sets a handler with 2 parameters:
~~~~~
::c++
  router.http_get(
    R"(/article/:article_id/:page(\d+))",
    []( auto req, auto params ){
      const auto article_id = params[ "article_id" ];
      auto page = std::to_string( params[ "page" ] );
      // ...
    } );
~~~~~

Note that express handler receives 2 parameters not only request handle
but also `route_params_t` instance that holds parameters of the request:
```
::c++
using express_request_handler_t =
    std::function< request_handling_status_t( request_handle_t, route_params_t ) >;
```

Route path defines a set of named and indexed parameters.
Named parameters starts with `:`, followed by non-empty parameter name
(only A-Za-z0-9_ are allowed). After parameter name it is possible to
set a capture regex enclosed in brackets
(actually a subset of regex - none of the group types are allowed).
Indexed parameters are simply a capture regex in brackets.

Let's show how it works by example.
First let's assume that variable `router` is a pointer to express router.
So that is how we add a request handler with a single parameter:
~~~~~
::c++

  // GET request with single parameter.
  router->http_get( "/single/:param", []( auto req, auto params ){
    return
      init_resp( req->create_response() )
        .set_body( "GET request with single parameter: " + params[ "param" ] )
        .done();
  } );
~~~~~


The following requests will be routed to that handler:

* http://localhost/single/123 param="123"
* http://localhost/single/parameter/ param="parameter"
* http://localhost/single/another-param param="another-param"

But the following will not:

* http://localhost/single/123/and-more
* http://localhost/single/
* http://localhost/single-param/123

Let's use more parameters and assign a capture regex for them:
~~~~~
::c++
  // POST request with several parameters.
  router->http_post( R"(/many/:year(\d{4}).:month(\d{2}).:day(\d{2}))",
    []( auto req, auto params ){
      return
        init_resp( req->create_response() )
          .set_body( "POST request with many parameters:\n"
            "year: "+ params[ "year" ] + "\n" +
            "month: "+ params[ "month" ] + "\n" +
            "day: "+ params[ "day" ] + "\n"
            "body: " + req->body() )
          .done();
    } );
~~~~~

The following requests will be routed to that handler:

* http://localhost/many/2017.01.01 year="2017", month="01", day="01"
* http://localhost/many/2018.06.03 year="2018", month="06", day="03"
* http://localhost/many/2017.12.22 year="2017", month="12", day="22"

But the following will not:

* http://localhost/many/2o17.01.01
* http://localhost/many/2018.06.03/events
* http://localhost/many/17.12.22

Using indexed parameters is practically the same, just omit parameters names:
~~~~~
::c++
  // GET request with indexed parameters.
  router->http_get( R"(/indexed/([a-z]+)-(\d+)/(one|two|three))",
    []( auto req, auto params ){
      return
        init_resp( req->create_response() )
          .set_body( "POST request with indexed parameters:\n"
            "#0: "+ params[ 0 ] + "\n" +
            "#1: "+ params[ 1 ] + "\n" +
            "#2: "+ params[ 2 ] + "\n" )
          .done();
    } );
~~~~~

The following requests will be routed to that handler:

* http://localhost/indexed/xyz-007/one #0="xyz", #1="007", #2="one"
* http://localhost/indexed/ABCDE-2017/two #0="ABCDE", #1="2017", #2="two"
* http://localhost/indexed/sobjectizer-5/three #0="sobjectizer", #1="5", #2="three"

But the following will not:

* http://localhost/indexed/xyz-007/zero
* http://localhost/indexed/173-xyz/one
* http://localhost/indexed/ABCDE-2017/one/two/three

See full [example](./dev/sample/express_router_tutorial/main.cpp)

For details on `route_params_t` and `express_router_t` see
[express.hpp](./dev/restinio/router/express.cpp).

# *RESTinio* context entities running on asio::io_context

*RESTinio* runs its logic on `asio::io_context`.
*RESTinio* works with asio on the base of callbacks,
that means tha some context is always passed from one callback to another.
There are two main entities the contexts of which is passed between callbacks:

* acceptor -- receives new connections and creates connection objects that
performs session logic;
* connection -- does tcp io-operations, http-parsing and calls handler.


### Acceptor

There is a single instance of acceptor and as much connections as needed.

Acceptors life cycle is trivial and is the following:

1. Start listening for new connection.
2. Receive new tcp-connection.
3. Create connection handler object and start running it.
4. Back to step 1'.

When the server is closed cycle breaks up.

To set custom options for acceptor use `server_settings_t::acceptor_options_setter()`.

By default *RESTinio* accepts connections one-by-one,
so a big number of clients initiating simultaneous connections might be a problem
even when running `asio::io_context` on a pool of threads.
There are a number of options to tune *RESTinio* for such cases.

* Increase the number of concurrent accepts. By default *RESTinio*
initiates only one accept operation, but when running server on
N threads then up to N accepts can be handled concurrently.
See `server_settings_t::concurrent_accepts_count()`.
* After accepting new connection on socket *RESTinio* creates
internal connection wrapper object. The creation of such object can
be done separately (in another callback posted on asio).
So creating connection instance that involves allocations
and initialization can be done in a context that is independent to acceptors one.
It makes on-accept callback to run faster, thus more connections can be
accepted in the same time interval.
See `server_settings_t::separate_accept_and_create_connect()`

Example of using acceptor options:
~~~~~
::c++
// using traits_t = ...
restinio::http_server_t< traits_t >
  server{
    restinio::create_child_io_context( 4 ),
    restinio::server_settings_t< traits_t >{}
      .port( port )
      .buffer_size( 1024 )
      .max_pipelined_requests( 4 )
      .request_handler( db )
      // Using acceptor options:
      .acceptor_options_setter(
        []( auto & options ){
          options.set_option( asio::ip::tcp::acceptor::reuse_address( true ) );
        } )
      .concurrent_accepts_count( 4 )
      .separate_accept_and_create_connect( true ) };
~~~~~

### Connection

Connections life cycle is more complicated and cannot be expressed lineary.
Simultaneously connection runs two logical objectives. The first one is
responsible for receiving requests and passing them to handler (read part) and
the second objective is streaming resulting responses back to client (write part).
Such logical separation comes from HTTP pipelining support and
various types of response building strategies.

Without error handling and timeouts control Read part looks like this:

1. Start reading from socket.
2. Receive a portion of data from socket and parse HTTP request out of it.
3. If HTTP message parsing is incomplete then go back to step 1.
4. If HTTP message parsing is complete pass request and connection to request handler.
5. If request handler rejects request, then push not-implemented response (status 501)
to outgoing queue and stop reading from socket.
5. If request was accepted and the number of requests in process is less than
`max_pipelined_requests` then go back to step 1.
6. Stop reading socket until awaken by the write part.

And the Write part looks like this:

1. Wait for response pieces initiated from user domain
either directly inside of handler call or from other context where
response actually is being built.
2. Push response data to outgoing queue with consideration of associated response position
(multiple request can be in process, and response for a given request
cannot be written to socket before writing all previous responses to it).
3. Check if there is outgoing data ready to send.
4. If there is no ready data available then go back to step 1.
5. Send ready data.
6. Wait for write operation to complete. If more response pieces comes while
write operation runs it is simply received (steps 1-2 without any further go).
7. After write operation completes:
if last committed response was marked to close connection
then connection is closed and destroyed.
8. If it appears that the room for more pipeline requests became available again
then awake the read part.
9. Go back to step 3.

Of course implementation has error checks. Also implementation controls timeouts of
operations that are spread in time:

* reading the request: from starting reading bytes from socket up to
parsing a complete http-message;
* handling the request: from passing request data and connection handle
to request handler up to getting response to be written to socket;
* writing response to socket.

When handling a request there are two possible cases:

* response is created inside the request handlers call;
* request handler delegates handling job to other context via
some kind of async API.

The first case is trivial and response is simply begins to be written.

The second case and its possibility is a key point of *RESTinio* being created for.
As request data and connection handle are wrapped in shared pointers
so they can be moved to other context.
So it is possible to create handlers that can interact with async API.
When response data is ready response can be built and sent using request handle.
After response building is complete connection handle
will post the necessary job to run on host `asio::io_context`.
So one can perform asynchronous request handling and
not to block worker threads.

To set custom options for acceptor use `server_settings_t::socket_options_setter()`:
~~~~~
::c++
// using traits_t = ...
restinio::http_server_t< traits_t >
  server{
    restinio::create_child_io_context( 4 ),
    restinio::server_settings_t< traits_t >{}
      .port( port )
      .buffer_size( 1024 )
      .max_pipelined_requests( 4 )
      .request_handler( db )
      // Using custom socket options:
      .socket_options_setter(
        []( auto & options ){
          options.set_option( asio::ip::tcp::no_delay{ true } );
        } ) };
~~~~~

# Using external io_context

*RESTinio* can run its logic on externals io_context.
So, for example, it is possible to run more than one server using the same io_context:
~~~~~
::c++
// External io_context.
asio::io_context io_context;

using server_t = restinio::http_server_t<>;
using settings_t = restinio::server_settings_t<>;

server_t srv1{
  restinio::external_io_context( io_context ),
  settings_t{}
    .port( 8080 )
    .address( "localhost" )
    .request_handler( create_request_handler( "server1" ) ) };

server_t srv2{
  restinio::external_io_context( io_context ),
  settings_t{}
    .port( 8081 )
    .address( "localhost" )
    .request_handler( create_request_handler( "server2" ) ) };

asio::signal_set break_signals{ io_context, SIGINT };
break_signals.async_wait(
  [&]( const asio::error_code & ec, int ){
    if( !ec )
    {
      srv1.close_sync();
      srv2.close_sync();
    }
  } );

asio::post( io_context, [&] {
  srv1.open_sync();
  srv2.open_sync();
});

io_context.run();
~~~~~

Helper function `restinio::external_io_context()`
create such io_context holder that passes to server only its reference.

See also a full [sample](./dev/sample/using_external_io_context/main.cpp).

# Buffers

RESTinio has a capability to receive not only string buffers but also
constant and custom buffers. Message builders has
body setters methods (set_body(), append_body(), append_chunk())
with an argument of a type `buffer_storage_t`
(see [buffers.hpp](./dev/restinio/buffers.hpp) for more details).
`buffer_storage_t` is a wrapper for different type of buffers
that creates `asio::const_buffer` out of different implementations:

* const buffers based on data pointer and data size;
* string buffers based on `std::string`;
* shared buffer - a shared_ptr on an object with data-size interface:
 `std::shared_ptr< Buf >` where `Buf` has `data()` and `size()`
 methods returning `void*` (or convertible to it) and
`size_t` (or convertible to).

Const buffers are intended for cases when the data is defined
as a constant char sequence and its lifetime is guaranteed to be long enough
(for example a c-strings defined globally).
To make the usage of const buffers safer `buffer_storage_t` constructors
don't accept pointer and size params directly, and to instantiate
a `buffer_storage_t` object that refers to const buffers a helper `const_buffer_t`
class must be used. There is a helper function `const_buffer()` that helps to create
`const_buffer_t`. Let's have a look on a clarifying example:

~~~~~
::c++
// Request handler:
[]( restinio::request_handle_t req ){
  // Create response builder.
  auro resp = req->create_response();

  const char * resp = "0123456789 ...";

  // Set response part as const buffer.
  resp.set_body( restinio::const_buffer( resp ) ); // OK, size will be calculated with std::strlen().
  resp.set_body( restinio::const_buffer( resp, 4 ) ); // OK, size will be 4.

  // When not using restinio::const_buffer() helper function
  // char* will be treated as a parameter for std::string constructor.
  resp.set_body( resp ); // OK, but std::string will be actually used.

  const std::string temp{ "watch the lifetime, please" };

  // Using a temporary source for const buffer.
  resp.set_body( restinio::const_buffer( temp.data(), temp.size() ) ); // BAD!

  // Though using a temporary source directly is OK.
  resp.set_body( temp ); // OK, will create a copy of the string.

  // Though using a temporary source directly is OK.
  resp.set_body( temp ); // OK, will create a copy of the string.

  // ...
}
~~~~~

The simplest option is to use std::string. Passed string is copied or moved if possible.

The third option is to use shared (custom) buffers wrapped in shared_ptr:
`std::shared_ptr< Buffer >`. `Buffer` type is  required to have data()/size()
member functions, so it is possible to obtain a pointer to data and data size.
For example `std::shared_ptr< std::string >` can be used.
Such form of buffers was introduced for dealing with the cases
when there are lots of parallel requests that must be served with the same response
(or partly the same, so identical parts can be wrapped in shared buffers).

# TLS support

Restinio support HTTS using ASIO ssl facilities (based on OpenSSL).

To create https server it is needed to include extra header file `restinio/tls.hpp`.
This file contains necessary customization classes and structs
that make `restinio::http_server_t` usable as https server.
For specializing `restinio::http_server_t` to work as https server
one should use `restinio::tls_traits_t` (or `restinio::single_thread_tls_traits_t`)
for it and also it is vital to set TLS context using `asio::ssl::context`.
That setting is added to `server_settings_t` class instantiated with TLS traits.

Lets look through an example:
~~~~~
::c++
// ...
using traits_t =
  restinio::single_thread_tls_traits_t<
    restinio::asio_timer_factory_t,
    restinio::single_threaded_ostream_logger_t,
    router_t >;

asio::ssl::context tls_context{ asio::ssl::context::sslv23 };
tls_context.set_options(
  asio::ssl::context::default_workarounds
  | asio::ssl::context::no_sslv2
  | asio::ssl::context::single_dh_use );

tls_context.use_certificate_chain_file( certs_dir + "/server.pem" );
tls_context.use_private_key_file(
  certs_dir + "/key.pem",
  asio::ssl::context::pem );
tls_context.use_tmp_dh_file( certs_dir + "/dh2048.pem" );

restinio::run(
  restinio::on_this_thread< traits_t >()
    .address( "localhost" )
    .request_handler( server_handler() )
    .read_next_http_message_timelimit( 10s )
    .write_http_response_timelimit( 1s )
    .handle_request_timeout( 1s )
    .tls_context( std::move( tls_context ) ) );

// ...
~~~~~

See full [sample](./dev/sample/hello_world_https/main.cpp) for details.


# Version history

|       Feature        | description | release  |
|----------------------|-------------|----------|
| Running server simplification | Add functions to deal with boilerplate code for running server in simple cases | 0.3.0 |
| Improve internal design | Redesign server start/stop logic | 0.3.0 |
| Web Sockets | Basic support for Web Sockets | 0.3.0 |
| Acceptor options | Custom options for socket can be set in settings. | 0.3.0 |
| Separate accept and create connection | creating connection instance that involves allocations and initialization can be done in a context that is independent to acceptors context. | 0.3.0 |
| Concurrent accept | Server can accept several client connections concurrently. | 0.3.0 |
| Add uri helpers | A number of functions to work with query string see [uri_helpers.hpp](./dev/restinio/uri_helpers.hpp). | 0.3.0 |
| Improve header fields API | Type/enum support for known header fields and their values. | 0.2.2 |
| TLS support | Sopport for HTTPS with OpenSSL | 0.2.2 |
| External buffers | Support external (constant) buffers support for body and/or body parts. | 0.2.2 |
| Benchmarks | Non trivial benchmarks. Comparison with other libraries with similar features on the range of various scenarios. | started independent [project](https://bitbucket.org/sobjectizerteam/restinio-benchmark-jun2017) |
| Routers for message handlers | Support for a URI dependent routing to a set of handlers (express-like router). | 0.2.1 |
| Bind localhost aliases | Accept "localhost" and "ip6-localhost" as address parameter for server to bound to.  | 0.2.1 |
| Chunked transfer encoding | Support for chunked transfer encoding. Separate responses on header and body chunks, so it will be possible to send header and then body divided on chunks. | 0.2.0 |
| HTTP pipelining | [HTTP pipelining](https://en.wikipedia.org/wiki/HTTP_pipelining) support. Read, parse and call a handler for incoming requests independently. When responses become available send them to client in order of corresponding requests. | 0.2.0 |
| Address binding | Bind server to specific ip address. | 0.1.0 |
| Timeout control | Enable timeout guards for operations of receiving request (read and parse complete request), handling request, write response | 0.1.0 |
| Logging | Support for logging of internal server work. | 0.1.0 |
| ASIO thread pool | Support ASIO running on a thread pool. | 0.1.0 |
| IPv6 | IPv6 support. | 0.1.0 |

# License

*RESTinio* is distributed under BSD-3-CLAUSE license (see [LICENSE](./LICENSE) file).

For the license of *asio* library see COPYING file in *asio* distributive.

For the license of *nodejs/http-parser* library
see LICENSE file in *nodejs/http-parser* distributive.

For the license of *fmtlib* see LICENSE file in *fmtlib* distributive.

For the license of *SObjectizer* library see LICENSE file in *SObjectizer* distributive.

For the license of *rapidjson* library see LICENSE file in *rapidjson* distributive.

For the license of *json_dto* library see LICENSE file in *json_dto* distributive.

For the license of *args* library see LICENSE file in *args* distributive.

For the license of *CATCH* library see LICENSE file in *CATCH* distributive.
