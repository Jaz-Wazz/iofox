// boost_beast
#include <boost/beast/http/empty_body.hpp>

// stl
#include <cstdint>
#include <string>
#include <vector>

// iofox
#include <iofox/http.hpp>

// catch2
#include <catch2/catch_test_macros.hpp>

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
