// boost_asio
#include <boost/asio/co_spawn.hpp>
#include <boost/asio/error.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/tcp.hpp>

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
