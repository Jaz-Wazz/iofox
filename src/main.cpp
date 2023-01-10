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

	std::string out;
	char buffer[4];
	while(auto bytes_readed = co_await client.read_body_piece(buffer, 4))
	{
		fmt::print("read: {} bytes.\n", *bytes_readed);
		out += std::string(buffer, *bytes_readed);
	}

	std::cout << response_header << '\n';
	std::cout << out << '\n';
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
