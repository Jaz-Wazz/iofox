// boost_asio
#include <boost/asio/any_io_executor.hpp>
#include <boost/asio/as_tuple.hpp>
#include <boost/asio/associated_executor.hpp>
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

template <class T = boost::asio::deferred_t>
auto custom_async_op(auto executor, T token = boost::asio::deferred)
{
	auto impl = [](auto state) -> void
	{
		fmt::print("[custom_async_op] - wait.\n");
		boost::asio::steady_timer timer {state.get_io_executor(), std::chrono::seconds(5)};
		co_await timer.async_wait(boost::asio::deferred);
		fmt::print("[custom_async_op] - end.\n");
		co_return {};
	};

	auto sub_handler = boost::asio::experimental::co_composed<void(boost::system::error_code)>(std::move(impl), executor);
	return boost::asio::async_initiate<decltype(token), void(boost::system::error_code)>(std::move(sub_handler), token);
}

namespace iofox
{
	template <class T = boost::asio::deferred_t>
	auto repeat(int count, auto operation, T token = boost::asio::deferred)
	{
		auto impl = [=](auto state) -> void
		{
			for(int i = 0; i < count; i++) co_await operation;
			co_return {};
		};

		auto sub_handler = boost::asio::experimental::co_composed<void(boost::system::error_code)>(std::move(impl));
		return boost::asio::async_initiate<decltype(token), void(boost::system::error_code)>(std::move(sub_handler), token);
	}
}

auto coro() -> iofox::coro<void>
{
	auto executor = co_await boost::asio::this_coro::executor;

	// auto op = custom_async_op(executor, boost::asio::deferred);
	// co_await iofox::repeat(executor, 3, op, iofox::use_coro);

	// co_await iofox::repeat(3, custom_async_op(executor, boost::asio::deferred), io::);
	// co_await iofox::repeat(3, custom_async_op(executor));
	// co_await iofox::repeat(3, socket.async_write_some(args..., iofox::deferred));
	// co_await iofox::repeat(3, []{ return awaitable_based(args...); });

	// co_await (custom_async_op(executor, iofox::deferred) | iofox::repeat(3));

	// co_await custom_async_op(executor, iofox::repeat(3, iofox::deferred));
	// co_await custom_async_op(executor, iofox::deferred | iofox::repeat(3) | iofox::as_tuple);

	// iofox::http::headers headers = {{"host", "exmaple.com"}};
	// iofox::http::request request {"GET", "/", headers};
	// auto response = co_await iofox::http::send(executor, "http://exmaple.com", request, iofox::deffered | iofox::retry_timeout(10s, retry_handler));

	// using token = iofox::deffered | iofox::timeout(10s) | iofox::retry(retry_handler);
	// ...
	// iofox::http::headers headers = {{"host", "exmaple.com"}};
	// iofox::http::request request {"GET", "/", headers};
	// auto response = co_await iofox::http::send(executor, "http://exmaple.com", request, token);

	// iofox::http::headers headers = {{"host", "exmaple.com"}};
	// iofox::http::request request {"GET", "/", headers};
	// auto response = co_await iofox::retry_timeout(10s, retry_handler,
	// {
	//		iofox::http::send(executor, "http://exmaple.com", request, iofox::deffered)
	// }, token);

	// boost::asio::steady_timer timer {executor};
	// timer.async_wait()
}

TEST_CASE()
{
	iofox::this_thread::set_language("en_us");
	boost::asio::io_context io_context;
	boost::asio::co_spawn(io_context, coro(), iofox::rethrowed);
	io_context.run();
}
