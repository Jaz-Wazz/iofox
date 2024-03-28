// boost_asio
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
#include <boost/asio/steady_timer.hpp>
#include <boost/asio/this_coro.hpp>
#include <boost/asio/use_awaitable.hpp>
#include <boost/asio/deferred.hpp>
#include <boost/asio/experimental/parallel_group.hpp>

// boost_system
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

using namespace std::chrono_literals;

template <class CompletionToken>
struct timed_token
{
	std::chrono::milliseconds timeout;
	CompletionToken& token;
};

template <class... Signatures>
struct timed_initiation
{
	template <class CompletionHandler, class Initiation, class... InitArgs>
	void operator()(CompletionHandler handler, std::chrono::milliseconds timeout, Initiation&& initiation, InitArgs&&... init_args)
	{
		using boost::asio::experimental::make_parallel_group;

		auto ex		= boost::asio::get_associated_executor(handler, boost::asio::get_associated_executor(initiation));
		auto alloc	= boost::asio::get_associated_allocator(handler);
		auto timer	= std::allocate_shared<boost::asio::steady_timer>(alloc, ex, timeout);

		make_parallel_group
		(
			boost::asio::bind_executor(ex, [&](auto && token)
			{
				return timer->async_wait(std::forward<decltype(token)>(token));
			}),
			boost::asio::bind_executor(ex, [&](auto && token)
			{
				return boost::asio::async_initiate<decltype(token), Signatures...>
				(
					std::forward<Initiation>(initiation), token,
					std::forward<InitArgs>(init_args)...
				);
			})
		).async_wait(boost::asio::experimental::wait_for_one(), [handler = std::move(handler), timer](std::array<std::size_t, 2>, std::error_code, auto... underlying_op_results) mutable
		{
			timer.reset();
			std::move(handler)(std::move(underlying_op_results)...);
		});
	}
};

template <class InnerCompletionToken, class... Signatures>
struct boost::asio::async_result<timed_token<InnerCompletionToken>, Signatures...>
{
	template <class Initiation, class... InitArgs>
	static auto initiate(Initiation && init, timed_token<InnerCompletionToken> t, InitArgs &&... init_args)
	{
		return asio::async_initiate<InnerCompletionToken, Signatures...>
		(
			timed_initiation<Signatures...>{},
			t.token,
			t.timeout,
			std::forward<Initiation>(init),
			std::forward<InitArgs>(init_args)...
		);
	}
};

auto coro() -> iofox::coro<void>
{
	fmt::print("run.\n");
	boost::asio::steady_timer timer {co_await boost::asio::this_coro::executor, 5s};
	auto [ec] = co_await timer.async_wait(boost::asio::as_tuple(timed_token{10s, iofox::use_coro}));
	fmt::print("end, result: '{}'.\n", ec.message());
}

TEST_CASE()
{
	iofox::this_thread::set_language("en_us");
	boost::asio::io_context io_context;
	boost::asio::co_spawn(io_context, coro(), iofox::rethrowed);
	io_context.run();
}
