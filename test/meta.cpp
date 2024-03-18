// boost_beast
#include <boost/beast/http/empty_body.hpp>
#include <boost/beast/http/string_body.hpp>
#include <boost/beast/http/vector_body.hpp>

// stl
#include <string>
#include <vector>
#include <type_traits>

// iofox
#include <iofox/meta.hpp>

// catch2
#include <catch2/catch_test_macros.hpp>

TEST_CASE("meta")
{
	using a = iofox::meta::transform_body<void>;
	using b = iofox::meta::transform_body<std::string>;
	using c = iofox::meta::transform_body<std::vector<char>>;
	using d = iofox::meta::transform_body<int>;
	REQUIRE(std::is_same_v<a, boost::beast::http::empty_body>);
	REQUIRE(std::is_same_v<b, boost::beast::http::string_body>);
	REQUIRE(std::is_same_v<c, boost::beast::http::vector_body<char>>);
	REQUIRE(std::is_same_v<d, int>);
}
