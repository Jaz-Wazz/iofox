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
	// static constexpr bool is_preferable = true;
	static constexpr bool is_requirable = true;
	using polymorphic_query_result_type = int;
	int i;
	custom_property(int value = 0): i(value) {}
};

// struct custom_executor: public boost::asio::io_context::executor_type
// {

// };

auto require(auto executor, custom_property prop)
{
	return 0;
}

int query(auto executor, custom_property prop)
{
	return prop.i;
}

using custom_any_io_executor = boost::asio::execution::any_executor
<
	class custom_property,
	boost::asio::execution::context_as_t<boost::asio::execution_context&>,
	boost::asio::execution::blocking_t::never_t,
	boost::asio::execution::prefer_only<boost::asio::execution::blocking_t::possibly_t>,
	boost::asio::execution::prefer_only<boost::asio::execution::outstanding_work_t::tracked_t>,
	boost::asio::execution::prefer_only<boost::asio::execution::outstanding_work_t::untracked_t>,
	boost::asio::execution::prefer_only<boost::asio::execution::relationship_t::fork_t>,
	boost::asio::execution::prefer_only<boost::asio::execution::relationship_t::continuation_t>
>;

TEST_CASE()
{
	boost::asio::io_context io_context;
	custom_property custom_property_v;
	boost::asio::io_context::executor_type executor = io_context.get_executor();
	// executor.require()

	using x = boost::asio::require_result_t<boost::asio::any_io_executor, custom_property>;

	boost::asio::strand<boost::asio::io_context::executor_type> strand = boost::asio::make_strand(io_context);

	// auto x = boost::asio::require(executor, custom_property_v);

	{
		// custom_executor custom_executor {io_context.get_executor()};
		// auto x = boost::asio::prefer(custom_executor, custom_property_v);

		// int value		= boost::asio::query(custom_executor, custom_property_v);
		// auto & context	= boost::asio::query(custom_executor, boost::asio::execution::context);
		// fmt::print("[custom_executor] - value: '{}'.\n", value);
	}
	{
		boost::asio::any_io_executor custom_any_io_executor {io_context.get_executor()};
		// custom_any_io_executor.require(custom_property_v);
		// custom_any_io_executor custom_any_io_executor {io_context.get_executor()};
		// custom_any_io_executor = boost::asio::prefer(custom_any_io_executor, custom_property_v);
		auto x = boost::asio::require(custom_any_io_executor, custom_property_v);

		int value		= boost::asio::query(custom_any_io_executor, custom_property_v);
		auto & context	= boost::asio::query(custom_any_io_executor, boost::asio::execution::context);
		fmt::print("[custom_any_executor from io_context::executor_type] - value: '{}'.\n", value);
	}
	{
		// custom_executor custom_executor {io_context.get_executor()};
		// custom_any_io_executor custom_any_io_executor {custom_executor};
		// int value		= boost::asio::query(custom_any_io_executor, custom_property_v);
		// auto & context	= boost::asio::query(custom_any_io_executor, boost::asio::execution::context);
		// fmt::print("[custom_any_executor from custom_executor] - value: '{}'.\n", value);
	}
	{
		// boost::asio::any_io_executor any_io_executor {io_context.get_executor()};
		// int value		= any_io_executor.query(custom_property_v);
		// auto & context	= any_io_executor.query(boost::asio::execution::context);
	}
}
