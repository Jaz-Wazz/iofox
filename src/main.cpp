#include <boost/asio/co_spawn.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/this_coro.hpp>
#include <fmt/core.h>
#include <iostream>
#include <string>
#include <vector>
#include <iofox/iofox.hpp>

namespace asio = boost::asio;			// NOLINT.
namespace this_coro = asio::this_coro;	// NOLINT.

auto coro() -> io::coro<void>
{
	io::http::start_line start_line {"GET", "/", "HTTP/1.0"};

	co_await io::http::read_start_line();
	fmt::print("test.\n");
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
