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

struct results
{
	std::string_view method;
	std::string_view path;
	int minor_version;
	std::size_t num_headers;
	int code;
};

results parse(const char * buffer, std::size_t size, phr_header * headers, std::size_t last_len)
{
	const char * method, * path;
	size_t method_len, path_len, num_headers;
	int minor_version;
	auto ret = phr_parse_request(buffer, size, &method, &method_len, &path, &path_len, &minor_version, headers, &num_headers, last_len);
	return {{method, method_len}, {path, path_len}, minor_version, num_headers, ret};
}

void test_request()
{
	struct phr_header headers[4];

	const char * buf = "GET /page.sas HTTP/1.1\r\n";

	auto ret = parse(buf, 17, headers, 0);
	fmt::print("Return: '{}'.\n",			ret.code);
	fmt::print("Method: '{}'.\n",			ret.method);
	fmt::print("Path: '{}'.\n",				ret.path);
	fmt::print("Minor version: '{}'.\n",	ret.minor_version);
	fmt::print("Headers count: '{}'.\n\n",	ret.num_headers);
}

int main() try
{
	test_request();
	return 0;
}
catch(std::exception & e) { fmt::print("Exception: '{}'.\n", e.what()); }
catch(...) { fmt::print("Exception: 'unknown'.\n"); }
