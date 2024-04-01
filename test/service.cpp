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

struct custom_property
{
	template <class T> static constexpr bool is_applicable_property_v = true;
	static constexpr bool is_requirable = true;
	using polymorphic_query_result_type = int;
	custom_property(int value = 0): i(value) {}
	int i;
};

template <class T>
struct custom_executor: public T
{
	using inner_executor_type = T;

	custom_executor() requires std::default_initializable<T>
	: T() {}

	template <class X>
	custom_executor(const X & other_executor) requires std::convertible_to<X, T>
	: T(other_executor) {}

	template <class X>
	custom_executor(const custom_executor<X> & other_executor) requires std::convertible_to<X, T>
	: T(other_executor.get_inner_executor()) {}

	template <class X>
	custom_executor(custom_executor<X> && other_executor) requires std::convertible_to<X, T>
	: T(other_executor.get_inner_executor()) {}

	template <class X>
	custom_executor & operator=(const custom_executor<X> & other_executor) noexcept requires std::convertible_to<X, T>
	{
		T::operator=(other_executor.get_inner_executor());
		return *this;
	}

	template <class X>
	custom_executor & operator=(const custom_executor<X> && other_executor) noexcept requires std::convertible_to<X, T>
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

auto require(auto executor, custom_property prop)
{
	return custom_executor<decltype(executor)>(executor);
}

template <class T>
auto require(custom_executor<T> executor, custom_property prop)
{
	executor.custom_property_value = prop.i;
	return executor;
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

	boost::asio::io_context::executor_type executor = io_context.get_executor();
	auto value_x = boost::asio::require(executor, custom_property());
	auto value_xxx = boost::asio::require(value_x, boost::asio::execution::blocking_t::never);
	auto value_xxxx = boost::asio::require(value_x, custom_property(15));


	auto value_y = boost::asio::require(value_x, custom_property());
	// value_x.

	// custom_any_io_executor custom_any_io_executor {io_context.get_executor()};
	// auto value_y = boost::asio::require(custom_any_io_executor, custom_property());

	// boost::asio::any_io_executor any_io_executor {io_context.get_executor()};
	// auto value_y = boost::asio::require(any_io_executor, custom_property());



	// custom_executor<boost::asio::io_context::executor_type> custom_executor_1 {executor};
	// custom_executor_1 = custom_executor_1;
	// boost::asio::io_context::executor_type e{};

	// custom_executor<boost::asio::strand<boost::asio::io_context::executor_type>> custom_executor_2 {executor};

	// auto & context = boost::asio::query(executor, boost::asio::execution::context);

	// boost::asio::io_context io_context;
	// custom_property custom_property_v;
	// boost::asio::io_context::executor_type executor = io_context.get_executor();
	// // executor.require()

	// using x = boost::asio::require_result_t<boost::asio::any_io_executor, custom_property>;

	// boost::asio::strand<boost::asio::io_context::executor_type> strand = boost::asio::make_strand(io_context);

	// // auto x = boost::asio::require(executor, custom_property_v);

	// {
	// 	// custom_executor custom_executor {io_context.get_executor()};
	// 	// auto x = boost::asio::prefer(custom_executor, custom_property_v);

	// 	// int value		= boost::asio::query(custom_executor, custom_property_v);
	// 	// auto & context	= boost::asio::query(custom_executor, boost::asio::execution::context);
	// 	// fmt::print("[custom_executor] - value: '{}'.\n", value);
	// }
	// {
	// 	boost::asio::any_io_executor custom_any_io_executor {io_context.get_executor()};
	// 	// custom_any_io_executor.require(custom_property_v);
	// 	// custom_any_io_executor custom_any_io_executor {io_context.get_executor()};
	// 	// custom_any_io_executor = boost::asio::prefer(custom_any_io_executor, custom_property_v);
	// 	auto x = boost::asio::require(custom_any_io_executor, custom_property_v);

	// 	int value		= boost::asio::query(custom_any_io_executor, custom_property_v);
	// 	auto & context	= boost::asio::query(custom_any_io_executor, boost::asio::execution::context);
	// 	fmt::print("[custom_any_executor from io_context::executor_type] - value: '{}'.\n", value);
	// }
	// {
	// 	// custom_executor custom_executor {io_context.get_executor()};
	// 	// custom_any_io_executor custom_any_io_executor {custom_executor};
	// 	// int value		= boost::asio::query(custom_any_io_executor, custom_property_v);
	// 	// auto & context	= boost::asio::query(custom_any_io_executor, boost::asio::execution::context);
	// 	// fmt::print("[custom_any_executor from custom_executor] - value: '{}'.\n", value);
	// }
	// {
	// 	// boost::asio::any_io_executor any_io_executor {io_context.get_executor()};
	// 	// int value		= any_io_executor.query(custom_property_v);
	// 	// auto & context	= any_io_executor.query(boost::asio::execution::context);
	// }
}
