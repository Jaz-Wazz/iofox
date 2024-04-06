// boost_asio
#include <boost/asio/any_io_executor.hpp>
#include <boost/asio/execution/any_executor.hpp>
#include <boost/asio/execution/blocking.hpp>
#include <boost/asio/execution/context.hpp>
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
	struct custom_property_t
	{
		template <class T> static constexpr bool is_applicable_property_v = true;
		static constexpr bool is_preferable = true;
		static constexpr bool is_requirable = true;
		using polymorphic_query_result_type = int;
		int value = 0;
	};

	template <boost::asio::execution::executor T>
	struct packed_executor: public T
	{
		using inner_executor_type = T;
		T get_inner_executor() const noexcept { return *this; }
		int custom_property_value = 0;

		int query(const iofox::custom_property_t &) const
		{
			return custom_property_value;
		}

		auto require(const iofox::custom_property_t & property) const
		{
			packed_executor<T> modified_executor = *this;
			modified_executor.custom_property_value = property.value;
			return modified_executor;
		}

		decltype(auto) query(const auto & property) const
		{
			return boost::asio::query(get_inner_executor(), property);
		}

		decltype(auto) require(const auto & property) const
		{
			return packed_executor(boost::asio::require(get_inner_executor(), property));
		}
	};

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

	const auto edited_packed_executor_x = boost::asio::require(packed_executor, iofox::custom_property_t(32));
	const auto edited_packed_executor_y = boost::asio::require(packed_executor, boost::asio::execution::blocking.never);
	const auto edited_packed_executor_z = boost::asio::prefer(packed_executor, boost::asio::execution::blocking.never);

	auto value_x = boost::asio::query(edited_packed_executor_x, iofox::custom_property_t());
	auto & value_y = boost::asio::query(edited_packed_executor_x, boost::asio::execution::context_t());
	fmt::print("value: '{}'.\n", value_x);
}
