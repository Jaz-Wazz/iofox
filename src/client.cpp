#include <boost/asio/awaitable.hpp>
#include <boost/asio/co_spawn.hpp>
#include <boost/asio/completion_condition.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/address.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/this_coro.hpp>
#include <boost/asio/use_awaitable.hpp>
#include <boost/asio/read.hpp>
#include <boost/asio/coroutine.hpp>
#include <boost/asio/experimental/coro.hpp>
#include <boost/asio/experimental/use_coro.hpp>
#include <chrono>
#include <coroutine>
#include <cstring>
#include <fmt/core.h>
#include <fmt/chrono.h>
#include <exception>
#include <fstream>
#include <iofox/iofox.hpp>
#include <iostream>
#include <ranges>
#include <span>
#include <string>
#include <string_view>
#include <variant>

namespace asio = boost::asio;			// NOLINT.
namespace this_coro = asio::this_coro;	// NOLINT.

// class sas
// {
// 	void operator co_await() {}
// 	// void await_transform(sas s) {}
// };

// class co: public asio::awaitable<void>
// {
// 	public: using asio::awaitable<void>::executor_type;
// 	public: using asio::awaitable<void>::value_type;
// 	// public: using asio::awaitable<void>::awaitable;
// 	public: co(asio::awaitable<void> && aw): asio::awaitable<void>(std::move(aw)) {}
// 	public: co & operator co_await() { fmt::print("sas.\n"); return *this; }
// };

// template <typename... T> class std::coroutine_traits<co, T...>: public std::coroutine_traits<asio::awaitable<void>>
// {
// 	public: using std::coroutine_traits<asio::awaitable<void>>::coroutine_traits;
// 	public: using std::coroutine_traits<asio::awaitable<void>>::promise_type;
// };

// void operator co_await(sas s) { fmt::print("sas.\n"); }

// auto x(asio::ip::tcp::socket socket) -> co
// {
// 	std::string buffer;
// 	co_await socket.async_read_some(asio::buffer(buffer), io::use_coro);
// 	co_return;
// }

// auto xxx() -> co { fmt::print("sis.\n"); co_return; }

auto reader_e(io::isstream stream) -> io::coro<void>
{
	char * data = new char[528888890];
	std::size_t size = 0;

	for(;size != 528888890; size++)
	{
		auto valoraw = stream.async_get();
		data[size] = valoraw.contains() ? valoraw.value() : co_await valoraw.awaitable();
	}
}

auto open_session() -> io::coro<asio::ip::tcp::socket>
{
	asio::ip::tcp::socket socket {co_await this_coro::executor};
	co_await socket.async_connect({asio::ip::make_address("127.0.0.1"), 555}, io::use_coro);
	co_return socket;
}

auto benchmark(auto title, auto callable) -> io::coro<void>
{
	auto start = std::chrono::steady_clock::now();
	co_await callable(co_await open_session());
	auto finish = std::chrono::steady_clock::now();
	fmt::print("Benchmark '{}': {}.\n", title, std::chrono::duration_cast<std::chrono::milliseconds>(finish - start));
}

auto coro() -> io::coro<void>
{
	co_await benchmark("reader_e", reader_e);
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
