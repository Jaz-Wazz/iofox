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
		static constexpr bool is_requirable = true;
		using polymorphic_query_result_type = int;
		int value = 0;
	};

	template <class T>
	int query(iofox::executor<T> executor, iofox::custom_property_t)
	{
		return executor.custom_property_value;
	}

	template <class T>
	auto require(iofox::executor<T> executor, iofox::custom_property_t property)
	{
		executor.custom_property_value = property.value;
		return executor;
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
		boost::asio::execution::prefer_only<iofox::custom_property_t>
	>;
}

TEST_CASE()
{
	boost::asio::io_context io_context;

	// SECTION("")
	// {
	// 	iofox::executor<boost::asio::io_context::executor_type> executor {io_context.get_executor()};

	// 	auto ex_x = boost::asio::require(executor, iofox::custom_property_t(10));
	// 	auto ex_y = boost::asio::prefer(executor, iofox::custom_property_t(10));
	// 	auto ex_z = boost::asio::query(executor, iofox::custom_property_t());
	// }

	// SECTION("")
	// {
	// 	iofox::any_io_executor executor {io_context.get_executor()};

	// 	auto ex_x = boost::asio::prefer(executor, iofox::custom_property_t(10));
	// 	auto ex_y = boost::asio::query(executor, iofox::custom_property_t());
	// 	// require not allowed.
	// }

	// SECTION("any_executors_conversation")
	// {
	// 	boost::asio::any_io_executor	any_executor_asio	{iofox::any_io_executor(io_context.get_executor())};
	// 	iofox::any_io_executor			any_executor_iofox	{boost::asio::any_io_executor(io_context.get_executor())};
	// }

	// SECTION("")
	// {
	// 	iofox::executor<boost::asio::io_context::executor_type> executor {io_context.get_executor()};

	// 	auto modified_executor = boost::asio::require(executor, iofox::custom_property_t(32));
	// 	fmt::print("[iofox::executor] - Require propetry value: '32'.\n");

	// 	auto value = boost::asio::query(modified_executor, iofox::custom_property_t());
	// 	fmt::print("[iofox::executor] - Query propetry value: '{}'.\n", value);
	// }

	// SECTION("")
	// {
	// 	iofox::executor<boost::asio::io_context::executor_type> executor {io_context.get_executor()};

	// 	auto modified_executor = boost::asio::prefer(executor, iofox::custom_property_t(32));
	// 	fmt::print("[iofox::executor] - Prefer propetry value: '32'.\n");

	// 	auto value = boost::asio::query(modified_executor, iofox::custom_property_t());
	// 	fmt::print("[iofox::executor] - Query propetry value: '{}'.\n", value);
	// }

	// SECTION("")
	// {
	// 	iofox::executor<boost::asio::io_context::executor_type> executor {io_context.get_executor()};

	// 	auto modified_executor = boost::asio::require(executor, iofox::custom_property_t(32));
	// 	fmt::print("[iofox::executor] - Require propetry value: '32'.\n");

	// 	iofox::any_io_executor any_executor {modified_executor};

	// 	auto value = boost::asio::query(any_executor, iofox::custom_property_t());
	// 	fmt::print("[iofox::executor] - Query propetry value: '{}'.\n", value);
	// }

	// SECTION("")
	// {
	// 	iofox::executor<boost::asio::io_context::executor_type> executor {io_context.get_executor()};
	// 	iofox::any_io_executor any_executor {executor};

	// 	auto modified_executor = boost::asio::prefer(any_executor, iofox::custom_property_t(32));
	// 	fmt::print("[iofox::executor] - Require propetry value: '32'.\n");


	// 	auto value = boost::asio::query(modified_executor, iofox::custom_property_t());
	// 	fmt::print("[iofox::executor] - Query propetry value: '{}'.\n", value);
	// }

	// SECTION("")
	// {
	// 	iofox::any_io_executor any_executor {io_context.get_executor()};

	// 	auto modified_executor = boost::asio::prefer(any_executor, iofox::custom_property_t(32));
	// 	fmt::print("[iofox::executor] - Prefer propetry value: '32'.\n");


	// 	auto value = boost::asio::query(modified_executor, iofox::custom_property_t());
	// 	fmt::print("[iofox::executor] - Query propetry value: '{}'.\n", value);
	// }
}
