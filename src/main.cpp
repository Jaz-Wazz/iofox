#include <boost/asio/co_spawn.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/ssl/stream.hpp>
#include <boost/asio/this_coro.hpp>
#include <boost/beast/core/error.hpp>
#include <boost/beast/core/file.hpp>
#include <boost/beast/core/file_base.hpp>
#include <boost/beast/core/flat_buffer.hpp>
#include <boost/beast/http/buffer_body.hpp>
#include <boost/beast/http/empty_body.hpp>
#include <boost/beast/http/fields.hpp>
#include <boost/beast/http/message.hpp>
#include <boost/beast/http/parser.hpp>
#include <boost/beast/http/serializer.hpp>
#include <boost/beast/http/string_body.hpp>
#include <boost/beast/http/vector_body.hpp>
#include <boost/beast/http/write.hpp>
#include <boost/beast/http/file_body.hpp>
#include <boost/system/detail/error_code.hpp>
#include <fmt/core.h>
#include <fstream>
#include <iofox.hpp>
#include <twitch.hpp>
#include <util.hpp>
#include <iostream>
#include <string>
#include <variant>
#include <vector>

namespace asio = boost::asio;			// NOLINT.
namespace beast = boost::beast;			// NOLINT.
namespace http = beast::http;			// NOLINT.
namespace this_coro = asio::this_coro;	// NOLINT.

auto coro() -> io::coro<void>
{
	io::http::client client;
	co_await client.connect("https://httpbin.org");

	std::string body = "very_long_message_with_very_big_data_for_test";

	// io::http::request_header request_header {"POST", "/post", {{"host", "httpbin.org"}, {"content-length", std::to_string(body.length())}}};
	io::http::request_header request_header {"POST", "/post", {{"host", "httpbin.org"}, {"Transfer-Encoding", "chunked"}}};
	co_await client.write_header(request_header);

	for(int i = 0; i < body.length(); i += 4)
	{
		auto slice = body.substr(i, 4);
		co_await client.write_body_piece(slice.data(), slice.size());
		fmt::print("write: '{}'.\n", slice);
	}
	co_await client.write_body_piece_tail();

	io::http::response_header response_header;
	co_await client.read_header(response_header);
	std::cout << response_header << '\n';

	std::string response_body;
	co_await client.read_body(response_body);
	std::cout << response_body << '\n';
}

auto read_big_file() -> io::coro<void>
{
	io::http::client client;
	co_await client.connect("https://sabnzbd.org");

	io::http::request_header request_header {"GET", "/tests/internetspeed/50MB.bin", {{"host", "sabnzbd.org"}}};
	co_await client.write_header(request_header);

	io::http::response_header response_header;
	co_await client.read_header(response_header);
	std::cout << response_header;

	std::string response_body;
	std::cout << "start read" << '\n';
	co_await client.read_body(response_body);
	std::cout << "to file" << '\n';
	std::ofstream("50mb.bin") << response_body;
}

auto read_chunked_encoding() -> io::coro<void>
{
	io::http::client client;
	co_await client.connect("https://jigsaw.w3.org");

	io::http::request_header request_header {"GET", "/HTTP/ChunkedScript", {{"host", "jigsaw.w3.org"}}};
	co_await client.write_header(request_header);

	io::http::response_header response_header;
	co_await client.read_header(response_header);
	std::cout << response_header;

	std::string response_body;
	co_await client.read_body(response_body);
	std::cout << response_body << '\n';
}

auto read_consistent_test() -> io::coro<void>
{
	io::http::client client;
	// co_await client.connect("https://jigsaw.w3.org");
	// io::http::request_header request_header {"GET", "/HTTP/ChunkedScript", {{"host", "jigsaw.w3.org"}}};
	// co_await client.write_header(request_header);

	// co_await client.connect("https://www.google.com");
	// io::http::request_header request_header {"GET", "/", {{"host", "www.google.com"}}};
	// co_await client.write_header(request_header);

	// co_await client.connect("https://exmaple.com");
	// io::http::request_header request_header {"GET", "/", {{"host", "exmaple.com"}}};
	// co_await client.write_header(request_header);

	co_await client.connect("https://adbtc.top");
	io::http::request_header request_header {"GET", "/", {{"host", "adbtc.top"}}};
	co_await client.write_header(request_header);

	io::http::response<std::string> response;
	co_await client.read_basic(response);
	std::cout << response << '\n';
}

auto generate_test() -> io::coro<void>
{
	io::http::response<std::string> response {200, {{"key", "val"}, {"foo", "bar"}}};
	response.body() = "wouagwagnwae\n;ogiwae\nfowiajfawoeg\niahweg\noiawegol";
	response.chunked(true);
	std::cout << response << '\n';
	co_return;
}

int main() try
{
	io::windows::set_asio_locale(io::windows::lang::english);
	asio::io_context ctx;
	asio::co_spawn(ctx, generate_test(), io::rethrowed);
	return ctx.run();
}
catch(std::exception & e) { fmt::print("Exception: '{}'.\n", e.what()); }
catch(...) { fmt::print("Exception: 'unknown'.\n"); }
