// boost_asio
#include <boost/asio/any_io_executor.hpp>
#include <boost/asio/as_tuple.hpp>
#include <boost/asio/async_result.hpp>
#include <boost/asio/awaitable.hpp>
#include <boost/asio/bind_executor.hpp>
#include <boost/asio/buffer.hpp>
#include <boost/asio/detail/type_traits.hpp>
#include <boost/asio/error.hpp>
#include <boost/asio/experimental/cancellation_condition.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/co_spawn.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ssl/context.hpp>
#include <boost/asio/steady_timer.hpp>
#include <boost/asio/this_coro.hpp>
#include <boost/asio/use_awaitable.hpp>
#include <boost/asio/deferred.hpp>
#include <boost/asio/experimental/parallel_group.hpp>
#include <boost/asio/experimental/co_composed.hpp>

// boost_system
#include <boost/system/detail/error_code.hpp>
#include <boost/system/error_code.hpp>

// boost_core
#include <boost/core/demangle.hpp>

// stl
#include <chrono> // IWYU pragma: keep

// iofox
#include <iofox/coro.hpp>
#include <iofox/rethrowed.hpp>
#include <iofox/this_thread.hpp>

// fmt.
#include <fmt/core.h>

// catch2
#include <catch2/catch_test_macros.hpp>
#include <stdexcept>

auto http_request(int i, char c) -> boost::asio::awaitable<void>
{
	fmt::print("garox.\n");
	co_return;
}

auto custom_async_op(auto executor, auto token)
{
	auto impl = [](auto state) -> void
	{
		// fmt::print("[handler] - execute...\n");
		boost::asio::steady_timer timer {state.get_io_executor(), std::chrono::seconds(5)};
		co_await timer.async_wait(boost::asio::deferred);
		co_return {};
	};

	auto sub_handler = boost::asio::experimental::co_composed<void(boost::system::error_code)>(std::move(impl), executor);
	return boost::asio::async_initiate<decltype(token), void(boost::system::error_code)>(std::move(sub_handler), token);
}

auto custom_async_op_based_on_awaitable() -> iofox::coro<void>
{
	boost::asio::steady_timer timer {co_await boost::asio::this_coro::executor, std::chrono::seconds(5)};
	co_await timer.async_wait(boost::asio::deferred);
}

auto coro() -> iofox::coro<void>
{
	auto executor = co_await boost::asio::this_coro::executor;

	{
		auto op = boost::asio::co_spawn(executor, custom_async_op_based_on_awaitable(), boost::asio::deferred);
		// auto op2 = op; // Copy compile-time error.
	}
	{
		boost::asio::steady_timer timer {co_await boost::asio::this_coro::executor};
		auto op = timer.async_wait(boost::asio::deferred);
		auto op2 = op;

		fmt::print("[timer_deffered] - run.\n");
		timer.expires_after(std::chrono::seconds(5)); co_await std::move(op);
		fmt::print("[timer_deffered] - end.\n");

		fmt::print("[timer_deffered] - run copy.\n");
		timer.expires_after(std::chrono::seconds(5)); co_await std::move(op2);
		fmt::print("[timer_deffered] - end copy.\n");
	}
	{
		auto op = custom_async_op(executor, boost::asio::deferred);
		auto op2 = op;

		fmt::print("[custom_async_op_deffered] - run.\n");
		co_await std::move(op);
		fmt::print("[custom_async_op_deffered] - end.\n");

		fmt::print("[custom_async_op_deffered] - run copy.\n");
		co_await std::move(op2);
		fmt::print("[custom_async_op_deffered] - end copy.\n");
	}
}

TEST_CASE()
{
	iofox::this_thread::set_language("en_us");
	boost::asio::io_context io_context;
	boost::asio::co_spawn(io_context, coro(), iofox::rethrowed);
	io_context.run();
}
