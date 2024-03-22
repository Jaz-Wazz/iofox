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
	auto coro = [] -> iofox::coro<void> { fmt::print("[coro] - sas.\n"); co_return; };

	// bind executor with completion token - work around completition token call.
	boost::asio::co_spawn(context, coro(), boost::asio::bind_executor(logging_executor{}, [](auto...)
	{
		fmt::print("garox.\n");
	}));

	// bind executor with coro - not work&
	boost::asio::co_spawn(context, boost::asio::bind_executor(logging_executor{}, coro), iofox::rethrowed);
	context.run();

	// boost::asio::io_context context_b;
	// auto strand_executor = boost::asio::make_strand(context_a);

	// auto coro = [] -> iofox::coro<void>
	// {
	// 	const auto executor = co_await boost::asio::this_coro::executor;
	// 	auto & service = boost::asio::use_service<some_service>(executor.context());

	// 	for(int i = 0; i < 10; i++)
	// 	{
	// 		fmt::print("[coro] - context: '{}', service value: '{}'.\n", (void *) &executor.context(), service.i++);
	// 	}
	// 	co_return;
	// };

	// boost::asio::co_spawn(context_a, coro(), iofox::rethrowed);
	// boost::asio::co_spawn(context_a.get_executor(), coro(), iofox::rethrowed);
	// boost::asio::co_spawn(context_a.get_executor().context(), coro(), iofox::rethrowed);
	// boost::asio::co_spawn(strand_executor, coro(), iofox::rethrowed);
	// boost::asio::co_spawn(context_b, coro(), iofox::rethrowed);
	// context_a.run();
	// context_b.run();
}
