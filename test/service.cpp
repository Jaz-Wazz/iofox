// boost_asio
#include "iofox/this_thread.hpp"
#include <boost/asio/any_io_executor.hpp>
#include <boost/asio/as_tuple.hpp>
#include <boost/asio/async_result.hpp>
#include <boost/asio/bind_executor.hpp>
#include <boost/asio/buffer.hpp>
#include <boost/asio/error.hpp>
#include <boost/asio/experimental/cancellation_condition.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/co_spawn.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ssl/context.hpp>

// iofox
#include <boost/asio/steady_timer.hpp>
#include <boost/asio/this_coro.hpp>
#include <boost/asio/use_awaitable.hpp>
#include <boost/system/detail/error_code.hpp>
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
#include <boost/asio/experimental/parallel_group.hpp>
using namespace std::chrono_literals;

// template <class CompletionToken>
// struct timed_token
// {
// 	CompletionToken& token;
// };

// template <class... Signatures>
// struct timed_initiation
// {
// 	template <class CompletionHandler, class Initiation, class... InitArgs>
// 	void operator()(CompletionHandler handler, Initiation&& initiation, InitArgs&&... init_args)
// 	{
// 		using boost::asio::experimental::make_parallel_group;
// 		using boost::asio::experimental::wait_for_one;

// 		auto ex		= boost::asio::get_associated_executor(handler, boost::asio::get_associated_executor(initiation));
// 		auto alloc	= boost::asio::get_associated_allocator(handler);

// 		auto op_underlying = boost::asio::bind_executor(ex, [&](auto && token)
// 		{
// 			return boost::asio::async_initiate<decltype(token), Signatures...>
// 			(
// 				std::forward<Initiation>(initiation), token,
// 				std::forward<InitArgs>(init_args)...
// 			);
// 		});

// 		auto group = make_parallel_group(op_underlying);
// 		group.async_wait(wait_for_one(), [handler = std::move(handler)](std::array<std::size_t, 1>, std::error_code, auto... underlying_op_results) mutable
// 		{
// 			std::move(handler)(std::move(underlying_op_results)...);
// 		});
// 	}
// };

// template <class InnerCompletionToken, class... Signatures>
// struct boost::asio::async_result<timed_token<InnerCompletionToken>, Signatures...>
// {
// 	template <class Initiation, class... InitArgs>
// 	static auto initiate(Initiation && init, timed_token<InnerCompletionToken> t, InitArgs &&... init_args)
// 	{
// 		return asio::async_initiate<InnerCompletionToken, Signatures...>
// 		(
// 			timed_initiation<Signatures...>{},
// 			t.token,
// 			std::forward<Initiation>(init),
// 			std::forward<InitArgs>(init_args)...
// 		);
// 	}
// };

#include <boost/core/demangle.hpp>

auto coro() -> iofox::coro<void>
{
	fmt::print("run.\n");
	boost::asio::steady_timer timer {co_await boost::asio::this_coro::executor, 5s};

	boost::asio::use_awaitable_t token;
	int result = co_await boost::asio::async_initiate<boost::asio::use_awaitable_t<>, void(boost::system::error_code, int)>([&](auto && handler)
	{
		auto executor = boost::asio::get_associated_executor(handler);
		fmt::print("handler type: '{}'.\n", boost::core::demangle(typeid(handler).name()));
		fmt::print("executor type: '{}'.\n", boost::core::demangle(typeid(executor).name()));
		fmt::print("executor wrapped type: '{}'.\n", boost::core::demangle(executor.target_type().name()));

		// handler(boost::asio::error::name_too_long, 10);
		// handler({}, 10);

		timer.async_wait([handler = std::move(handler)](auto ec) mutable
		{
			fmt::print("handler inner type: '{}'.\n", boost::core::demangle(typeid(handler).name()));
			handler({}, 15);
		});
	}, token);

	fmt::print("end, result: '{}'.\n", result);
}

TEST_CASE()
{
	iofox::this_thread::set_language("en_us");
	boost::asio::io_context io_context;
	boost::asio::co_spawn(io_context, coro(), iofox::rethrowed);
	io_context.run();
}
