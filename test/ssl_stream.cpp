// boost_asio
#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ssl/context.hpp>

// boost_system
#include <boost/system/error_code.hpp>

// openssl
#include <openssl/ssl.h>

// iofox
#include <iofox/ssl_stream.hpp>

// catch2
#include <catch2/catch_test_macros.hpp>

TEST_CASE("ssl_stream")
{
	boost::asio::io_context io_context;
	boost::asio::ssl::context ssl_context {boost::asio::ssl::context::tls};

	SECTION("constructor")
	{
		SSL * ssl_stream_native_handle = SSL_new(ssl_context.native_handle());

		iofox::ssl_stream ssl_stream_x {io_context, ssl_context};
		iofox::ssl_stream ssl_stream_y {io_context, ssl_stream_native_handle};
	}

	SECTION("move_constructor")
	{
		iofox::ssl_stream ssl_stream_iofox {io_context, ssl_context};
		boost::asio::ssl::stream<boost::asio::ip::tcp::socket> ssl_stream_asio {io_context, ssl_context};

		iofox::ssl_stream ssl_stream_x = std::move(ssl_stream_iofox);
		iofox::ssl_stream ssl_stream_y = std::move(ssl_stream_asio);
	}

	SECTION("move_assigment")
	{
		iofox::ssl_stream ssl_stream {io_context, ssl_context};
		iofox::ssl_stream ssl_stream_iofox {io_context, ssl_context};
		boost::asio::ssl::stream<boost::asio::ip::tcp::socket> ssl_stream_asio {io_context, ssl_context};

		ssl_stream = std::move(ssl_stream_iofox);
		ssl_stream = std::move(ssl_stream_asio);
	}

	SECTION("set_hostname")
	{
		iofox::ssl_stream ssl_stream {io_context, ssl_context};
		boost::system::error_code ec;

		ssl_stream.set_tlsext_hostname("exmaple.com");
		ssl_stream.set_tlsext_hostname("exmaple.com", ec);
		REQUIRE(!ec.failed());
	}
}
