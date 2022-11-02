#include <boost/asio/as_tuple.hpp>
#include <boost/asio/awaitable.hpp>
#include <boost/asio/buffer.hpp>
#include <boost/asio/co_spawn.hpp>
#include <boost/asio/connect.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ssl/context.hpp>
#include <boost/asio/steady_timer.hpp>
#include <boost/asio/this_coro.hpp>
#include <boost/asio/use_awaitable.hpp>
#include <boost/beast/core/error.hpp>
#include <boost/beast/core/flat_buffer.hpp>
#include <boost/beast/core/tcp_stream.hpp>
#include <boost/beast/http/buffer_body.hpp>
#include <boost/beast/http/detail/type_traits.hpp>
#include <boost/beast/http/dynamic_body.hpp>
#include <boost/beast/http/empty_body.hpp>
#include <boost/beast/http/file_body.hpp>
#include <boost/beast/http/impl/file_body_win32.hpp>
#include <boost/beast/http/message.hpp>
#include <boost/beast/http/parser.hpp>
#include <boost/beast/http/read.hpp>
#include <boost/beast/http/serializer.hpp>
#include <boost/beast/http/status.hpp>
#include <boost/beast/http/string_body.hpp>
#include <boost/beast/http/vector_body.hpp>
#include <boost/beast/http/verb.hpp>
#include <boost/beast/http/write.hpp>
#include <chrono>
#include <cstddef>
#include <fmt/core.h>
#include <functional>
#include <iostream>
#include <httpfox.hpp>
#include <iterator>
#include <optional>
#include <stdexcept>
#include <string>
#include <utility>
#include <expected>
#include <jaja_notation.hpp>

namespace asio = boost::asio;					// NOLINT
namespace beast = boost::beast;					// NOLINT
namespace http = boost::beast::http;			// NOLINT
namespace this_coro = boost::asio::this_coro;	// NOLINT

auto perform_http(auto host, auto & request, auto & response) -> asio::awaitable<void>
{
	auto ctx = co_await this_coro::executor;
	asio::ip::tcp::resolver resolver {ctx};
	auto resolver_results = co_await resolver.async_resolve(host, "http", asio::use_awaitable);
	asio::ip::tcp::socket sock {ctx};
	co_await asio::async_connect(sock, resolver_results, asio::use_awaitable);
	request.set("Host", host);
	co_await beast::http::async_write(sock, request, asio::use_awaitable);
	beast::flat_buffer buffer;
	co_await beast::http::async_read(sock, buffer, response, asio::use_awaitable);
};

auto take_archive_location(std::string path) -> asio::awaitable<std::expected<std::pair<std::string, std::string>, int>>
{
	beast::http::request<beast::http::empty_body> request {beast::http::verb::get, "/download" + path, 11};
	beast::http::response<beast::http::string_body> response;
	co_await perform_http("archive.org", request, response);
	if(response.result_int() == 302)
	{
		auto location = response.at("Location");
		co_return std::make_pair(location.substr(7, location.find('/', 7) - 7), location.substr(location.find('/', 7)));
	}
	else co_return std::unexpected(response.result_int());
}

auto session(beast::tcp_stream stream_user) -> asio::awaitable<void>
{
	http::response<http::string_body> response;
	http::response_serializer<http::string_body> serializer {response};

	response.chunked(true);
	for(int i = 0; i < 50000; i++) response.body() += "00000000000000000000000000000000";

	auto [err, bytes_writed] = co_await http::async_write(stream_user, serializer, asio::as_tuple(asio::use_awaitable));
	if(err.failed() && err != beast::http::error::need_buffer) throw err;

	stream_user.socket().shutdown(asio::ip::tcp::socket::shutdown_send);
	fmt::print("done.\n");

	// // Data.
	// auto host = "ia902302.us.archive.org";
	// auto path = "/22/items/1_20211016_20211016/1.png";

	// // Connect to archive.
	// auto resolver_results = co_await asio::ip::tcp::resolver(co_await this_coro::executor).async_resolve(host, "http", asio::use_awaitable);
	// asio::ip::tcp::socket sock_archive {co_await this_coro::executor};
	// co_await asio::async_connect(sock_archive, resolver_results, asio::use_awaitable);

	// // Send request to archive.
	// http::request<http::empty_body> request_archive {http::verb::get, path, 11};
	// request_archive.set("Host", host);
	// co_await http::async_write(sock_archive, request_archive, asio::use_awaitable);

	// // Read response header from archive.
	// http::response_parser<http::buffer_body> parser_archive_response;
	// beast::flat_buffer buf;
	// co_await http::async_read_header(sock_archive, buf, parser_archive_response, asio::use_awaitable);
	// std::cout << parser_archive_response.get().base() << '\n';

	// // Prepare user response data.
	// http::response<http::buffer_body> response_user;
	// http::response_serializer<http::buffer_body> serializer_user {response_user};

	// // Incremental read from archive -> Incremental write to user.
	// while(!parser_archive_response.is_done())
	// {
	// 	char transmission_buf[128];
	// 	parser_archive_response.get().body().data = transmission_buf;
	// 	parser_archive_response.get().body().size = 128;
	// 	auto [err1, bytes_readed] = co_await http::async_read_some(sock_archive, buf, parser_archive_response, asio::as_tuple(asio::use_awaitable));
	// 	// for(int i = 0; i < 128; i++)
	// 	// {
	// 	// 	std::cout << transmission_buf[i];
	// 	// }
	// 	if(err1.failed() && err1 != http::error::need_buffer) throw err1;
	// 	fmt::print("[reader] - read {} bytes.\n", bytes_readed);

	// 	response_user.body().data = transmission_buf;
	// 	response_user.body().size = bytes_readed;
	// 	auto [err, bytes_writed] = co_await http::async_write(stream_user, serializer_user, asio::as_tuple(asio::use_awaitable));
	// 	if(err.failed() && err != http::error::need_buffer) throw err;
	// 	fmt::print("[writer] - Write {} bytes.\n", bytes_writed);
	// 	// for(int i = 0; i < 128; i++) transmission_buf[i] = 0;
	// 	// parser_archive_response.get().body().size = 128 - parser_archive_response.get().body().size;
	// 	// parser_archive_response.get().body().data = transmission_buf;
    //     // parser_archive_response.get().body().more = ! parser_archive_response.is_done();
	// }

	// // Close user connection.
    // stream_user.socket().shutdown(asio::ip::tcp::socket::shutdown_send);
}

auto listener() -> asio::awaitable<void>
{
	auto ctx = co_await this_coro::executor;
	asio::ip::tcp::acceptor acceptor {ctx, {asio::ip::tcp::v4(), 80}};
    for(;;)
	{
		beast::tcp_stream tcp_stream {co_await acceptor.async_accept(asio::use_awaitable)};
		asio::co_spawn(ctx, session(std::move(tcp_stream)), hf::rethrowed);
	}
}

class http_stream
{
	prv asio::ip::tcp::socket sock;
	prv http::response_parser<http::buffer_body> parser;
	prv beast::flat_buffer buf;

	pbl http_stream(auto executor): sock(executor) {}

	pbl auto connect(auto host) -> asio::awaitable<void>
	{
		auto resolver_results = co_await asio::ip::tcp::resolver(co_await this_coro::executor).async_resolve(host, "http", asio::use_awaitable);
		co_await asio::async_connect(sock, resolver_results, asio::use_awaitable);
	}

	pbl auto write(auto request) -> asio::awaitable<void>
	{
		co_await http::async_write(sock, request, asio::use_awaitable);
	}

	pbl auto read(auto & response) -> asio::awaitable<void>
	{
		co_await http::async_read(sock, buf, response, asio::use_awaitable);
	}

	pbl auto write_header(auto & header) -> asio::awaitable<void>
	{
		// impl.
	}

	pbl auto read_header(auto & header) -> asio::awaitable<void>
	{
		parser.get().base() = header;
		auto header_size = co_await http::async_read_header(sock, buf, parser, asio::use_awaitable);
	}

	pbl auto write_body()
	{
		//  1: [write request] + [read response]
		//	2: [write request header] >> [write request body] + [read response header] >> [read response body]
		//	3: [write request header chunk] >> [write request body chunk] + [read response header chunk] >> [read response body chunk]

		// 1: [packages]
		// 2: [header] + [body]
		// 3: [header chunk] + [body chunk]

		// http::buffer_body;
		// http::string_body;
		// http::vector_body;
		// http::empty_body;
		// http::file_body;
		// http::dynamic_body;

		// 1:
		// response res = http_send(request);

		// 1:
		// http_stream http_stream;
		// http_stream.write(request | response);
		// http_stream.read(request | response);

		// 2:
		// http_stream.write_header(request_header | response_header);
		// http_stream.read_header(request_header | response_header)
		// http_stream.write_body(body);	<------------------------???
		// http_stream.read_body(body);		<------------------------???

		// 3:
		// http_stream.write_header_chunk();	<------------------------???
		// http_stream.write_header_chunk();	xxxxxxxxxxxxxxxxxxxxxxxxxxx
		// http_stream.write_body_chunk(buffer, size);
		// http_stream.read_body_chunk(buffer, size);

		// impl.
	}

	pbl auto read_body()
	{
		// impl.
	}

	pbl auto read_body(char * buffer, std::size_t size) -> asio::awaitable<std::optional<std::size_t>>
	{
		if(!parser.is_done())
		{
			parser.get().body().data = buffer;
			parser.get().body().size = size;
			auto [e, bytes_readed] = co_await http::async_read(sock, buf, parser, asio::as_tuple(asio::use_awaitable));
			if(e.failed() && e != http::error::need_buffer) throw e;
			if(bytes_readed > size) throw std::runtime_error("read header first");
			co_return bytes_readed;
		} else co_return std::nullopt;
	}
};

class https_stream
{
	// prv http::request_parser<http::buffer_body>			parser_request;
	// prv http::request_serializer<http::buffer_body>		serializer_request;
	// prv http::response_serializer<http::buffer_body>	serializer_response;
	prv inline static asio::ssl::context						ssl_ctx {asio::ssl::context::tlsv13_client};
	prv http::response_parser<http::buffer_body>				parser_response;
	prv beast::flat_buffer										buf;
	prv asio::ssl::stream<asio::ip::tcp::socket>				stream;

	pbl https_stream(auto executor): stream(executor, ssl_ctx) {}

	pbl auto connect(auto host) -> asio::awaitable<void>
	{
		auto resolver_results = co_await asio::ip::tcp::resolver(co_await this_coro::executor).async_resolve(host, "https", asio::use_awaitable);
		co_await asio::async_connect(stream.lowest_layer(), resolver_results, asio::use_awaitable);
	}

	pbl auto handshake(auto host) -> asio::awaitable<void>
	{
		SSL_set_tlsext_host_name(stream.native_handle(), host);
		co_await stream.async_handshake(stream.client, asio::use_awaitable);
	}

	pbl auto write(auto package) -> asio::awaitable<void>
	{
		co_await http::async_write(stream, package, asio::use_awaitable);
	}

	pbl auto read(auto & package) -> asio::awaitable<void>
	{
		co_await http::async_read(stream, buf, package, asio::use_awaitable);
	}

	pbl auto write_header(auto & header);

	pbl auto read_header(auto & header) -> asio::awaitable<void>
	{
		co_await http::async_read_header(stream, buf, parser_response, asio::use_awaitable);
		header = parser_response.get().base();
	}

	pbl auto read_request_body(auto & body);
	pbl auto read_response_body(auto & body);
	pbl auto write_request_body(auto & body);
	pbl auto write_response_body(auto & body);

	pbl auto write_request_body_chunk();
	pbl auto read_request_body_chunk();
	pbl auto write_response_body_chunk();

	pbl auto read_response_body_chunk(char * buffer, std::size_t size) -> asio::awaitable<std::optional<std::size_t>>
	{
		if(!parser_response.is_done())
		{
			parser_response.get().body().data = buffer;
			parser_response.get().body().size = size;
			auto [err, bytes_readed] = co_await http::async_read(stream, buf, parser_response, asio::as_tuple(asio::use_awaitable));
			co_return size - parser_response.get().body().size;
		} else co_return std::nullopt;
	}
};

class https_stream_v1
{
	// Basic operations.
	pbl auto write(auto package);
	pbl auto read(auto & package);

	// Splitted operation.
	pbl auto write_header(auto & header);
	pbl auto read_header(auto & header);
	pbl auto read_request_body(auto & body);
	pbl auto read_response_body(auto & body);
	pbl auto write_request_body(auto & body);
	pbl auto write_response_body(auto & body);

	// Chunk operations.
	pbl auto write_request_body_chunk();
	pbl auto read_request_body_chunk();
	pbl auto write_response_body_chunk();
	pbl auto read_response_body_chunk();
};


class https_stream_test
{
	pbl struct
	{
		pbl struct
		{
			void header() {}
			void body() {}
			auto operator()() {}
		} request;

		pbl struct
		{
			void header() {}
			void body() {}
			auto operator()() {}
		} response;

		auto operator()() {}
	} write;

	pbl struct
	{
		pbl struct
		{
			void header() {}
			void body() {}
			auto operator()() {}
		} request;

		pbl struct
		{
			void header() {}
			void body() {}
			auto operator()() {}
		} response;

		auto operator()() {}
	} read;
};

class stream
{
	pbl auto operator <<(auto i) -> asio::awaitable<void>
	{

	}
};

auto test() -> asio::awaitable<void>
{
	https_stream_test stream;
	stream.read.response.body.chunk();

	// stream stream;
	// int request;
	// co_await (stream << request);
	// co_await stream.write.request.header();


	// Data.
	auto host = "jigsaw.w3.org";
	auto path = "/HTTP/ChunkedScript";

	https_stream https_stream {co_await this_coro::executor};
	co_await https_stream.connect(host);
	co_await https_stream.handshake(host);

	http::request<http::empty_body> request {http::verb::get, path, 11};
	request.set("Host", host);
	co_await https_stream.write(request);

	https_stream_test stream_test;
	stream_test.write.request.header();



	// std::string ret;

	// char buf[512];
	// while(auto bytes_readed = co_await https_stream.read_response_body_chunk(buf, 512))
	// {
	// 	fmt::print("Readed: {} bytes.\n", bytes_readed.value());
	// 	ret.append(buf, bytes_readed.value());
	// }

	http::response_header<> header;
	co_await https_stream.read_header(header);
	std::cout << header;

	// http::response<http::string_body> response;
	// co_await https_stream.read(response);
	// std::cout << response.body();

	co_return;
}

int main() try
{
	SetThreadUILanguage(MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US));
    asio::io_context ctx;
    asio::co_spawn(ctx, test(), hf::rethrowed);
    return ctx.run();
} catch(std::exception e) { fmt::print("Except: '{}'.\n", e.what()); }
