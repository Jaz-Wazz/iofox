// boost_asio
#include <boost/asio/any_io_executor.hpp>
#include <boost/asio/bind_executor.hpp>
#include <boost/asio/execution/allocator.hpp>
#include <boost/asio/execution/any_executor.hpp>
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
#include <memory>
#include <stdexcept>

struct custom_property
{
	template <class T> static constexpr bool is_applicable_property_v = true;
	using polymorphic_query_result_type = int;
};

struct custom_executor: public boost::asio::io_context::executor_type
{
	int i = 10;
};

int query(custom_executor executor, custom_property)
{
	return executor.i;
}

using custom_any_executor = boost::asio::execution::any_executor<custom_property>;

TEST_CASE()
{
	boost::asio::io_context io_context;

	// boost::asio::strand<boost::asio::io_context::executor_type> s = boost::asio::make_strand(io_context);
	// s.query(boost::asio::execution::allocator);
	// auto allo = boost::asio::query(s, boost::asio::execution::allocator);

	custom_executor custom_executor {io_context.get_executor()};
	custom_property custom_property;
	int value	= boost::asio::query(custom_executor, custom_property);
	auto alloc	= boost::asio::query(custom_executor, boost::asio::execution::allocator);
	fmt::print("value: '{}'.\n", value);

	custom_any_executor any_executor;
	int i = any_executor.query(custom_property);

	// boost::asio::execution::any_executor<::custom_property>::find_convertible_property<::custom_property>::value;
	// custom_any_executor::find_convertible_property<class custom_property>::type;
	// boost::asio::any_io_executor any_executor {custom_executor};

	// boost::asio::execution::any_executor<boost::asio::execution::blocking_t::possibly_t> ex;
	// auto ex2 = boost::asio::require(ex, boost::asio::execution::blocking.possibly);

	// any_executor.query(custom_property);
	// int another_value = boost::asio::query(any_executor, custom_property);
}
