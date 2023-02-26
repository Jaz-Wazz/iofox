#include <boost/asio/co_spawn.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/this_coro.hpp>
#include <fmt/core.h>
#include <iofox.hpp>
#include <iostream>
#include <string>
#include <string_view>
#include <span>
#include <vector>
#include <picohttpparser.h>

namespace asio = boost::asio;			// NOLINT.
namespace beast = boost::beast;			// NOLINT.
namespace http = beast::http;			// NOLINT.
namespace this_coro = asio::this_coro;	// NOLINT.

void test_request()
{
	const char * method;
	size_t method_len;
	const char * path;
	size_t path_len;
	int minor_version;
	struct phr_header headers[4];
	size_t num_headers;

	const char * buf = "GET /page.sas HTTP/1.1\r\n";
	int ret = phr_parse_request(buf, 26, &method, &method_len, &path, &path_len, &minor_version, headers, &num_headers, 0);
	fmt::print("Return: '{}'.\n\n", ret);

	fmt::print("Method: '{}'.\n", std::string(method, method_len));
	fmt::print("Path: '{}'.\n", std::string(path, path_len));
	fmt::print("Minor version: '{}'.\n", minor_version);
	fmt::print("Headers count: '{}'.\n", num_headers);
}

int main() try
{
	test_request();
	return 0;
}
catch(std::exception & e) { fmt::print("Exception: '{}'.\n", e.what()); }
catch(...) { fmt::print("Exception: 'unknown'.\n"); }
