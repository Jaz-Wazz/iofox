// boost_asio
#include <boost/asio/io_context.hpp>
#include <boost/asio/co_spawn.hpp>

// stl
#include <vector>

// iofox
#include <iofox/coro.hpp>
#include <iofox/rethrowed.hpp>
#include <iofox/http.hpp>
#include <iofox/send.hpp>

// catch2
#include <catch2/catch_test_macros.hpp>

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
