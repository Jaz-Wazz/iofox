#pragma once

#include <boost/beast/core/flat_buffer.hpp>
#include <boost/beast/core/tcp_stream.hpp>
#include <boost/beast/ssl/ssl_stream.hpp>
#include <boost/beast/http/write.hpp>
#include <boost/beast/http/read.hpp>
#include <boost/asio/ip/address.hpp>
#include <boost/asio/this_coro.hpp>
#include <boost/url/urls.hpp>
#include <iofox/core.hpp>
#include <string_view>
#include <stdexcept>
#include <chrono>
#include <string>

#define beast boost::beast
#define asio boost::asio
#define this_coro asio::this_coro

namespace io::http
{
	template <typename T = std::string>
	inline auto send
	(
		beast::ssl_stream<beast::tcp_stream> & stream,
		const auto & request,
		const std::chrono::steady_clock::duration timeout = std::chrono::seconds(60)
	)
	-> io::coro<io::http::response<T>>
	{
		stream.next_layer().expires_after(timeout);
		co_await beast::http::async_write(stream, request, io::use_coro);
		io::http::response<T> response;
		beast::flat_buffer buffer;
		co_await beast::http::async_read(stream, buffer, response, io::use_coro);
		stream.next_layer().expires_never();

		co_return response;
	}

	template <typename T = std::string>
	inline auto send
	(
		const boost::url url,
		const auto & request,
		const std::chrono::steady_clock::duration timeout = std::chrono::seconds(60)
	)
	-> io::coro<io::http::response<T>>
	{
		if(url.scheme() != "https") throw std::runtime_error("unsupported_protocol");
		auto stream = co_await io::open_https_stream(url.host());
		co_return co_await io::http::send<T>(stream, request, timeout);
	}

	template <typename T = std::string>
	inline auto send
	(
		const std::string_view url,
		const auto & request,
		const std::chrono::steady_clock::duration timeout = std::chrono::seconds(60)
	)
	-> io::coro<io::http::response<T>>
	{
		co_return co_await io::http::send<T>(boost::url(url), request, timeout);
	}

	template <typename T = std::string>
	inline auto send
	(
		const boost::url proxy,
		const boost::url url,
		const auto & request,
		const std::chrono::steady_clock::duration timeout = std::chrono::seconds(60)
	)
	-> io::coro<io::http::response<T>>
	{
		if(proxy.scheme() != "https") throw std::runtime_error("unsupported_proxy_protocol");
		if(url.scheme() != "https") throw std::runtime_error("unsupported_protocol");

		beast::tcp_stream stream {co_await this_coro::executor};
		stream.expires_after(std::chrono::seconds(14));
		co_await stream.async_connect({asio::ip::make_address(proxy.host()), proxy.port_number()}, io::use_coro);
		stream.expires_never();

		beast::ssl_stream<beast::tcp_stream> tunnel = co_await io::http::proxy::open_ssl_tunnel(stream, url.host());
		co_await io::ssl::handshake_http_client(tunnel, url.host());
		co_return co_await io::http::send<T>(tunnel, request, timeout);
	}

	template <typename T = std::string>
	inline auto send
	(
		const boost::url proxy,
		const std::string_view url,
		const auto & request,
		const std::chrono::steady_clock::duration timeout = std::chrono::seconds(60)
	)
	-> io::coro<io::http::response<T>>
	{
		co_return co_await io::http::send<T>(proxy, boost::url(url), request, timeout);
	};
}

#undef beast
#undef asio
#undef this_coro
