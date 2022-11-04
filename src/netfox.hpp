#pragma once
#include <boost/asio/as_tuple.hpp>
#include <boost/asio/awaitable.hpp>
#include <boost/asio/connect.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ssl/stream.hpp>
#include <boost/asio/this_coro.hpp>
#include <boost/asio/use_awaitable.hpp>
#include <boost/beast/core/flat_buffer.hpp>
#include <boost/beast/http/buffer_body.hpp>
#include <boost/beast/http/error.hpp>
#include <boost/beast/http/parser.hpp>
#include <boost/beast/http/read.hpp>
#include <boost/beast/http/serializer.hpp>
#include <boost/beast/http/write.hpp>
#include <expected>
#include <jaja_notation.hpp>
#include <optional>

namespace netfox
{
	enum class side {client, server};
}

namespace netfox::detail
{
	namespace asio		= boost::asio;				// NOLINT
	namespace this_coro = boost::asio::this_coro;	// NOLINT
	namespace beast		= boost::beast;				// NOLINT
	namespace http		= boost::beast::http;		// NOLINT

	template <side> class http_stream;

	template <> class http_stream<side::client>
	{
		prv std::optional<asio::ip::tcp::socket> stream;
		prv http::response_parser<http::buffer_body> parser;
		prv std::optional<http::request<http::buffer_body>> serializer_request;
		prv std::optional<http::request_serializer<http::buffer_body>> serializer;
		prv beast::flat_buffer buf;

		pbl auto connect(auto host) -> asio::awaitable<void>
		{
			auto ctx = co_await this_coro::executor;
			stream.emplace(ctx);
			auto resolver_results = co_await asio::ip::tcp::resolver(ctx).async_resolve(host, "http", asio::use_awaitable);
			co_await asio::async_connect(stream->lowest_layer(), resolver_results, asio::use_awaitable);
		}

		pbl auto write_request(auto & request) noexcept -> asio::awaitable<std::expected<void, int>>
		{
			co_await http::async_write(*stream, request, asio::use_awaitable);
		}

		pbl auto write_request_header(auto & header) -> asio::awaitable<void>
		{
			serializer_request.emplace(header);
			serializer.emplace(*serializer_request);
			co_await http::async_write_header(*stream, *serializer, asio::use_awaitable);
		}

		pbl auto write_request_body();
		pbl auto write_request_body_chunk();

		pbl auto read_response(auto & response) -> asio::awaitable<void>
		{
			co_await http::async_read(*stream, buf, response, asio::use_awaitable);
		}

		pbl auto read_response_header(auto & header) -> asio::awaitable<void>
		{
			co_await http::async_read_header(*stream, buf, parser, asio::use_awaitable);
			header = parser.get().base();
		}

		pbl auto read_response_body();

		pbl auto read_response_body_chunk(char * buffer, std::size_t size) -> asio::awaitable<std::optional<std::size_t>>
		{
			if(!parser.is_done())
			{
				parser.get().body().data = buffer;
				parser.get().body().size = size;
				auto [err, bytes_readed] = co_await http::async_read(*stream, buf, parser, asio::as_tuple(asio::use_awaitable));
				if(err.failed() && err != http::error::need_buffer) throw err;
				co_return size - parser.get().body().size;
			} else co_return std::nullopt;
		}
	};
}

namespace netfox
{
	namespace http
	{
		template <side side> using stream = detail::http_stream<side>;
	}
}
