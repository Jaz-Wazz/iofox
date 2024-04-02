// boost_asio
#include <boost/asio/any_io_executor.hpp>
#include <boost/asio/awaitable.hpp>
#include <boost/asio/bind_executor.hpp>
#include <boost/asio/execution/allocator.hpp>
#include <boost/asio/execution/any_executor.hpp>
#include <boost/asio/execution/blocking.hpp>
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
#include <concepts>
#include <fmt/core.h>
#include <iofox/coro.hpp>
#include <iofox/service.hpp>
#include <iofox/rethrowed.hpp>

// catch2
#include <catch2/catch_test_macros.hpp>
#include <memory>
#include <stdexcept>

// auto require(auto executor, custom_property prop)
// {
// 	return custom_executor<decltype(executor)>(executor);
// }

// template <class T>
// auto require(custom_executor<T> executor, custom_property prop)
// {
// 	executor.custom_property_value = prop.i;
// 	return executor;
// }

// using custom_any_io_executor = boost::asio::execution::any_executor
// <
// 	class custom_property,
// 	boost::asio::execution::context_as_t<boost::asio::execution_context&>,
// 	boost::asio::execution::blocking_t::never_t,
// 	boost::asio::execution::prefer_only<boost::asio::execution::blocking_t::possibly_t>,
// 	boost::asio::execution::prefer_only<boost::asio::execution::outstanding_work_t::tracked_t>,
// 	boost::asio::execution::prefer_only<boost::asio::execution::outstanding_work_t::untracked_t>,
// 	boost::asio::execution::prefer_only<boost::asio::execution::relationship_t::fork_t>,
// 	boost::asio::execution::prefer_only<boost::asio::execution::relationship_t::continuation_t>
// >;

namespace iofox
{
	template <class T>
	struct executor: public T
	{
		using inner_executor_type = T;

		executor() requires std::default_initializable<T>
		: T() {}

		template <class X>
		executor(const X & other_executor) requires std::convertible_to<X, T>
		: T(other_executor) {}

		template <class X>
		executor(const executor<X> & other_executor) requires std::convertible_to<X, T>
		: T(other_executor.get_inner_executor()) {}

		template <class X>
		executor(executor<X> && other_executor) requires std::convertible_to<X, T>
		: T(other_executor.get_inner_executor()) {}

		template <class X>
		executor & operator=(const executor<X> & other_executor) noexcept requires std::convertible_to<X, T>
		{
			T::operator=(other_executor.get_inner_executor());
			return *this;
		}

		template <class X>
		executor & operator=(const executor<X> && other_executor) noexcept requires std::convertible_to<X, T>
		{
			T::operator=(std::move(other_executor.get_inner_executor()));
			return *this;
		}

		T get_inner_executor() const noexcept
		{
			return *this;
		}

		int custom_property_value = 0;
	};

	struct custom_property_t
	{
		template <class T> static constexpr bool is_applicable_property_v = true;
		using polymorphic_query_result_type = int;
	};

	template <class T>
	int query(iofox::executor<T> executor, iofox::custom_property_t)
	{
		return executor.custom_property_value;
	}
}

TEST_CASE()
{
	boost::asio::io_context io_context;
	iofox::executor<boost::asio::io_context::executor_type> executor {io_context.get_executor()};
	executor.custom_property_value = 123;

	int value = boost::asio::query(executor, iofox::custom_property_t());
	fmt::print("value: '{}'.\n", value);
}
