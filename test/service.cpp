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

// fmt
#include <fmt/core.h>
#include <fmt/ranges.h>

// catch2
#include <catch2/catch_test_macros.hpp>
#include <tuple>

namespace iofox
{
	template <class T, class... Args>
	concept any_of = (std::same_as<T, Args> || ...);

	template <class T, class... Args>
	concept none_of = (!std::same_as<T, Args> && ...);

	template <class T, template <class...> class X>
	concept template_of = requires(T t){ []<class... Args>(X<Args...> &){}(t); };

	template <class T, class E>
	concept queryable_of = boost::asio::can_query_v<T, E>;

	template <class T, class E>
	concept requirable_of = boost::asio::can_require_v<T, E>;

	template <class T>
	struct packed_arg
	{
		template <class X> static constexpr bool is_applicable_property_v = true;
		static constexpr bool is_preferable = true;
		static constexpr bool is_requirable = true;
		using polymorphic_query_result_type = T *;
		T * ptr = nullptr;
		packed_arg() = default;
		packed_arg(T & ref): ptr(&ref) {}
	};

	template <class T, class... Args>
	concept unpackable_executor = boost::asio::execution::executor<T> && (iofox::queryable_of<T, iofox::packed_arg<Args>> && ...);

	template <boost::asio::execution::executor T, class... Args>
	struct packed_executor: public T
	{
		using inner_executor_type = T;
		std::tuple<Args &...> packed_args;

		packed_executor() requires (!iofox::template_of<T, iofox::packed_executor>) = default;

		packed_executor(const T & executor, Args &... args) requires (!iofox::template_of<T, iofox::packed_executor>)
		: T(executor), packed_args(std::forward_as_tuple(args...)) {}

		T get_inner_executor() const noexcept
		{
			return *this;
		}

		template <iofox::any_of<Args...> X>
		X * query(const iofox::packed_arg<X> &) const
		{
			return &std::get<X &>(packed_args);
		}

		template <iofox::any_of<Args...> X>
		auto require(const iofox::packed_arg<X> & packed_arg) const
		{
			auto transform = [&]<class U>(U & arg) -> auto & { if constexpr(std::same_as<U, X>) return *packed_arg.ptr; else return arg; };
			return std::apply([&](auto &... args) { return packed_executor(get_inner_executor(), transform(args)...); }, packed_args);
		}

		template <iofox::none_of<Args...> X>
		auto require(const iofox::packed_arg<X> & packed_arg) const
		{
			return std::apply([&](auto &... args) { return iofox::packed_executor(get_inner_executor(), args..., *packed_arg.ptr); }, packed_args);
		}

		template <iofox::queryable_of<T> X>
		decltype(auto) query(const X & property) const
		{
			return boost::asio::query(get_inner_executor(), property);
		}

		template <iofox::requirable_of<T> X>
		decltype(auto) require(const X & property)
		{
			auto executor = boost::asio::require(get_inner_executor(), property);
			return std::apply([&](auto &... args){ return iofox::packed_executor(executor, args...); }, packed_args);
		}
	};

	struct any_executor: boost::asio::execution::any_executor
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
	> {};

	template <class T, iofox::unpackable_executor<T> E>
	inline T & unpack_arg(const E & executor)
	{
		auto * ptr = boost::asio::query(executor, iofox::packed_arg<T>());
		return (ptr != nullptr) ? *ptr : throw std::runtime_error("err");
	}
}

template <boost::asio::execution::executor T>
void some_async_operation(const T & executor, int value_int, char value_char)
{
	fmt::print("[some_async_operation] - int: '{}', char: '{}'.\n", value_int, value_char);
}

template <iofox::unpackable_executor<int, char> T>
void some_async_operation(const T & executor)
{
	int & ref_int	= iofox::unpack_arg<int>(executor);
	char & ref_char	= iofox::unpack_arg<char>(executor);
	some_async_operation(executor, ref_int, ref_char);
}

TEST_CASE()
{
	// Values.
	int value_int = 10;
	char value_char = 'a';

	// Make packed executor.
	iofox::packed_executor packed_executor {boost::asio::system_executor(), value_int};

	// Pack values.
	auto new_executor = boost::asio::require(packed_executor, iofox::packed_arg<int>(value_int), iofox::packed_arg<char>(value_char));

	// Invoke async operation.
	some_async_operation(new_executor);
}
