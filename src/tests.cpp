// boost_asio
#include <boost/asio/co_spawn.hpp>
#include <boost/asio/error.hpp>
#include <boost/asio/io_context.hpp>

// boost_beast
#include <boost/beast/http/empty_body.hpp>
#include <boost/beast/http/error.hpp>

// boost_system
#include <boost/system/error_code.hpp>

// stl
#include <cstdint>
#include <string>
#include <vector>

// iofox
#include <iofox/coro.hpp>
#include <iofox/error.hpp>
#include <iofox/service.hpp>
#include <iofox/rethrowed.hpp>
#include <iofox/http.hpp>
#include <iofox/send.hpp>
#include <iofox/this_thread.hpp>

// other
#include <catch2/catch_test_macros.hpp>
#include <fmt/core.h>

TEST_CASE("is_common_disconnect")
{
	REQUIRE(iofox::error::is_common_disconnect(boost::asio::error::connection_aborted) == true);
	REQUIRE(iofox::error::is_common_disconnect(boost::asio::error::connection_reset) == true);
	REQUIRE(iofox::error::is_common_disconnect(boost::asio::error::connection_refused) == true);
	REQUIRE(iofox::error::is_common_disconnect(boost::beast::http::error::end_of_stream) == true);

	REQUIRE(iofox::error::is_common_disconnect(boost::asio::error::not_connected) == false);
	REQUIRE(iofox::error::is_common_disconnect(boost::beast::http::error::bad_method) == false);
}

TEST_CASE("is_common_timeout")
{
	REQUIRE(iofox::error::is_common_timeout(boost::asio::error::timed_out) == true);
	REQUIRE(iofox::error::is_common_timeout(boost::beast::error::timeout) == true);
	REQUIRE(iofox::error::is_common_timeout(boost::system::error_code(121, boost::asio::error::system_category)) == true);

	REQUIRE(iofox::error::is_common_timeout(boost::asio::error::not_connected) == false);
	REQUIRE(iofox::error::is_common_timeout(boost::beast::http::error::bad_method) == false);
}

TEST_CASE("service_mechanic")
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

TEST_CASE("request")
{
	SECTION("void")
	{
		iofox::http::request request;
		REQUIRE(typeid(request.body()) == typeid(boost::beast::http::empty_body::value_type));
	}

	SECTION("std::string")
	{
		iofox::http::request<std::string> request;
		REQUIRE(typeid(request.body()) == typeid(std::string));
	}

	SECTION("std::vector<char>")
	{
		iofox::http::request<std::vector<char>> request;
		REQUIRE(typeid(request.body()) == typeid(std::vector<char>));
	}

	SECTION("std::vector<std::int8_t>")
	{
		iofox::http::request<std::vector<std::int8_t>> request;
		REQUIRE(typeid(request.body()) == typeid(std::vector<std::int8_t>));
	}
}

TEST_CASE("response")
{
	SECTION("void")
	{
		iofox::http::response response;
		REQUIRE(typeid(response.body()) == typeid(boost::beast::http::empty_body::value_type));
	}

	SECTION("std::string")
	{
		iofox::http::response<std::string> response;
		REQUIRE(typeid(response.body()) == typeid(std::string));
	}

	SECTION("std::vector<char>")
	{
		iofox::http::response<std::vector<char>> response;
		REQUIRE(typeid(response.body()) == typeid(std::vector<char>));
	}

	SECTION("std::vector<std::int8_t>")
	{
		iofox::http::response<std::vector<std::int8_t>> response;
		REQUIRE(typeid(response.body()) == typeid(std::vector<std::int8_t>));
	}
}

TEST_CASE("send")
{
	auto coro_x = [&] -> iofox::coro<void>
	{
		iofox::http::request request {"POST", "/post", {{"Host", "httpbin.org"}}};
		iofox::http::response response = co_await iofox::http::send("https://httpbin.org", request);
	};

	auto coro_y = [&] -> iofox::coro<void>
	{
		iofox::http::request request {"POST", "/post", {{"Host", "httpbin.org"}}};
		iofox::http::response<std::vector<char>> response = co_await iofox::http::send<std::vector<char>>("https://httpbin.org", request);
	};

	boost::asio::io_context context;
	boost::asio::co_spawn(context, coro_x(), iofox::rethrowed);
	boost::asio::co_spawn(context, coro_y(), iofox::rethrowed);
	context.run();
}

TEST_CASE("this_thread_language")
{
	iofox::this_thread::set_language("en_us");
}
