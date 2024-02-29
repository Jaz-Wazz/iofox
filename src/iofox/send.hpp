#pragma once

// boost_beast
#include <boost/beast/core/flat_buffer.hpp>
#include <boost/beast/core/tcp_stream.hpp>
#include <boost/beast/http/read.hpp>
#include <boost/beast/http/write.hpp>
#include <boost/beast/ssl/ssl_stream.hpp>

// boost_url
#include <boost/url/urls.hpp>

// stl
#include <chrono>
#include <stdexcept>
#include <string>
#include <string_view>

// iofox
#include <iofox/coro.hpp>
#include <iofox/http.hpp>
#include <iofox/https.hpp>

namespace iofox::http
{
	template <typename T = std::string>
	inline auto send
	(
		boost::beast::ssl_stream<boost::beast::tcp_stream> & stream,
		const auto & request,
		const std::chrono::steady_clock::duration timeout = std::chrono::seconds(60)
	)
	-> iofox::coro<iofox::http::response<T>>
	{
		stream.next_layer().expires_after(timeout);
		co_await boost::beast::http::async_write(stream, request, iofox::use_coro);
		iofox::http::response<T> response;
		boost::beast::flat_buffer buffer;
		co_await boost::beast::http::async_read(stream, buffer, response, iofox::use_coro);
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
	-> iofox::coro<iofox::http::response<T>>
	{
		if(url.scheme() != "https") throw std::runtime_error("unsupported_protocol");
		auto stream = co_await iofox::open_https_stream(url.host());
		co_return co_await iofox::http::send<T>(stream, request, timeout);
	}

	template <typename T = std::string>
	auto send
	(
		const std::string_view url,
		const auto & request,
		const std::chrono::steady_clock::duration timeout = std::chrono::seconds(60)
	)
	-> iofox::coro<iofox::http::response<T>>
	{
		co_return co_await iofox::http::send<T>(boost::url(url), request, timeout);
	}

	extern template auto send<std::string>
	(
		const std::string_view url,
		const iofox::http::request<void> & request,
		const std::chrono::steady_clock::duration timeout
	)
	-> iofox::coro<iofox::http::response<std::string>>;
}
