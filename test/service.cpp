// boost_asio
#include <boost/asio/any_io_executor.hpp>
#include <boost/asio/buffer.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/co_spawn.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ssl/context.hpp>

// iofox
#include <chrono>
#include <functional>
#include <initializer_list>
#include <iofox/coro.hpp>
#include <iofox/rethrowed.hpp>

// fmt.
#include <fmt/core.h>

// stl.
#include <stdexcept>

// catch2
#include <catch2/catch_test_macros.hpp>

namespace iofox
{
	struct executor_wrapper: public boost::asio::io_context::executor_type
	{
		boost::asio::ssl::context * ssl_context = nullptr;
	};

	auto get_ssl_context(const boost::asio::any_io_executor & any_executor) -> boost::asio::ssl::context &
	{
		auto * ptr = any_executor.target<executor_wrapper>();
		if(ptr != nullptr) return *(ptr->ssl_context); else throw std::runtime_error("executor_not_contain_ssl_context");
	}

	auto make_executor_wrapper(boost::asio::io_context & io_context, boost::asio::ssl::context & ssl_context)
	{
		return executor_wrapper {io_context.get_executor(), &ssl_context};
	}
}

TEST_CASE("one")
{
	boost::asio::io_context io_context;
	boost::asio::ssl::context ssl_context_server {boost::asio::ssl::context::tls};
	boost::asio::ssl::context ssl_context_client {boost::asio::ssl::context::tls};
	auto executor_server = iofox::make_executor_wrapper(io_context, ssl_context_server);
	auto executor_client = iofox::make_executor_wrapper(io_context, ssl_context_client);

	auto coro = [](const char * name) -> iofox::coro<void>
	{
		auto & ssl_context = iofox::get_ssl_context(co_await boost::asio::this_coro::executor);
		fmt::print("[coro '{}'] - my ssl context is: '{}'.\n", name, (void *) &ssl_context);
		co_return;

		// auto & ssl_context	= iofox::get_ssl_context(executor);
		// auto & dns_resolver	= iofox::get_dns_resolver(executor);
		// auto & timeout		= iofox::get_timeout(executor);
		// auto & retry_handler	= iofox::get_retry_handler(executor);

		// co_await some_http_operation(); <- ssl_context, dns_resolver, timeout, retry_handler

		// auto executor = co_await iofox::this_coro::executor;


		// Structured concurency
		// iofox::retry(iofox::http::send(args...), timeout_handler());

		// Structured concurency
		// iofox::http::send(args...) | iofox::retry(timeout_handler());
	};

	boost::asio::co_spawn(executor_server, coro("server"), iofox::rethrowed);
	boost::asio::co_spawn(executor_client, coro("client"), iofox::rethrowed);
	io_context.run();
}

// auto respone = http_send("google.com", request, 15s);

struct op_teta
{
	iofox::coro<int> retry(auto handler)
	{
		return {};
	}
};

struct op_beta
{
	op_teta * timeout(int time)
	{
		return {};
	}
};

struct operation_socket_read
{
	op_beta * operator ->()
	{
		return {};
	}
};

operation_socket_read socket_read()
{
	return {};
}

#include <boost/asio/deferred.hpp>

namespace iofox
{
	template <class T>
	inline auto timeout(std::chrono::steady_clock::duration duration, std::initializer_list<T> op) -> iofox::coro<int>
	{
		co_return 0;
	}

	inline auto timeout(std::chrono::steady_clock::duration duration, auto op) -> iofox::coro<int>
	{
		co_return 0;
	}

	template <class T>
	inline auto retry(auto handler, std::initializer_list<T> op) -> iofox::coro<int>
	{
		co_return 0;
	}

	inline auto retry(auto handler, auto op) -> iofox::coro<int>
	{
		co_return 0;
	}

	template <class T>
	inline auto retry_timeout(auto handler, std::chrono::steady_clock::duration duration, std::initializer_list<T> op) -> iofox::coro<int>
	{
		co_return 0;
	}

	inline auto retry_timeout(auto handler, std::chrono::steady_clock::duration duration, auto op) -> iofox::coro<int>
	{
		co_return 0;
	}
}

auto foo() -> iofox::coro<void>
{
	// auto retry_handler = [](){};
	// auto result = co_await socket_read() -> timeout(10000) -> retry(retry_handler);

	// auto request = ...;
	// auto response = co_await iofox::send("https://google.com", request) -> timeout(10s) -> retry(retry_handler);

	// auto result = co_await socket.async_write_some(boost::asio::buffer(array, array_size), iofox::token)
	//						  -> timeout(10s)
	//						  -> retry(retry_handler);

	// auto result = co_await (socket.async_write_some(boost::asio::buffer(array, array_size), iofox::token)
	//						  | timeout(10s)
	//						  | retry(retry_handler));

	// auto result = co_await iofox::async_operation
	// {
	// 	socket.async_write_some(boost::asio::buffer(array, array_size), iofox::deffered),
	// 	iofox::timeout(10s),
	// 	iofox::retry(retry_handler)
	// };

	// auto result = iofox::retry(retry_handler, iofox::timeout(10s,
	// {
	// 	socket.async_write_some(boost::asio::buffer(array, array_size), iofox::deffered)
	// }));

	// auto result = iofox::retry_timeout(
	// {
	// 	socket.async_write_some(boost::asio::buffer(array, array_size), iofox::deffered)
	// }, 10s, retry_handler);

	// operation_alpha operation_alpha;
	// operation_alpha->

	boost::asio::io_context io_context;
	boost::asio::ip::tcp::socket socket {io_context};

	using namespace std::chrono_literals;

	// auto def = socket.async_write_some(boost::asio::buffer("sas"), boost::asio::deferred);

	// auto result = co_await iofox::timeout(15s, socket.async_write_some(boost::asio::buffer("sas"), boost::asio::deferred));

	// auto result = co_await iofox::timeout(15s,
	// {
	// 	socket.async_write_some(boost::asio::buffer("sas"), boost::asio::deferred)
	// });

	// auto handler = [](){};
	// auto result = co_await iofox::retry(handler, socket.async_write_some(boost::asio::buffer("sas"), boost::asio::deferred));

	// auto handler = [](){};
	// auto result = co_await iofox::retry(handler,
	// {
	// 	socket.async_write_some(boost::asio::buffer("sas"), boost::asio::deferred)
	// });

	// auto handler = [](){};
	// auto result = co_await iofox::retry_timeout(handler, 15s, socket.async_write_some(boost::asio::buffer("sas"), boost::asio::deferred));

	auto handler = [](){};
	auto result = co_await iofox::retry_timeout(handler, 15s,
	{
		socket.async_write_some(boost::asio::buffer("sas"), boost::asio::deferred)
	}); // <-- token &&&&&&

	// foox(socket.async_write_some(boost::asio::buffer("sas"), boost::asio::deferred));

	// auto handler = [](){};
	// auto op = socket.async_write_some(boost::asio::buffer("sas"), boost::asio::deferred);
	// auto result = co_await iofox::retry_timeout(handler, 15s, op);

	// auto result = co_await socket.async_write_some(boost::asio::buffer("sas"), iofox::co_rat(handler, 15s));

	co_return;
	// std::function<int()> func_ret_int = []{ co_return 5; };
}
