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

	io::http::response_header response_header;
	co_await client.read_header(response_header);
	std::cout << response_header << '\n';

	std::string body;
	co_await client.read_body(body);
	std::cout << body << '\n';
}

int main() try
{
	io::windows::set_asio_locale(io::windows::lang::english);
	asio::io_context ctx;
	asio::co_spawn(ctx, coro(), io::rethrowed);
	return ctx.run();
}
catch(std::exception & e) { fmt::print("Exception: '{}'.\n", e.what()); }
catch(...) { fmt::print("Exception: 'unknown'.\n"); }
