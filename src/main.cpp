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

class parser
{
	public: std::string_view method;
	public: std::string_view path;
	public: int minor_version;
	public: std::size_t headers_count;
	public: phr_header headers[4];

	public: int parse(const char * buffer, std::size_t size, std::size_t last_len)
	{
		const char * method, * path;
		size_t method_len, path_len;
		int ret = phr_parse_request(buffer, size, &method, &method_len, &path, &path_len, &minor_version, headers, &headers_count, last_len);
		this->method = {method, method_len};
		this->path = {path, path_len};
		return ret;
	}
};

void test_request()
{
	parser parser;
	const char * buf = "GET /page.sas HTTP/1.1\r\n";

	auto ret = parser.parse(buf, 17, 0);
	fmt::print("Return: '{}'.\n",			ret);
	fmt::print("Method: '{}'.\n",			parser.method);
	fmt::print("Path: '{}'.\n",				parser.path);
	fmt::print("Minor version: '{}'.\n",	parser.minor_version);
	fmt::print("Headers count: '{}'.\n\n",	parser.headers_count);
}

int main() try
{
	test_request();
	return 0;
}
catch(std::exception & e) { fmt::print("Exception: '{}'.\n", e.what()); }
catch(...) { fmt::print("Exception: 'unknown'.\n"); }
