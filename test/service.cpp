// boost_asio
#include <boost/asio/any_io_executor.hpp>
#include <boost/asio/as_tuple.hpp>
#include <boost/asio/async_result.hpp>
#include <boost/asio/awaitable.hpp>
#include <boost/asio/bind_executor.hpp>
#include <boost/asio/buffer.hpp>
#include <boost/asio/detail/type_traits.hpp>
#include <boost/asio/error.hpp>
#include <boost/asio/experimental/cancellation_condition.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/co_spawn.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ssl/context.hpp>
#include <boost/asio/steady_timer.hpp>
#include <boost/asio/this_coro.hpp>
#include <boost/asio/use_awaitable.hpp>
#include <boost/asio/deferred.hpp>
#include <boost/asio/experimental/parallel_group.hpp>

// boost_system
#include <boost/system/error_code.hpp>

// boost_core
#include <boost/core/demangle.hpp>

// stl
#include <chrono> // IWYU pragma: keep

// iofox
#include <iofox/coro.hpp>
#include <iofox/rethrowed.hpp>
#include <iofox/this_thread.hpp>

// fmt.
#include <fmt/core.h>

// catch2
#include <catch2/catch_test_macros.hpp>

auto http_request(int i, char c) -> boost::asio::awaitable<void>
{
	fmt::print("garox.\n");
	co_return;
}

class custom_awaitable
{

};

// namespace boost::asio
// {
// 	template <>
// 	struct result_of<custom_awaitable (boost::asio::detail::awaitable_frame_base<boost::asio::any_io_executor> *)>
// 	{
// 		using type = boost::asio::awaitable<void>;
// 	};
// };

#define co_generator(x) []{ return x; }

template <auto & invocable>
auto wrapped_operation(auto... args)
{
	return invocable(args...);
};

namespace iofox
{
	auto repeat(int count, auto operation) -> iofox::coro<void>
	{
		for(int i = 0; i < count; i++)
		{
			co_await std::move(operation());
		}
	}

	auto run_multiply(auto & ref, auto... args)
	{
		return [=]{ return ref(args...); };
	}
}

// #include <iostream>
// #include <boost/asio.hpp>

// using namespace boost::asio;

// template <class Awaitable>
// struct Awaiter {
// 	int n;
// 	Awaitable awaitable;

// 	bool await_ready() { return false; }

// 	template <typename Promise>
// 	void await_suspend(auto handle) {
// 		if (n-- > 0) {
// 			awaitable.then([this, handle](auto&&... args) {
// 				repeat(n, awaitable).then([handle](auto&&... args) {
// 					handle.resume();
// 				});
// 			});
// 		} else {
// 			handle.resume();
// 		}
// 	}

// 	void await_resume() {}
// };

// Define a coroutine that repeats the awaitable n times
// template <typename Awaitable>
// auto repeat(int n, Awaitable awaitable) {


//     return Awaiter{n, std::move(awaitable)};
// }

struct custom_class
{
	operator boost::asio::awaitable<void> &&()
	{
		return {};
	}
};

auto coro() -> iofox::coro<void>
{
	custom_class sas;
	boost::asio::awaitable<void> aw {sas};
	co_await sas;

	co_await iofox::repeat(10, []{ return http_request(10, 'a'); });

	co_await iofox::retry_timeout(10s, retry_handler, []
	{
		return http_request(10, 'a');
	});

	// co_await repeat(10, http_request(10, 'a'));

	// auto awaitable = http_request(10, 'a');

	// if(!awaitable.await_ready())
	// {
	// 	awaitable.
	// }

	// co_await iofox::repeat(10, []{ return http_request(10, 'a'); });
	// co_await iofox::repeat(10, co_generator(http_request(10, 'a')));
	// co_await iofox::repeat(10, wrapped_operation<http_request>(15, 'a'));

	// co_await iofox::repeat(10, iofox::run_multiply(http_request, 10, 'a'));

	// co_await iofox::repeat(10, iofox::run_multiply(http_request, 10, 'a'));

	// auto generator = []{ return so(); };
	// auto generator = generator(so());



	// boost::asio::awaitable<void> awaitable = so();
	// custom_awaitable cu_awaitable;
	// co_await std::move(awaitable);
	// co_await std::move(cu_awaitable);


	// boost::asio::result_of<custom_awaitable(boost::asio::detail::awaitable_frame_base<boost::asio::any_io_executor> *)>
	// boost::asio::result_of<>;

	co_return;
}

TEST_CASE()
{
	iofox::this_thread::set_language("en_us");
	boost::asio::io_context io_context;
	boost::asio::co_spawn(io_context, coro(), iofox::rethrowed);
	io_context.run();
}
