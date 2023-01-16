#include <boost/asio/co_spawn.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/this_coro.hpp>
#include <fmt/core.h>
#include <iofox.hpp>
#include <iostream>
#include <string>
#include <vector>

namespace asio = boost::asio;			// NOLINT.
namespace beast = boost::beast;			// NOLINT.
namespace http = beast::http;			// NOLINT.
namespace this_coro = asio::this_coro;	// NOLINT.

auto coro() -> io::coro<void>
{
	io::http::client client;
	co_await client.connect("https://httpbin.org");

	io::http::request_header request_header {"POST", "/post", {{"host", "httpbin.org"}, {"transfer-encoding", "chunked"}}};
	co_await client.write_header(request_header);
	co_await client.write_body_octets("hui", 3);
	co_await client.write_body_octets("sui", 3);
	co_await client.write_body_octets("xui", 3);
	co_await client.write_body_chunk_tail();

	io::http::response<std::string> response;
	co_await client.read(response);
	std::cout << response << '\n';
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
