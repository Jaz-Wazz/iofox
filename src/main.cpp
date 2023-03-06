#include <boost/asio/co_spawn.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/this_coro.hpp>
#include <boost/asio/buffer.hpp>
#include <cstring>
#include <fmt/core.h>
#include <iostream>
#include <string>
#include <vector>
#include <iofox/iofox.hpp>

namespace asio = boost::asio;			// NOLINT.
namespace this_coro = asio::this_coro;	// NOLINT.

auto coro() -> io::coro<void>
{
	// auto request = co_await io::http::read_request(sock, buffer);
	// auto body	= co_await io::http::read_body(sock, buffer);

	// io::http::request request {"GET", "/", "HTTP/1.0", {{"key", "val"}, {"foo", "bar"}}}:
	// co_await io::http::write(sock, buffer, request);
	// co_await http_stream.write(request);

	// auto start_line = co_await io::http::read_start_line(sock, buffer);
	// auto start_line = co_await io::http::read_request<std::string>(sock, buffer);
	// io::http::start_line start_line;
	// co_await io::http::read(sock, buffer, start_line);

	std::string string = "GET /page.sas HTTP/1.0\r\n";
	auto buffer = asio::dynamic_buffer(string);
	auto start_line = io::http::read_start_line(buffer);
	co_return;
}

int main() try
{
	io::windows::set_asio_locale(io::windows::lang::english);
	asio::io_context ctx;
	asio::co_spawn(ctx, coro(), io::rethrowed);
	return ctx.run();
}
catch(const std::exception & e)
{
	fmt::print("Exception: '{}'.\n", e.what());
}
