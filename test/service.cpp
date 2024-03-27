// boost_asio
#include <boost/asio/any_io_executor.hpp>
#include <boost/asio/as_tuple.hpp>
#include <boost/asio/buffer.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/co_spawn.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ssl/context.hpp>

// iofox
#include <boost/asio/steady_timer.hpp>
#include <boost/asio/this_coro.hpp>
#include <boost/asio/use_awaitable.hpp>
#include <chrono>
#include <functional>
#include <initializer_list>
#include <iofox/coro.hpp>
#include <iofox/rethrowed.hpp>

// fmt.
#include <fmt/core.h>

// stl.
#include <stdexcept>

// catch2
#include <catch2/catch_test_macros.hpp>

#include <boost/asio/deferred.hpp>
using namespace std::chrono_literals;

namespace iofox
{
	template <class T>
	inline auto timeout(std::chrono::steady_clock::duration duration, std::initializer_list<T> op) -> iofox::coro<int>
	{
		co_return 0;
	}

	inline auto timeout(std::chrono::steady_clock::duration duration, auto op) -> iofox::coro<int>
	{
		co_return 0;
	}

	template <class T>
	inline auto retry(auto handler, std::initializer_list<T> op) -> iofox::coro<int>
	{
		co_return 0;
	}

	inline auto retry(auto handler, auto op) -> iofox::coro<int>
	{
		co_return 0;
	}

	template <class T>
	inline auto retry_timeout(auto handler, std::chrono::steady_clock::duration duration, std::initializer_list<T> op) -> iofox::coro<int>
	{
		co_return 0;
	}

	inline auto retry_timeout(auto handler, std::chrono::steady_clock::duration duration, auto op) -> iofox::coro<int>
	{
		co_return 0;
	}

	template <typename... T>
	inline auto repeat(int count, boost::asio::deferred_async_operation<T...> operation) -> iofox::coro<void>
	{
		boost::asio::deferred_async_operation<T...> op_x = operation;
		boost::asio::deferred_async_operation<T...> op_y = operation;
		boost::asio::deferred_async_operation<T...> op_z = operation;
		fmt::print(".\n");

		// op_x | op_y;

		// boost::asio::deferred | boost::asio::use_awaitable;


		auto use_tuble_and_awaitable = boost::asio::as_tuple(boost::asio::use_awaitable);
		// iofox::use_tuple | iofox::use_awaitable
		// iofox::repeat(10, socket.async_write_some(args..., token));
		// auto result = co_await socket.async_write_some(args..., iofox::use_repeat(10, token));
		// auto result = co_await socket.async_write_some(args..., iofox::use_timeout(10s, iofox::use_repeat(10, token)));
		// auto result = co_await socket.async_write_some(args..., token | iofox::use_timeout(10s) | iofox::use_repeat(10));
		// auto result = co_await (socket.async_write_some(args..., token) | iofox::use_timeout(10s) | iofox::use_repeat(10)));

		co_await std::move(op_x);
		fmt::print(".\n");
		co_await std::move(op_y);
		fmt::print(".\n");
		co_await std::move(op_z);
		fmt::print(".\n");
	}
}

auto coro() -> iofox::coro<void>
{
	boost::asio::steady_timer timer {co_await boost::asio::this_coro::executor, 1s};

	// fmt::print("wait 1 sec.\n");
	// co_await timer.async_wait(iofox::use_coro);
	// fmt::print("wait end.\n");

	fmt::print("wait 1 sec.\n");
	co_await iofox::repeat(10, timer.async_wait(boost::asio::deferred));
	fmt::print("wait end.\n");
}

TEST_CASE()
{
	boost::asio::io_context io_context;
	boost::asio::co_spawn(io_context, coro(), iofox::rethrowed);
	io_context.run();
}
