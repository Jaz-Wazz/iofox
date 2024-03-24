// boost_asio
#include <boost/asio/any_io_executor.hpp>
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
#include <boost/asio/require_concept.hpp>

// iofox
#include <fmt/core.h>
#include <iofox/coro.hpp>
#include <iofox/service.hpp>
#include <iofox/rethrowed.hpp>

// catch2
#include <catch2/catch_test_macros.hpp>
#include <stdexcept>

struct custom_property
{
	template <class T> static constexpr bool is_applicable_property_v = true;
};

struct custom_executor: public boost::asio::io_context::executor_type
{
	int i = 10;
};

int query(custom_executor executor, custom_property)
{
	return executor.i;
}

TEST_CASE()
{
	boost::asio::io_context io_context;
	boost::asio::strand<boost::asio::io_context::executor_type> s = boost::asio::make_strand(io_context);

	custom_executor custom_executor {io_context.get_executor()};
	custom_property custom_property;
	int value	= boost::asio::query(custom_executor, custom_property);
	auto alloc	= boost::asio::query(custom_executor, boost::asio::execution::allocator);
	fmt::print("value: '{}'.\n", value);
}
