// boost_asio
#include <boost/asio/error.hpp>

// boost_beast
#include <boost/beast/core/error.hpp>
#include <boost/beast/http/error.hpp>

// boost_system
#include <boost/system/error_code.hpp>

// iofox
#include <iofox/error.hpp>

// catch2
#include <catch2/catch_test_macros.hpp>

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
