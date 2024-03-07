// boost_asio
#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/tcp.hpp>

// boost_system
#include <boost/system/error_code.hpp>

// iofox
#include <iofox/this_thread.hpp>

// catch2
#include <catch2/catch_test_macros.hpp>

TEST_CASE("this_thread_language")
{
	iofox::this_thread::set_language("en_us");
	boost::asio::io_context context;
	boost::asio::ip::tcp::resolver resolver {context};
	boost::system::error_code ec;
	resolver.resolve("host_which_not_exist.com", "http", ec);

	#ifdef _WIN32
		REQUIRE(ec.message() == "No such host is known");
	#endif
}
