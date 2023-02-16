#include <boost/asio/co_spawn.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/steady_timer.hpp>
#include <boost/asio/this_coro.hpp>
#include <boost/asio/experimental/channel.hpp>
#include <boost/system/detail/error_code.hpp>
#include <chrono>
#include <fmt/core.h>
#include <fmt/ranges.h>
#include <iofox.hpp>
#include <iostream>
#include <optional>
#include <string>
#include <vector>
#include <set>
#include <map>

namespace asio = boost::asio;			// NOLINT.
namespace beast = boost::beast;			// NOLINT.
namespace http = beast::http;			// NOLINT.
namespace this_coro = asio::this_coro;	// NOLINT.

asio::io_context ctx;
io::broadcast_channel<int> channel {ctx};

// auto produser() -> io::coro<void>
// {
// 	for(int i = 0;; i++)
// 	{
// 		co_await asio::steady_timer(ctx, std::chrono::seconds(1)).async_wait(io::use_coro);
// 		fmt::print("[produser] - send: '{}'.\n", i);
// 		co_await channel.send(i);
// 	}
// }

// auto consumer(char c) -> io::coro<void>
// {
// 	io::subscriber_channel sub_channel {channel, co_await this_coro::executor};
// 	for(int i = 0; i < 5; i++)
// 	{
// 		int x = co_await sub_channel.async_receive(io::use_coro);
// 		fmt::print("[consumer '{}'] - recieve: '{}'.\n", c, x);
// 	}
// }

// auto coro() -> io::coro<void>
// {
// 	fmt::print("[coro] - run produser.\n");
// 	asio::co_spawn(ctx, produser(), io::rethrowed);

// 	co_await asio::steady_timer(co_await this_coro::executor, std::chrono::seconds(5)).async_wait(io::use_coro);
// 	fmt::print("[coro] - run consumer 'a'.\n");
// 	asio::co_spawn(ctx, consumer('a'), io::rethrowed);
// }

std::map<int, bool> map {{1, true}, {2, true}, {3, false}, {4, true}};

auto coro() -> io::coro<void>
{
	for(auto it = map.begin(); it != map.end(); it++)
	{
		co_await asio::steady_timer(co_await this_coro::executor, std::chrono::seconds(1)).async_wait(io::use_coro);
		if(it->second == false) it = map.erase(it);
		fmt::print("[coro] - Read '{}'.\n", it->first);
	}
};

int main() try
{
	io::windows::set_asio_locale(io::windows::lang::english);
	asio::co_spawn(ctx, coro(), io::rethrowed);
	return ctx.run();
}
catch(std::exception & e) { fmt::print("Exception: '{}'.\n", e.what()); }
catch(...) { fmt::print("Exception: 'unknown'.\n"); }
