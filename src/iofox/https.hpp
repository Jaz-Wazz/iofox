#pragma once

// boost_asio
#include <boost/asio/any_io_executor.hpp>
#include <boost/asio/ssl/context.hpp>
#include <boost/asio/this_coro.hpp>

// boost_beast
#include <boost/beast/core/tcp_stream.hpp>
#include <boost/beast/ssl/ssl_stream.hpp>

// stl
#include <chrono>
#include <string>

// iofox
#include <iofox/coro.hpp>
#include <iofox/dns.hpp>
#include <iofox/ssl.hpp>

namespace iofox
{
	inline auto open_https_stream(const boost::asio::any_io_executor & executor, boost::asio::ssl::context & context, const std::string & host)
	-> iofox::coro<boost::beast::ssl_stream<boost::beast::tcp_stream>>
	{
		boost::beast::ssl_stream<boost::beast::tcp_stream> stream {executor, context};

		stream.next_layer().expires_after(std::chrono::seconds(19));
		co_await stream.next_layer().async_connect(co_await iofox::dns::resolve("https", host), iofox::use_coro);
		stream.next_layer().expires_never();

		co_await iofox::ssl::handshake_http_client(stream, host);
		co_return stream;
	}

	inline auto open_https_stream(const std::string & host) -> iofox::coro<boost::beast::ssl_stream<boost::beast::tcp_stream>>
	{
		co_return co_await iofox::open_https_stream(co_await boost::asio::this_coro::executor, co_await iofox::ssl::context(), host);
	}
}
