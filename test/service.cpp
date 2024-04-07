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
#include <fmt/ranges.h>

// catch2
#include <catch2/catch_test_macros.hpp>
#include <tuple>

namespace iofox
{
	template <class T>
	struct packed_arg
	{
		template <class X> static constexpr bool is_applicable_property_v = true;
		static constexpr bool is_preferable = true;
		static constexpr bool is_requirable = true;
		using polymorphic_query_result_type = T;
		T value {};
	};

	template <boost::asio::execution::executor T, class... Args>
	struct packed_executor: public T
	{
		using inner_executor_type = T;
		T get_inner_executor() const noexcept { return *this; }
		std::tuple<Args...> packed_args;

		template <class X>
		auto query(const iofox::packed_arg<X> &) const
		{
			return std::get<X>(packed_args);
		}

		template <class X>
		auto require(const iofox::packed_arg<X> & packed_arg) const
		{
			packed_executor<T, Args...> executor = *this;
			std::get<X>(executor.packed_args) = packed_arg.value;
			return executor;
		}

		decltype(auto) query(const auto & property) const requires boost::asio::can_query_v<T, decltype(property)>
		{
			return boost::asio::query(get_inner_executor(), property);
		}

		decltype(auto) require(const auto & property) const requires boost::asio::can_require_v<T, decltype(property)>
		{
			auto underlying_executor = boost::asio::require(get_inner_executor(), property);
			packed_executor<decltype(underlying_executor), Args...> executor {underlying_executor};
			executor.packed_args = packed_args;
			return executor;
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
		boost::asio::execution::prefer_only<iofox::packed_arg<int>>,
		boost::asio::execution::prefer_only<iofox::packed_arg<char>>
	>;
}

template <class... T>
struct packed_tuple
{
	std::tuple<T...> tuple;
	packed_tuple() = default;

	template <class... X>
	packed_tuple(const packed_tuple<X...> & other)
	{
		tuple = std::apply([](T... head, auto... tail) { return std::make_tuple(head...); }, other.tuple);
	}
};

TEST_CASE()
{
	SECTION("packed_executor <- system_executor")
	{
		iofox::packed_executor<boost::asio::system_executor, int, char> packed_executor;

		packed_executor = boost::asio::require(packed_executor, iofox::packed_arg<int>(45));
		packed_executor = boost::asio::require(packed_executor, iofox::packed_arg<char>('b'));

		auto value_int	= boost::asio::query(packed_executor, iofox::packed_arg<int>());
		auto value_char	= boost::asio::query(packed_executor, iofox::packed_arg<char>());

		fmt::print("arg int: '{}'.\n", value_int);
		fmt::print("arg char: '{}'.\n", value_char);
	}

	SECTION("any_executor <- packed_executor <- system_executor")
	{
		iofox::packed_executor<boost::asio::system_executor, int, char> packed_executor;
		iofox::any_executor any_executor {packed_executor};

		any_executor = boost::asio::prefer(any_executor, iofox::packed_arg<int>(88));
		any_executor = boost::asio::prefer(any_executor, iofox::packed_arg<char>('g'));

		auto value_int	= boost::asio::query(any_executor, iofox::packed_arg<int>());
		auto value_char	= boost::asio::query(any_executor, iofox::packed_arg<char>());

		fmt::print("arg int: '{}'.\n", value_int);
		fmt::print("arg char: '{}'.\n", value_char);
	}

	SECTION("downgrading")
	{
		packed_tuple<int, char, short, double, long> packed_tuple_1;
		packed_tuple_1.tuple = {42, 'a', 43.3, 55.56, 45678};
		fmt::print("0 -> '{}'.\n", packed_tuple_1.tuple);

		packed_tuple<int, char, short> packed_tuple_2 {packed_tuple_1};
		fmt::print("1 -> '{}'.\n", packed_tuple_2.tuple);
	}
}

// Concepts:
// -
// iofox::packed_executor<iofox::property::ssl_context, iofox::property::dns_resolver> executor;
// iofox::packed_executor<boost::asio::ssl_context, boost::asio::ip::tcp::resolver> executor;
// iofox::packed_executor<boost::asio::steady_timer> executor;
// -
// auto value = boost::asio::query(executor, iofox::packed_arg<boost::asio::ssl::context>);
// auto value = iofox::unpack_arg<boost::asio::ssl::context>(executor);
// auto value = iofox::unpack_arg<int>(executor);
// -
// auto packed_executor = iofox::make_packed_executor(executor, 10, 'a');
// iofox::packed_executor packed_executor {executor, 64, 'a'};
