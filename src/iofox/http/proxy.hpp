#pragma once

#include <boost/beast/core/flat_buffer.hpp>
#include <boost/beast/core/tcp_stream.hpp>
#include <boost/beast/ssl/ssl_stream.hpp>
#include <boost/beast/http/empty_body.hpp>
#include <boost/beast/http/parser.hpp>
#include <boost/beast/http/write.hpp>
#include <boost/beast/http/read.hpp>
#include <iofox/core.hpp>
#include <exception>
#include <utility>
#include <chrono>
#include <string>

#define prv private:
#define pbl public:
#define beast boost::beast

namespace io::http::proxy
{
	class bad_ssl_tunnel: public std::exception
	{
		prv int value;

		pbl bad_ssl_tunnel(int code)
		: value(code) {}

		auto what() const noexcept -> const char *
		{
			return "bad_ssl_tunnel";
		}

		auto code() const -> const int
		{
			return value;
		}
	};

	inline auto open_ssl_tunnel
	(
		beast::tcp_stream & stream,
		std::string host,
		const std::chrono::steady_clock::duration timeout = std::chrono::seconds(60)
	) -> io::coro<beast::ssl_stream<beast::tcp_stream>>
	{
		stream.expires_after(timeout);

		// Send connect request.
		io::http::request request {"CONNECT", host + ":443"};
		co_await beast::http::async_write(stream, request, io::use_coro);

		// Read connect response headers.
		beast::flat_buffer buffer;
		beast::http::response_parser<beast::http::empty_body> parser;
		co_await beast::http::async_read_header(stream, buffer, parser, io::use_coro);

		stream.expires_never();

		// Check status.
		if(parser.get().result_int() != 200) throw bad_ssl_tunnel(parser.get().result_int());

		// Construct overlying stream.
		co_return beast::ssl_stream<beast::tcp_stream>(std::move(stream), co_await io::ssl::context());
	}
}

#undef prv
#undef pbl
#undef beast
