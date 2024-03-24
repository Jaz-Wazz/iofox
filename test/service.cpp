// boost_asio
#include <boost/asio/bind_executor.hpp>
#include <boost/asio/execution/allocator.hpp>
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

// iofox
#include <fmt/core.h>
#include <iofox/coro.hpp>
#include <iofox/service.hpp>
#include <iofox/rethrowed.hpp>

// catch2
#include <catch2/catch_test_macros.hpp>

struct executor_wrapper: public boost::asio::io_context::executor_type
{
	// executor_wrapper(boost::asio::io_context & io_context): boost::asio::io_context::executor_type(io_context)
	// {

	// }

	// boost::asio::execution_context& query(
    //   boost::asio::execution::context_t) const noexcept
	// {
	// 	return *context_;
	// }

	// public: using boost::asio::io_context::basic_executor_type<std::allocator<void>, 0>::basic_executor_type;
};

template <int I = 0>
struct context_t
{
#if defined(BOOST_ASIO_HAS_VARIABLE_TEMPLATES)
  template <typename T>
  static constexpr bool is_applicable_property_v = boost::asio::is_executor<T>::value;
#endif // defined(BOOST_ASIO_HAS_VARIABLE_TEMPLATES)

  static constexpr bool is_requirable = false;
  static constexpr bool is_preferable = false;

#if defined(BOOST_ASIO_HAS_STD_ANY)
  typedef std::any polymorphic_query_result_type;
#endif // defined(BOOST_ASIO_HAS_STD_ANY)

  constexpr context_t()
  {
  }

  template <typename T>
  struct static_proxy
  {
#if defined(BOOST_ASIO_HAS_DEDUCED_QUERY_STATIC_CONSTEXPR_MEMBER_TRAIT)
    struct type
    {
      template <typename P>
      static constexpr auto query(P&& p)
        noexcept(
          noexcept(
            std::conditional_t<true, T, P>::query(static_cast<P&&>(p))
          )
        )
        -> decltype(
          std::conditional_t<true, T, P>::query(static_cast<P&&>(p))
        )
      {
        return T::query(static_cast<P&&>(p));
      }
    };
#else // defined(BOOST_ASIO_HAS_DEDUCED_QUERY_STATIC_CONSTEXPR_MEMBER_TRAIT)
    typedef T type;
#endif // defined(BOOST_ASIO_HAS_DEDUCED_QUERY_STATIC_CONSTEXPR_MEMBER_TRAIT)
  };

  template <typename T>
  struct query_static_constexpr_member :
    boost::asio::traits::query_static_constexpr_member<
      typename static_proxy<T>::type, context_t> {};

#if defined(BOOST_ASIO_HAS_DEDUCED_STATIC_QUERY_TRAIT) \
  && defined(BOOST_ASIO_HAS_SFINAE_VARIABLE_TEMPLATES)
  template <typename T>
  static constexpr typename query_static_constexpr_member<T>::result_type
  static_query()
    noexcept(query_static_constexpr_member<T>::is_noexcept)
  {
    return query_static_constexpr_member<T>::value();
  }

  template <typename E, typename T = decltype(context_t::static_query<E>())>
  static constexpr const T static_query_v = context_t::static_query<E>();
#endif // defined(BOOST_ASIO_HAS_DEDUCED_STATIC_QUERY_TRAIT)
       //   && defined(BOOST_ASIO_HAS_SFINAE_VARIABLE_TEMPLATES)
};

#if defined(BOOST_ASIO_HAS_DEDUCED_STATIC_QUERY_TRAIT) \
  && defined(BOOST_ASIO_HAS_SFINAE_VARIABLE_TEMPLATES)
template <int I> template <typename E, typename T>
const T context_t<I>::static_query_v;
#endif // defined(BOOST_ASIO_HAS_DEDUCED_STATIC_QUERY_TRAIT)
       //   && defined(BOOST_ASIO_HAS_SFINAE_VARIABLE_TEMPLATES)

typedef context_t<> context_tt;

constexpr context_tt context;

namespace boost::asio::traits
{
	// 	template <typename T>
	// struct static_query<T, context_tt,
	//   enable_if_t<
	//     context_t<0>::
	//       query_static_constexpr_member<T>::is_valid
	//   >>
	// {
	//   static constexpr bool is_valid = true;
	//   static constexpr bool is_noexcept = true;

	//   typedef typename context_t<0>::
	//     query_static_constexpr_member<T>::result_type result_type;

	//   static constexpr result_type value()
	//   {
	//     return context_t<0>::
	//       query_static_constexpr_member<T>::value();
	//   }
	// };

	// template <>
	// struct query_member<boost::asio::io_context::executor_type, context_tt>
	// {
	// 	static constexpr bool is_valid = true;
	// 	static constexpr bool is_noexcept = true;
	// 	typedef boost::asio::execution_context& result_type;
	// };

	// template <typename Property>
	// struct query_static_constexpr_member<boost::asio::io_context::executor_type, Property,
	// 	typename enable_if<
	// 	std::is_convertible<Property, boost::asio::execution::blocking_t>::value
	// 	>::type>
	// {
	// 	static constexpr bool is_valid = true;
	// 	static constexpr bool is_noexcept = true;
	// 	typedef boost::asio::execution::blocking_t::never_t result_type;
	// 	static constexpr result_type value() noexcept { return result_type(); }
	// };
}

// Implement the query customization point for the custom property

// template<typename Executor>
// int query(Executor& executor, int) {

//     // Implement the logic to obtain the custom property value from the executor

//     // This could involve accessing executor-specific information or state
//     // custom_property result;
//     // Populate result with the custom property value
//     return 0;
// }

// Define a custom property type
struct custom_property {
    // Define any properties or methods specific to your custom property
    // For example:
    int custom_value;
};


// Implement the query customization point for the custom property
template<typename Executor>
custom_property query(Executor& executor, custom_property) {
    // Implement the logic to obtain the custom property value from the executor
    // This could involve accessing executor-specific information or state
    custom_property result;
    // Populate result with the custom property value
    return result;
}

TEST_CASE("one")
{
	boost::asio::execution::context_t x;
	boost::asio::io_context io_context;
	executor_wrapper executor_wrapper {io_context.get_executor()};

	auto & result_x = boost::asio::query(io_context.get_executor(), boost::asio::execution::context);
	auto & result_y = boost::asio::query(executor_wrapper, boost::asio::execution::context);
	auto result_z = boost::asio::query(executor_wrapper, boost::asio::execution::allocator);
	// auto result_g = boost::asio::query(io_context, context);
	auto result_g = boost::asio::query(io_context.get_executor(), custom_property());
	// auto result_g = boost::asio::query(io_context, 5);

	// io_context.get_executor().
	// wrapper.

	auto coro = [] -> iofox::coro<void>
	{
		const auto executor = co_await boost::asio::this_coro::executor;
		fmt::print("[coro] - executor type name: '{}'.\n", executor.target_type().name());

		auto & result_y = boost::asio::query(executor, boost::asio::execution::context);
		// auto result_z = boost::asio::query(executor, boost::asio::execution::allocator);
		co_return;
	};

	boost::asio::co_spawn(executor_wrapper, coro(), iofox::rethrowed);
	io_context.run();
}
