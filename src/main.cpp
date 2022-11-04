#include "netfox.hpp"
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
#include <boost/beast/http/fields.hpp>
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
#include <netfox.hpp>

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

class sas
{
	pbl auto operator >>(int i) -> asio::awaitable<void>
	{

	}

	pbl auto operator [](asio::awaitable<void> i) -> asio::awaitable<void>
	{

	}
};

auto test() -> asio::awaitable<void>
{
	// netfox::http::stream<netfox::side::client> stream;
	// co_await stream.connect("exmaple.com");

	// http::request<http::empty_body> request {http::verb::get, "/", 11};
	// if(auto result = co_await stream.write(request))
	// {
	// 	// stream.read(only_body, request);
	// 	// stream.read(only_header, request);
	// 	// stream
	// }
	// else
	// {
	// 	fmt::print("Write error: '{}'.\n", result.error());
	// }

	// stream.expires_from_now(boost::posix_time::seconds(60));

	// co_await stream.async(stream >> 5);

	// auto x = stream >> 5; co_await std::move(x);

	// [stream >> var]

	// client_stream[]

	// stream.read(only_header);
	// stream.write(only_body);
	// co_await stream[stream >> var];


	// http::request<http::empty_body> request {http::verb::get, "/", 11};
	// request.set("Host", "exmaple.com");
	// co_await client_stream.write_request(request);

	http::header<true> header;
	header.method(http::verb::get);
	header.target("/");
	header.set("Host", "exmaple.com");
	co_await client_stream.write_request_header(header);

	http::response<http::string_body> response;
	co_await client_stream.read_response(response);

	std::cout << response;

	// http::response_header<> header;
	// co_await client_stream.read_response_header(header);
	// std::cout << header;

	// std::string ret;
	// char buffer[8];
	// while(auto bytes_readed = co_await client_stream.read_response_body_chunk(buffer, 8))
	// {
	// 	fmt::print("Readed: {} bytes.\n", bytes_readed.value());
	// 	ret.append(buffer, bytes_readed.value());
	// }
	// std::cout << ret;


	// nf::http::stream<nf::side::client> x;


	// Data.
	// auto host = "jigsaw.w3.org";
	// auto path = "/HTTP/ChunkedScript";

	// nf::https::stream https_stream;
	// co_await https_stream.connect(host);
	// co_await https_stream.handshake_client(host);

	// http::request<http::empty_body> request {http::verb::get, path, 11};
	// request.set("Host", host);
	// co_await (https_stream << request);

	// http::response<http::string_body> response;
	// co_await (https_stream >> response);

	// std::cout << response;
	// co_return;
}

int main() try
{
	SetThreadUILanguage(MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US));
    asio::io_context ctx;
    asio::co_spawn(ctx, test(), hf::rethrowed);
    return ctx.run();
} catch(std::exception e) { fmt::print("Except: '{}'.\n", e.what()); }
