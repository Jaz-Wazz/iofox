// boost_asio
#include <boost/asio/bind_executor.hpp>
#include <boost/asio/execution_context.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/co_spawn.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ssl/context.hpp>
#include <boost/asio/this_coro.hpp>
#include <boost/asio/strand.hpp>

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

struct logging_executor
{
	// All executors must be no-throw equality comparable.
	bool operator==(const logging_executor &) const noexcept = default;

	// All executors must provide a const member function execute().
	void execute(std::invocable auto handler) const
	{
		fmt::print("handler invocation starting.\n");
		std::move(handler)();
		fmt::print("handler invocation complete.\n");
	}
};

TEST_CASE("one")
{
	boost::asio::io_context context;

	auto coro = [] -> iofox::coro<void>
	{
		const auto executor = co_await boost::asio::this_coro::executor;
		fmt::print("[coro] - executor type name: '{}'.\n", executor.target_type().name());
		co_return;
	};

	// bind executor with completion token - work around completition token call.
	boost::asio::co_spawn(context, coro(), boost::asio::bind_executor(logging_executor{}, [](auto...)
	{
		fmt::print("garox.\n");
	}));

	context.run();
}
