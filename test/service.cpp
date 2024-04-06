// boost_asio
#include <boost/asio/any_io_executor.hpp>
#include <boost/asio/execution/any_executor.hpp>
#include <boost/asio/execution/executor.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/query.hpp>
#include <boost/asio/require.hpp>
#include <boost/asio/system_executor.hpp>

// stl
#include <concepts> // IWYU pragma: keep

// fmt
#include <fmt/core.h>

// catch2
#include <catch2/catch_test_macros.hpp>

namespace iofox
{
	template <boost::asio::execution::executor T>
	struct packed_executor: public T
	{
		using inner_executor_type = T;
		T get_inner_executor() const noexcept { return *this; }
		int custom_property_value = 0;
	};

	struct custom_property_t
	{
		template <class T> static constexpr bool is_applicable_property_v = true;
		static constexpr bool is_preferable = true;
		static constexpr bool is_requirable = true;
		using polymorphic_query_result_type = int;
		int value = 0;
	};

	template <class T>
	int query(iofox::packed_executor<T> executor, iofox::custom_property_t)
	{
		return executor.custom_property_value;
	}

	template <class T>
	auto require(iofox::packed_executor<T> executor, iofox::custom_property_t property)
	{
		executor.custom_property_value = property.value;
		return executor;
	}

	using any_executor = boost::asio::execution::any_executor
	<
		boost::asio::execution::context_as_t<boost::asio::execution_context&>,
		boost::asio::execution::blocking_t::never_t,
		boost::asio::execution::prefer_only<boost::asio::execution::blocking_t::possibly_t>,
		boost::asio::execution::prefer_only<boost::asio::execution::outstanding_work_t::tracked_t>,
		boost::asio::execution::prefer_only<boost::asio::execution::outstanding_work_t::untracked_t>,
		boost::asio::execution::prefer_only<boost::asio::execution::relationship_t::fork_t>,
		boost::asio::execution::prefer_only<boost::asio::execution::relationship_t::continuation_t>,
		boost::asio::execution::prefer_only<iofox::custom_property_t>
	>;
}

TEST_CASE()
{
	boost::asio::io_context io_context;
	iofox::packed_executor<boost::asio::io_context::executor_type> packed_executor {io_context.get_executor()};

	auto edited_packed_executor = boost::asio::require(packed_executor, iofox::custom_property_t(32));
	auto value = boost::asio::query(edited_packed_executor, iofox::custom_property_t());
	fmt::print("value: '{}'.\n", value);
}
