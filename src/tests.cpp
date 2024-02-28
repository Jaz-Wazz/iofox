// Boost.Asio.
#include <boost/asio/co_spawn.hpp>
#include <boost/asio/error.hpp>
#include <boost/asio/io_context.hpp>

// Boost.Beast.
#include <boost/beast/http/empty_body.hpp>
#include <boost/beast/http/error.hpp>

// Stl.
#include <cstdint>
#include <string>
#include <vector>

// Other.
#include <catch2/catch_test_macros.hpp>
#include <fmt/core.h>
#include <iofox.hpp>

TEST_CASE("is_common_disconnect")
{
	REQUIRE(io::error::is_common_disconnect(boost::asio::error::connection_aborted) == true);
	REQUIRE(io::error::is_common_disconnect(boost::asio::error::connection_reset) == true);
	REQUIRE(io::error::is_common_disconnect(boost::asio::error::connection_refused) == true);
	REQUIRE(io::error::is_common_disconnect(boost::beast::http::error::end_of_stream) == true);

	REQUIRE(io::error::is_common_disconnect(boost::asio::error::not_connected) == false);
	REQUIRE(io::error::is_common_disconnect(boost::beast::http::error::bad_method) == false);
}

TEST_CASE("service_mechanic")
{
	io::service<int> service;

	auto coro = [&] -> io::coro<void>
	{
		int & value = co_await service.get_or_make(0);
		value++;
		REQUIRE(value <= 10);
		co_return;
	};

	boost::asio::io_context context_x, context_y;
	for(int i = 0; i < 10; i++) boost::asio::co_spawn(context_x, coro(), io::rethrowed);
	for(int i = 0; i < 10; i++) boost::asio::co_spawn(context_y, coro(), io::rethrowed);
	context_x.run();
	context_y.run();
}

TEST_CASE("request")
{
	SECTION("void")
	{
		io::http::request request;
		REQUIRE(typeid(request.body()) == typeid(boost::beast::http::empty_body::value_type));
	}

	SECTION("std::string")
	{
		io::http::request<std::string> request;
		REQUIRE(typeid(request.body()) == typeid(std::string));
	}

	SECTION("std::vector<char>")
	{
		io::http::request<std::vector<char>> request;
		REQUIRE(typeid(request.body()) == typeid(std::vector<char>));
	}

	SECTION("std::vector<std::int8_t>")
	{
		io::http::request<std::vector<std::int8_t>> request;
		REQUIRE(typeid(request.body()) == typeid(std::vector<std::int8_t>));
	}
}

TEST_CASE("response")
{
	SECTION("void")
	{
		io::http::response response;
		REQUIRE(typeid(response.body()) == typeid(boost::beast::http::empty_body::value_type));
	}

	SECTION("std::string")
	{
		io::http::response<std::string> response;
		REQUIRE(typeid(response.body()) == typeid(std::string));
	}

	SECTION("std::vector<char>")
	{
		io::http::response<std::vector<char>> response;
		REQUIRE(typeid(response.body()) == typeid(std::vector<char>));
	}

	SECTION("std::vector<std::int8_t>")
	{
		io::http::response<std::vector<std::int8_t>> response;
		REQUIRE(typeid(response.body()) == typeid(std::vector<std::int8_t>));
	}
}
