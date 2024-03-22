// boost_asio
#include <boost/asio/bind_executor.hpp>
#include <boost/asio/execution_context.hpp>
#include <boost/asio/impl/execution_context.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/co_spawn.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ssl/context.hpp>
#include <boost/asio/this_coro.hpp>

// iofox
#include <fmt/core.h>
#include <iofox/coro.hpp>
#include <iofox/service.hpp>
#include <iofox/rethrowed.hpp>

// catch2
#include <catch2/catch_test_macros.hpp>

TEST_CASE("service")
{
	iofox::service<int> service;

	auto coro = [&] -> iofox::coro<void>
	{
		int & value = co_await service.get_or_make(0);
		value++;
		REQUIRE(value <= 10);
		co_return;
	};

	boost::asio::io_context context_x, context_y;
	for(int i = 0; i < 10; i++) boost::asio::co_spawn(context_x, coro(), iofox::rethrowed);
	for(int i = 0; i < 10; i++) boost::asio::co_spawn(context_y, coro(), iofox::rethrowed);
	context_x.run();
	context_y.run();
}

struct ssl_context_service: boost::asio::execution_context::service, boost::asio::ssl::context
{
	inline static boost::asio::execution_context::id id;
	ssl_context_service(boost::asio::execution_context & context)
	: boost::asio::execution_context::service(context), boost::asio::ssl::context(boost::asio::ssl::context::tls) {}
	void shutdown() {}
};

TEST_CASE("another_service")
{
	boost::asio::io_context context;

	auto coro = [&] -> iofox::coro<void>
	{
		const auto & executor = co_await boost::asio::this_coro::executor;
		auto & service = boost::asio::use_service<ssl_context_service>(executor.context());
		// service.set_verify_callback();

		boost::asio::use_service<ssl_context_service>((co_await boost::asio::this_coro::executor).context()).set_verify_callback(nullptr);
		get_ssl_context(co_await boost::asio::this_coro::executor).set_verify_callback(nullptr);
		get_ssl_context().set_verify_callback(nullptr);
		global_ssl_context->set_verify_callback(nullptr);
		global_ssl_context::set_verify_callback(nullptr);
		global_ssl_context::set_verify_callback(executor, nullptr);
		global_ssl_context::instance(nullptr);

		// asio style.
		// object object {executor};
		// object.async_operation(args..., token);
		// object.operation(args...);

		// &&&
		// ninja_ssl_context ssl_context {executor};
		// ssl_context.

		// &&&
		// auto & ssl_context = get_ssl_context(executor);
		// ssl_context.sas();

		// ssl_context_pool ???

		// iofox::tls::context_pool::

		auto coro_server = [] -> iofox::coro<void>
		{
			const auto executor = co_await boost::asio::this_coro::executor;
			// http_server server {executor};
			co_return;
		};

		auto coro_client = [] -> iofox::coro<void>
		{
			const auto executor = co_await boost::asio::this_coro::executor;
			// http_client client {executor};
			co_return;
		};

		// some magic: [executor] + [associated ssl_context].
		auto & executor_for_server = context;
		auto & executor_for_client = context;

		boost::asio::co_spawn(executor_for_server, coro_server(), iofox::rethrowed);
		boost::asio::co_spawn(executor_for_client, coro_client(), iofox::rethrowed);

		co_return;
	};

	boost::asio::co_spawn(context, coro(), iofox::rethrowed);
	context.run();
}

// namespace iofox::this_context
// {
// 	boost::asio::ip::tcp::resolver * dns_resolver = nullptr;
// };

// class service
// {
// 	iofox::coro<void> operator co_await()
// 	{
// 		co_return;
// 	}
// };


// service.async_make();
// service.make(executor);

// service.async_op(boost::asio::bind_executor(context, io::use_coro));

// [constructor parameter pack] [executor]
// co_await service.async_op(executor, construct_args, default_args);

// auto executor = ...;
// auto & resolver = iofox::resolver_service(executor);
// auto & resolver = co_await iofox::resolver_service(executor);
//

// struct serv: public boost::asio::execution_context::service
// {
// 	using key_type = serv;
// 	using id = serv;
// 	serv(boost::asio::execution_context & ctx): boost::asio::execution_context::service(ctx) {}
// 	void shutdown() {}
// };

// void my_handler(const boost::system::error_code & error)
// {
// 	auto ex = boost::asio::get_associated_executor(my_handler);
// 	fmt::print("handler, has_service: '{}'.\n", boost::asio::has_service<serv>(ex.context()));
// }

// TEST_CASE()
// {
// 	boost::asio::io_context context;
// 	auto e = boost::asio::bind_executor(context, 5);

// 	boost::asio::steady_timer timer {context, std::chrono::seconds(5)};
// 	boost::asio::make_service<serv>(context);
// 	fmt::print("main, has_service: '{}'.\n", boost::asio::has_service<serv>(context));
// 	timer.async_wait(my_handler);
// 	context.run();
// }

// TEST_CASE()
// {
	// auto coro = [&] -> iofox::coro<void>
	// {
		// async_op(..., handler);
		// service.get_or_make(..., handler);

		// service.get_or_make(io::use_coro);
		// service.get_or_make(executor, [](int & val){});

		// service.extract(context).async_resolve();
		// service[context].async_resolve();

		// iofox::this_context::dns_resolver(context)::async_resolve();

		// service s;
		// co_await s;
		// co_return;
	// };

	// boost::asio::io_context context;
	// iofox::service<int> service {context};







	// iofox::this_context::dns_resolver->async_resolve();
	// iofox::this_context::ssl_context;

	// auto executor = co_await boost::asio::this_coro::executor;
	// auto ssl_context = co_await iofox::this_context::ssl_context;

	// boost::asio::io_context context;
	// boost::asio::ip::tcp::acceptor acceptor {context};

	// iofox::dns::resolve("http", "exmaple.com");
	// (co_await iofox::ssl::context()).some_foo();

	// dns_resolver
	// ssl_context

	// iofox::dns::resolver;
	// iofox::ssl::context;

	// iofox::this_context::dns_resolver;
	// iofox::this_context::ssl_context;
// }
