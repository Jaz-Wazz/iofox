// boost_beast
#include <boost/beast/http/empty_body.hpp>
#include <boost/beast/http/string_body.hpp>
#include <boost/beast/http/vector_body.hpp>
#include <boost/beast/http/buffer_body.hpp>
#include <boost/beast/http/dynamic_body.hpp>
#include <boost/beast/http/file_body.hpp>
#include <boost/beast/http/span_body.hpp>

// stl
#include <string>
#include <tuple>
#include <vector>

// iofox
#include <iofox/http.hpp>

// catch2
#include <catch2/catch_template_test_macros.hpp>

using body_types = std::tuple
<
	void,
	std::string,
	std::vector<char>,
	boost::beast::http::empty_body,
	boost::beast::http::string_body,
	boost::beast::http::vector_body<char>,
	boost::beast::http::buffer_body,
	boost::beast::http::dynamic_body,
	boost::beast::http::file_body,
	boost::beast::http::span_body<char>
>;

TEMPLATE_LIST_TEST_CASE("request", "", body_types)
{
	iofox::http::headers headers = {{"key", "val"}, {"foo", "bar"}};
	iofox::http::request<TestType> request {"GET", "/", headers};

	REQUIRE(request.method_string() == "GET");
	REQUIRE(request.target() == "/");
	REQUIRE(request.at("key") == "val");
	REQUIRE(request.at("foo") == "bar");
}

TEMPLATE_LIST_TEST_CASE("response", "", body_types)
{
	iofox::http::headers headers = {{"key", "val"}, {"foo", "bar"}};
	iofox::http::response<TestType> response {200, headers};

	REQUIRE(response.result_int() == 200);
	REQUIRE(response.at("key") == "val");
	REQUIRE(response.at("foo") == "bar");
}
