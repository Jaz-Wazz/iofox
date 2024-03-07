// boost_asio
#include <boost/asio/io_context.hpp>
#include <boost/asio/co_spawn.hpp>

// iofox
#include <iofox/coro.hpp>
#include <iofox/service.hpp>
#include <iofox/rethrowed.hpp>

// catch2
#include <catch2/catch_test_macros.hpp>

TEST_CASE("service")
{
	iofox::service<int> service;

	auto coro = [&] -> iofox::coro<void>
	{
		int & value = co_await service.get_or_make(0);
		value++;
		REQUIRE(value <= 10);
		co_return;
	};

	boost::asio::io_context context_x, context_y;
	for(int i = 0; i < 10; i++) boost::asio::co_spawn(context_x, coro(), iofox::rethrowed);
	for(int i = 0; i < 10; i++) boost::asio::co_spawn(context_y, coro(), iofox::rethrowed);
	context_x.run();
	context_y.run();
}
