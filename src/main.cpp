#include <boost/asio/co_spawn.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/steady_timer.hpp>
#include <boost/asio/this_coro.hpp>
#include <boost/asio/experimental/channel.hpp>
#include <boost/system/detail/error_code.hpp>
#include <chrono>
#include <fmt/core.h>
#include <iofox.hpp>
#include <iostream>
#include <string>
#include <vector>

namespace asio = boost::asio;			// NOLINT.
namespace beast = boost::beast;			// NOLINT.
namespace http = beast::http;			// NOLINT.
namespace this_coro = asio::this_coro;	// NOLINT.

asio::io_context ctx;
std::vector<asio::experimental::channel<void(boost::system::error_code, int)> *> vector;

auto produser() -> io::coro<void>
{
	for(int i = 0;; i++)
	{
		co_await asio::steady_timer(ctx, std::chrono::seconds(1)).async_wait(io::use_coro);
		fmt::print("[produser] - send: '{}'.\n", i);
		for(auto el : vector) co_await el->async_send({}, i, io::use_coro);
	}
}

auto consumer(char c) -> io::coro<void>
{
	asio::experimental::channel<void(boost::system::error_code, int)> chan {ctx};
	vector.emplace_back(&chan);
	for(;;)
	{
		int x = co_await chan.async_receive(io::use_coro);
		fmt::print("[consumer '{}'] - recieve: '{}'.\n", c, x);
	}
}

int main() try
{
	io::windows::set_asio_locale(io::windows::lang::english);
	asio::co_spawn(ctx, produser(), io::rethrowed);
	asio::co_spawn(ctx, consumer('a'), io::rethrowed);
	asio::co_spawn(ctx, consumer('b'), io::rethrowed);
	asio::co_spawn(ctx, consumer('c'), io::rethrowed);
	return ctx.run();
}
catch(std::exception & e) { fmt::print("Exception: '{}'.\n", e.what()); }
catch(...) { fmt::print("Exception: 'unknown'.\n"); }
