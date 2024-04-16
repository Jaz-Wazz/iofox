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
	template <std::default_initializable T>
	struct packed_arg
	{
		template <class X> static constexpr bool is_applicable_property_v = true;
		static constexpr bool is_preferable = true;
		static constexpr bool is_requirable = true;
		using polymorphic_query_result_type = T;
		T value {};
	};

	template <boost::asio::execution::executor T, std::default_initializable... Args>
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

	template <boost::asio::execution::executor T, std::default_initializable... Args>
	packed_executor(T, Args...) -> packed_executor<T, Args...>;

	struct any_executor: boost::asio::execution::any_executor
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
	> {};

	template <class T, class... Args>
	concept unpackable_executor = requires(T executor)
	{
		requires boost::asio::execution::executor<T>;
		(boost::asio::query(executor, iofox::packed_arg<Args>()), ...);
	};

	template <class T>
	inline T & unpack_arg(const boost::asio::execution::executor auto & executor)
	{
		T * ptr = boost::asio::query(executor, iofox::packed_arg<T *>());
		return (ptr != nullptr) ? *ptr : throw std::runtime_error("err");
	}

	template <boost::asio::execution::executor T, class... Args>
	inline auto make_packed_executor(const T & executor, Args &&... args)
	{
		auto transform = []<class X>(X && arg) { if constexpr(std::is_lvalue_reference_v<X>) return &arg; else return arg; };
		return iofox::packed_executor(executor, transform(std::forward<Args>(args))...);
	}
}

template <boost::asio::execution::executor T>
void some_async_operation(const T & executor, int value_int, char value_char)
{
	fmt::print("[some_async_operation] - int: '{}', char: '{}'.\n", value_int, value_char);
}

template <iofox::unpackable_executor<int *, char *> T>
void some_async_operation(const T & executor)
{
	int & ptr_int	= iofox::unpack_arg<int>(executor);
	char & ptr_char	= iofox::unpack_arg<char>(executor);
	some_async_operation(executor, ptr_int, ptr_char);
}

TEST_CASE()
{
	// Values.
	int value_int = 10;
	char value_char = 'a';

	// Make packed executor.
	auto packed_executor = iofox::make_packed_executor(boost::asio::system_executor(), value_int, value_char, 42);

	// Invoke async operation.
	some_async_operation(boost::asio::system_executor(), value_int, value_char);
	some_async_operation(packed_executor);
}
