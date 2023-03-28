#include <boost/beast/core/error.hpp>
#include <chrono>
#include <iofox/third_party/picohttpparser.h>
#include <boost/asio/buffer.hpp>
#include <boost/asio/co_spawn.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/registered_buffer.hpp>
#include <boost/asio/this_coro.hpp>
#include <boost/asio/use_awaitable.hpp>
#include <boost/beast/http/parser.hpp>
#include <boost/beast/http/empty_body.hpp>
#include <fmt/core.h>
#include <fmt/chrono.h>
#include <iostream>
#include <ranges>
#include <stdexcept>
#include <string>
#include <iofox/iofox.hpp>

namespace asio = boost::asio;			// NOLINT.
namespace this_coro = asio::this_coro;	// NOLINT.
namespace beast = boost::beast;			// NOLINT.

int main() try
{
	io::windows::set_asio_locale(io::windows::lang::english);
	std::string buffer = "GET /page.html HTTP/1.1\r\n\r\n";

	auto time_0 = std::chrono::steady_clock::now();
	{
		// for(int i = 0; i < 10'000'000; i++)
		for(int i = 0; i < 1'000'000; i++)
		{
			beast::http::request_parser<beast::http::empty_body> parser;
			beast::error_code ec;
			parser.put(asio::buffer(buffer), ec);
			auto request = parser.release();
		}
	}
	auto time_1 = std::chrono::steady_clock::now();
	fmt::print("Time: {:%S} sec.\n", time_1 - time_0);
	fmt::print("Time: {} sec.\n", std::chrono::duration_cast<std::chrono::seconds>(time_1 - time_0));
	// fmt::print("Time one request: {:%S} sec.\n", 10'000'000 / (time_1 - time_0).count());

	// Req Sec
	// 100 4
	//

	// fmt::print("status: '{}'.\n", ec.message());
	// fmt::print("{} {} {}\n", request.method_string(), request.target(), request.version());

	return 0;
}
catch(const std::exception & e)
{
	fmt::print("Exception: '{}'.\n", e.what());
}
