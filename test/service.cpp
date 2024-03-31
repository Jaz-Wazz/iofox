// boost_asio
#include <boost/asio/any_io_executor.hpp>
#include <boost/asio/as_tuple.hpp>
#include <boost/asio/associated_executor.hpp>
#include <boost/asio/async_result.hpp>
#include <boost/asio/awaitable.hpp>
#include <boost/asio/bind_executor.hpp>
#include <boost/asio/buffer.hpp>
#include <boost/asio/cancellation_signal.hpp>
#include <boost/asio/cancellation_type.hpp>
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
#include <boost/asio/bind_cancellation_slot.hpp>

// boost_system
#include <boost/system/detail/error_code.hpp>
#include <boost/system/error_code.hpp>

// boost_core
#include <boost/core/demangle.hpp>

// stl
#include <boost/system/system_error.hpp>
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

auto cancellable_async_op_with_exceptions(auto executor, auto token)
{
	auto impl = [](auto state) -> void
	{
		try
		{
			state.throw_if_cancelled(true);

			// Not needed.
			// Set by default in "boost::asio::experimental::detail::co_composed_state" constructor.
			// Documentation: "By default, terminal per-operation cancellation is enabled for composed operations that use experimental::co_composed.".
			// state.reset_cancellation_state(boost::asio::enable_terminal_cancellation());

			fmt::print("[custom_async_op] - start.\n");
			boost::asio::steady_timer timer {state.get_io_executor(), std::chrono::seconds(5)};
			co_await timer.async_wait(boost::asio::deferred);
			fmt::print("[custom_async_op] - end.\n");
			co_return {{}, 15};
		}
		catch(const boost::system::system_error & error)
		{
			fmt::print("[custom_async_op] - cancel.\n");
			co_return {error.code(), 0};
		}
	};

	auto sub_handler = boost::asio::experimental::co_composed<void(boost::system::error_code, int)>(std::move(impl), executor);
	return boost::asio::async_initiate<decltype(token), void(boost::system::error_code, int)>(std::move(sub_handler), token);
}

boost::asio::cancellation_signal signal;

auto coro() -> iofox::coro<void>
{
	auto executor = co_await boost::asio::this_coro::executor;

	cancellable_async_op_with_exceptions(executor, boost::asio::bind_cancellation_slot(signal.slot(), [](auto ec, int value)
	{
		fmt::print("[completion_handler] - value: '{}', result: '{}'.\n", value, ec.message());
	}));
	signal.emit(boost::asio::cancellation_type::terminal);
}

TEST_CASE()
{
	iofox::this_thread::set_language("en_us");
	boost::asio::io_context io_context;
	boost::asio::co_spawn(io_context, coro(), iofox::rethrowed);
	io_context.run();
}
