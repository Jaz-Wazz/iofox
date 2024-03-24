// boost_asio
#include <boost/asio/any_io_executor.hpp>
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
#include <stdexcept>

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
	};

	boost::asio::co_spawn(executor_server, coro("server"), iofox::rethrowed);
	boost::asio::co_spawn(executor_client, coro("client"), iofox::rethrowed);
	io_context.run();
}
