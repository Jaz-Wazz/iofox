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

template <typename CompletionToken>
auto run_twice(CompletionToken&& token)
{
    return [token = std::forward<CompletionToken>(token)](const boost::system::error_code & ec) mutable
	{
        token(ec);
        token(ec);
    };
}

TEST_CASE()
{
	boost::asio::io_context io_context;

	boost::asio::steady_timer timer {io_context, 10s};
	timer.async_wait([](auto...){ fmt::print("10s end original.\n"); });
	timer.async_wait(run_twice([](auto...){ fmt::print("10s end custom token.\n"); }));

	// boost::asio::co_spawn(io_context, coro(), iofox::rethrowed);
	io_context.run();
}
