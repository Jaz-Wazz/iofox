#include "io_test.hpp"
#include <array>
#include <boost/asio/co_spawn.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/this_coro.hpp>
#include <boost/beast/http/read.hpp>
#include <fmt/core.h>
#include <iofox.hpp>
#include <iostream>
#include <string>
#include <string_view>
#include <span>
#include <vector>

namespace asio = boost::asio;			// NOLINT.
namespace beast = boost::beast;			// NOLINT.
namespace http = beast::http;			// NOLINT.
namespace this_coro = asio::this_coro;	// NOLINT.

void test_request()
{
	io_test::parser parser;
	parser.push("GET /gar");
	parser.push("ox.ht");
	parser.push("ml HTTP/");
	parser.push("1.1\r\n");
	parser.push("Host: exmaple.com\r\n");
	parser.push("ears: 2\r\n");
	parser.push("\r\n");

	if(auto method = parser.method())
	{
		fmt::print("Method: '{}'.\n", method.value());
	}
	if(auto path = parser.path())
	{
		fmt::print("Path: '{}'.\n", path.value());
	}
	if(auto version = parser.version())
	{
		fmt::print("Version: '{}'.\n", version.value());
	}
	parser.print_headers();
}

int main() try
{
	test_request();
	return 0;
}
catch(std::exception & e) { fmt::print("Exception: '{}'.\n", e.what()); }
catch(...) { fmt::print("Exception: 'unknown'.\n"); }
