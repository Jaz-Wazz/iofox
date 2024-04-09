// boost_asio
#include <boost/asio/any_io_executor.hpp>
#include <boost/asio/execution/any_executor.hpp>
#include <boost/asio/execution/blocking.hpp>
#include <boost/asio/execution/context.hpp>
#include <boost/asio/execution/executor.hpp>
#include <boost/asio/executor.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/query.hpp>
#include <boost/asio/require.hpp>
#include <boost/asio/ssl/context.hpp>
#include <boost/asio/steady_timer.hpp>
#include <boost/asio/system_executor.hpp>

// stl
#include <concepts> // IWYU pragma: keep
#include <stdexcept>
#include <type_traits>

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

		packed_executor() = default;
		packed_executor(const T & executor): T(executor) {}

		packed_executor(const T & executor, Args... args) requires (sizeof...(Args) > 0)
		: T(executor), packed_args(std::make_tuple(args...)) {}

		template <class... AnotherArgs>
		packed_executor(const packed_executor<T, AnotherArgs...> & other): T(other.get_inner_executor())
		{
			auto assign = [&](auto & arg){ arg = std::get<std::decay_t<decltype(arg)>>(other.packed_args); };
			std::apply([&](auto &... arg){ (assign(arg), ...); }, packed_args);
		}

		template <class X>
		auto query(const iofox::packed_arg<X> &) const requires (std::same_as<X, Args> || ...)
		{
			return std::get<X>(packed_args);
		}

		template <class X>
		auto require(const iofox::packed_arg<X> & packed_arg) const requires (std::same_as<X, Args> || ...)
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

	template <class T, class... Args>
	packed_executor(T, Args...) -> packed_executor<T, Args...>;

	using any_executor = boost::asio::execution::any_executor
	<
		boost::asio::execution::context_as_t<boost::asio::execution_context&>,
		boost::asio::execution::blocking_t::never_t,
		boost::asio::execution::prefer_only<boost::asio::execution::blocking_t::possibly_t>,
		boost::asio::execution::prefer_only<boost::asio::execution::outstanding_work_t::tracked_t>,
		boost::asio::execution::prefer_only<boost::asio::execution::outstanding_work_t::untracked_t>,
		boost::asio::execution::prefer_only<boost::asio::execution::relationship_t::fork_t>,
		boost::asio::execution::prefer_only<boost::asio::execution::relationship_t::continuation_t>,
		boost::asio::execution::prefer_only<iofox::packed_arg<int *>>,
		boost::asio::execution::prefer_only<iofox::packed_arg<char *>>
	>;

	template <class T>
	concept executor = boost::asio::execution::executor<T>;

	template <class T, class... Args>
	concept unpackable_executor = requires(T executor)
	{
		(boost::asio::query(executor, iofox::packed_arg<Args>()), ...);
	};
}

template <iofox::executor T>
void some_async_operation(T executor, int value_int, char value_char)
{
	fmt::print("[some_async_operation] - int: '{}', char: '{}'.\n", value_int, value_char);
}

template <iofox::unpackable_executor<int *, char *> T>
void some_async_operation(T executor)
{
	int * ptr_int	= boost::asio::query(executor, iofox::packed_arg<int *>());
	char * ptr_char	= boost::asio::query(executor, iofox::packed_arg<char *>());
	some_async_operation(executor, *ptr_int, *ptr_char);
}

TEST_CASE()
{
	int int_value = 10;
	char c = 'a';
	iofox::packed_executor packed_executor_ok	{boost::asio::system_executor(), &c, &int_value};
	iofox::packed_executor packed_executor_fail	{boost::asio::system_executor(), &c};
	iofox::any_executor any_executor;

	// Full signature.
	some_async_operation(boost::asio::system_executor(), 15, 'a');
	some_async_operation(any_executor, 15, 'a');
	some_async_operation(packed_executor_ok, 15, 'a');

	// Agruments from packed_executor.
	some_async_operation(any_executor);
	some_async_operation(packed_executor_ok);
	// some_async_operation(packed_executor_fail);
}
