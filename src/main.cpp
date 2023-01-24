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

	io::file file {"1.txt", io::file::mode::read};

	// io::http::request<std::string> request {"POST", "/post", {{"host", "httpbin.org"}, {"content-length", "4"}}, "qwertyuiop"};
	// io::http::request<std::string> request {"POST", "/post", {{"host", "httpbin.org"}, {"transfer-encoding", "chunked"}}, "qwertyuiop"};
	// io::http::request<io::file> request {"POST", "/post", {{"host", "httpbin.org"}, {"content-length", "6"}}, std::move(file)};
	io::http::request<io::file> request {"POST", "/post", {{"host", "httpbin.org"}, {"transfer-encoding", "chunked"}}, std::move(file)};
	co_await client.write(request);

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
