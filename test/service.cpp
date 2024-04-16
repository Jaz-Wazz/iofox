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

	struct custom_any_executor: iofox::any_executor
	{
		template <class T>
		auto & query(const iofox::packed_arg<T> & property) const
		{
			auto * ptr = boost::asio::query(static_cast<iofox::any_executor>(*this), property);
			return (ptr != nullptr) ? *ptr : throw std::runtime_error("bad_property_access");
		}

		decltype(auto) query(const auto & property) const
		{
			return boost::asio::query(static_cast<iofox::any_executor>(*this), property);
		}
	};
}

TEST_CASE()
{
	iofox::packed_arg<int> x;
	iofox::packed_arg<int *> y;
	iofox::packed_arg<int &> z;

	iofox::packed_executor<boost::asio::system_executor, int> packed_executor_x;
	iofox::packed_executor<boost::asio::system_executor, int *> packed_executor_y;
	iofox::packed_executor<boost::asio::system_executor, int &> packed_executor_z;

	int i = 42;
	iofox::packed_executor packed_executor_xx {boost::asio::system_executor(), 42};
	iofox::packed_executor packed_executor_yy {boost::asio::system_executor(), &i};
	iofox::packed_executor packed_executor_zz {boost::asio::system_executor(), std::ref(i)};

	// iofox::packed_executor packed_executor_xxx {42};


	// Construct by reference OR pointer.
	// iofox::packed_executor packed_executor {ref_a, ref_b, ptr_a, ptr_b};

	// Set by reference OR pointer.
	// boost::asio::require(packed_executor, iofox::packed_arg<int &>(ref_a));
	// boost::asio::require(packed_executor, iofox::packed_arg<int *>(ptr_a));

	// Get by reference OR pointer.
	// auto ref_a = boost::asio::query(packed_executor, iofox::packed_arg<int &>());
	// auto ptr_a = boost::asio::query(packed_executor, iofox::packed_arg<int *>());
	// ...
	// auto value = boost::asio::query(packed_executor, iofox::packed_arg<int>());

	// reference_arg<int>
	// value_arg<int>
	// pointer_argt<int>

	// packed_value
	// packed_referece
	// packed_pointer

	// - Хранилище уникальных типов, но они DefaultInitializable. (some value and pointers)
	// - Хранилище уникальных ссылок и указателей.

	// - Значение, но оно должно быть DefaultInitializable.	(В случае отсутсвия: Значение по умолчанию.)
	// - Указатель.											(В случае отсутсвия: Указатель на nullptr.)
	// - Ссылку.											(В случае отсутсвия: Exception.)

	// retry_handler - Спортный момент.
	// Он должен быть ОДНОГО ТИПА, для использования как iofox::packed_arg<T>.
	// А может он быть:
	// - Function ptr with void() signature.
	// - Function ptr with void(auto handler) signature.
	// - Callable (lambd) with with void() signature.
	// - Callable (lambd) with void(auto handler) signature.
	// - Каллабля или фонкщшон поинтер с void() но с возвратом -> iofox::coro

	// Внутри - указатели, Снаружи - ссылки.
	// Внутри - значения, Снаружи значения.

	// iofox::ssl_context ssl_contex;
	// iofox::dns_resolver dns_resolver;
	// iofox::steady_timer steady_timer;
	// ...
	// iofox::packed_executor packed_executor {boost::asio::system_executor(), ssl_context, dns_resolver, steady_timer};
	// ...
	// auto & value = boost::asio::query(packed_executor, iofox::packed_arg<ssl_context &>());

	// iofox::ssl_context ssl_contex;
	// iofox::dns_resolver dns_resolver;
	// iofox::steady_timer steady_timer;
	// ...
	// iofox::packed_executor packed_executor {boost::asio::system_executor(), &ssl_context, &dns_resolver, &steady_timer};
	// ...
	// auto * value = boost::asio::query(packed_executor, iofox::packed_arg<ssl_context *>());

	// iofox::ssl_context ssl_contex;
	// iofox::dns_resolver dns_resolver;
	// iofox::steady_timer steady_timer;
	// ...
	// iofox::packed_executor packed_executor {boost::asio::system_executor(), ssl_context, dns_resolver, steady_timer};
	// ...
	// auto & value = boost::asio::query(packed_executor, iofox::packed_arg<ssl_context &>());
	// auto & value = boost::asio::query(packed_executor, iofox::packed_arg<ssl_context *>());

	int i = 10;
	iofox::packed_executor packed_executor {boost::asio::system_executor(), &i};
	iofox::custom_any_executor custom_any_executor {packed_executor};
	auto & value_x	= boost::asio::query(custom_any_executor, iofox::packed_arg<int *>());
	auto & value_y	= boost::asio::query(custom_any_executor, boost::asio::execution::context);
}
