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

struct custom_property_t
{
    int custom_value = 10;

	template <typename T>
	static constexpr bool is_applicable_property_v = boost::asio::is_executor<T>::value;

	static constexpr bool is_requirable = false;
	static constexpr bool is_preferable = false;

	typedef std::any polymorphic_query_result_type;

	constexpr custom_property_t() {}

	// template <typename Executor>
	// static constexpr decltype(auto) static_query_v = Executor::query(custom_property_t());
};

constexpr custom_property_t custom_property;

// template <typename Executor>
// custom_property boost::asio::query(Executor& executor, custom_property)
// {
//     // Implement the logic to obtain the custom property value from the executor
//     // This could involve accessing executor-specific information or state
//     custom_property result;

//     // Populate result with the custom property value
//     return result;
// }

TEST_CASE("one")
{
	boost::asio::execution::context_t x;
	boost::asio::io_context io_context;
	executor_wrapper executor_wrapper {io_context.get_executor()};

	auto & result_x = boost::asio::query(io_context.get_executor(), boost::asio::execution::context);
	auto & result_y = boost::asio::query(executor_wrapper, boost::asio::execution::context);
	auto result_z = boost::asio::query(executor_wrapper, boost::asio::execution::allocator);
	auto result_g = boost::asio::query(executor_wrapper, custom_property);

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
