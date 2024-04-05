// boost_asio
#include <boost/asio/execution/any_executor.hpp>
#include <boost/asio/execution/executor.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/query.hpp>

// stl
#include <boost/asio/system_executor.hpp>
#include <concepts> // IWYU pragma: keep

// fmt
#include <fmt/core.h>

// catch2
#include <catch2/catch_test_macros.hpp>

namespace iofox
{
	template <boost::asio::execution::executor T>
	struct executor: public T
	{
		using inner_executor_type = T;
		T get_inner_executor() const noexcept { return *this; }
		int custom_property_value = 0;
	};

	struct custom_property_t
	{
		template <class T> static constexpr bool is_applicable_property_v = true;
		static constexpr bool is_preferable = true;
		using polymorphic_query_result_type = int;
	};

	template <class T>
	int query(iofox::executor<T> executor, iofox::custom_property_t)
	{
		return executor.custom_property_value;
	}

	using any_io_executor = boost::asio::execution::any_executor
	<
		boost::asio::execution::context_as_t<boost::asio::execution_context&>,
		boost::asio::execution::blocking_t::never_t,
		boost::asio::execution::prefer_only<boost::asio::execution::blocking_t::possibly_t>,
		boost::asio::execution::prefer_only<boost::asio::execution::outstanding_work_t::tracked_t>,
		boost::asio::execution::prefer_only<boost::asio::execution::outstanding_work_t::untracked_t>,
		boost::asio::execution::prefer_only<boost::asio::execution::relationship_t::fork_t>,
		boost::asio::execution::prefer_only<boost::asio::execution::relationship_t::continuation_t>,
		iofox::custom_property_t
	>;
}

TEST_CASE("executor")
{
	boost::asio::io_context io_context;
	boost::asio::system_executor system_executor;
	boost::asio::io_context::executor_type io_executor = io_context.get_executor();

	SECTION("default_constructor")
	{
		iofox::executor<boost::asio::system_executor> executor;
	}

	SECTION("copy_constructor")
	{
		iofox::executor<boost::asio::system_executor>			executor_a {system_executor};
		iofox::executor<boost::asio::io_context::executor_type>	executor_b {io_executor};
		auto executor_x = executor_a;
		auto executor_y = executor_b;
	}

	SECTION("move_constructor")
	{
		iofox::executor<boost::asio::system_executor>			executor_a {system_executor};
		iofox::executor<boost::asio::io_context::executor_type>	executor_b {io_executor};
		auto executor_x = std::move(executor_a);
		auto executor_y = std::move(executor_b);
	}

	SECTION("copy_assigment")
	{
		iofox::executor<boost::asio::system_executor>			executor_a {system_executor};
		iofox::executor<boost::asio::system_executor>			executor_b {system_executor};
		iofox::executor<boost::asio::io_context::executor_type>	executor_c {io_executor};
		iofox::executor<boost::asio::io_context::executor_type>	executor_d {io_executor};
		executor_a = executor_b;
		executor_c = executor_d;
	}

	SECTION("move_assigment")
	{
		iofox::executor<boost::asio::system_executor>			executor_a {system_executor};
		iofox::executor<boost::asio::system_executor>			executor_b {system_executor};
		iofox::executor<boost::asio::io_context::executor_type>	executor_c {io_executor};
		iofox::executor<boost::asio::io_context::executor_type>	executor_d {io_executor};
		executor_a = std::move(executor_b);
		executor_c = std::move(executor_d);
	}

	SECTION("underlying_copy_constructor")
	{
		iofox::executor<boost::asio::system_executor>			executor_x {system_executor};
		iofox::executor<boost::asio::io_context::executor_type>	executor_y {io_executor};
	}

	SECTION("underlying_move_constructor")
	{
		iofox::executor<boost::asio::system_executor>			executor_x {std::move(system_executor)};
		iofox::executor<boost::asio::io_context::executor_type>	executor_y {std::move(io_executor)};
	}
}

TEST_CASE()
{
	boost::asio::io_context io_context;
	iofox::executor<boost::asio::io_context::executor_type> executor {io_context.get_executor()};
	executor.custom_property_value = 123;

	// auto executor = iofox::make_executor();

	// auto executor = iofox::make_execution_tail(io_context, ssl_context, dns_resolver);
	// iofox::send(executor, request);

	// auto tail = iofox::make_tail(io_context, ssl_context, dns_resolver);
	// iofox::send(tail, request);

	// auto tail = iofox::make_execution_tail(io_context, ssl_context, dns_resolver);
	// iofox::send(tail, request);

	// auto execution_tail = iofox::make_tail(io_context, ssl_context, dns_resolver);
	// iofox::send(execution_tail, request);

	// iofox::execution_tail tail {io_context, ssl_context, dns_resolver};
	// iofox::send(tail, request);

	int value = boost::asio::query(executor, iofox::custom_property_t());
	fmt::print("value: '{}'.\n", value);

	iofox::any_io_executor any_io_executor {io_context.get_executor()};
	int value_2 = boost::asio::query(any_io_executor, iofox::custom_property_t());
	fmt::print("value 2: '{}'.\n", value_2);

	iofox::any_io_executor any_io_executor_2 {executor};
	int value_3 = boost::asio::query(any_io_executor_2, iofox::custom_property_t());
	fmt::print("value 3: '{}'.\n", value_3);
}
