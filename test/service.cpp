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

TEST_CASE()
{
	// associated_executor
	// binded_executor
	// linked_executor

	// parametric_executor
	// integrated_executor
	// attached_executor

	// amalgamated_executor

	// association_executor
	// execution_tail
	// tail
	// chain
	// execution_chain
	// default_executor
	// default_args_executor
	// refered_executor
	// wrapped_executor

	// packed_executor
	// auto executor = iofox::make_packed_executor(io_context.get_executor(), ssl_context, dns_resolver, timer);

	// extended_executor

	// auto executor = iofox::make_associated_executor(io_context.get_executor(), ssl_context, dns_resolver, timer);
	// auto executor = iofox::make_associated_tail(io_context.get_executor(), ssl_context, dns_resolver, timer);
	// auto executor = iofox::make_tail(io_context.get_executor(), ssl_context, dns_resolver, timer);

	// auto executor = iofox::bind_with_executor(io_context.get_executor(), ssl_context, dns_resolver, timer);

	// auto executor = iofox::make_binded_executor(io_context.get_executor(), ssl_context, dns_resolver, timer);

	// auto executor = iofox::associate_executor(io_context.get_executor(), ssl_context, dns_resolver, timer);

	boost::asio::io_context io_context;
		iofox::executor<boost::asio::io_context::executor_type> executor {io_context.get_executor()};
executor.custom_property_value = 123;

	int value = boost::asio::query(executor, iofox::custom_property_t());
	fmt::print("value: '{}'.\n", value);

	iofox::any_io_executor any_io_executor {io_context.get_executor()};
	int value_2 = boost::asio::query(any_io_executor, iofox::custom_property_t());
	fmt::print("value 2: '{}'.\n", value_2);

	iofox::any_io_executor any_io_executor_2 {executor};
	int value_3 = boost::asio::query(any_io_executor_2, iofox::custom_property_t());
	fmt::print("value 3: '{}'.\n", value_3);
}
