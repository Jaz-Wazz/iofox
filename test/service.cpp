// boost_asio
#include <boost/asio/bind_executor.hpp>
#include <boost/asio/execution/allocator.hpp>
#include <boost/asio/execution/context.hpp>
#include <boost/asio/execution_context.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/co_spawn.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/is_executor.hpp>
#include <boost/asio/ssl/context.hpp>
#include <boost/asio/this_coro.hpp>
#include <boost/asio/strand.hpp>
#include <boost/asio/query.hpp>
#include <boost/asio/require.hpp>

// iofox
#include <fmt/core.h>
#include <iofox/coro.hpp>
#include <iofox/service.hpp>
#include <iofox/rethrowed.hpp>

// catch2
#include <catch2/catch_test_macros.hpp>

struct executor_wrapper: public boost::asio::io_context::executor_type
{
	boost::asio::ssl::context * ssl_context = nullptr;
};

TEST_CASE("one")
{
	boost::asio::io_context io_context;
	executor_wrapper executor_wrapper {io_context.get_executor()};

	boost::asio::ssl::context ssl_context {boost::asio::ssl::context::tls};
	executor_wrapper.ssl_context = &ssl_context;

	auto coro = [] -> iofox::coro<void>
	{
		const auto executor = co_await boost::asio::this_coro::executor;
		auto ex = executor.target<class executor_wrapper>();
		fmt::print("[coro] - my ssl context is: '{}'.\n", (void *) ex->ssl_context);
		co_return;
	};

	boost::asio::co_spawn(executor_wrapper, coro(), iofox::rethrowed);
	io_context.run();
}
