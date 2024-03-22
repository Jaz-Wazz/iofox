// boost_asio
#include <boost/asio/bind_executor.hpp>
#include <boost/asio/execution_context.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/co_spawn.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ssl/context.hpp>
#include <boost/asio/this_coro.hpp>

// iofox
#include <fmt/core.h>
#include <iofox/coro.hpp>
#include <iofox/service.hpp>
#include <iofox/rethrowed.hpp>

// catch2
#include <catch2/catch_test_macros.hpp>

TEST_CASE()
{
	boost::asio::io_context context;

	auto coro_server = [] -> iofox::coro<void>
	{
		const auto executor = co_await boost::asio::this_coro::executor;
		// http_server server {executor};
		co_return;
	};

	auto coro_client = [] -> iofox::coro<void>
	{
		const auto executor = co_await boost::asio::this_coro::executor;
		// http_client client {executor};
		co_return;
	};

	// some magic: [executor] + [associated ssl_context].
	auto & executor_for_server = context;
	auto & executor_for_client = context;

	boost::asio::co_spawn(executor_for_server, coro_server(), iofox::rethrowed);
	boost::asio::co_spawn(executor_for_client, coro_client(), iofox::rethrowed);
}

TEST_CASE()
{

}
