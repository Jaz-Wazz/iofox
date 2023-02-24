#include <boost/asio/co_spawn.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/this_coro.hpp>
#include <fmt/core.h>
#include <iofox.hpp>
#include <iostream>
#include <string>
#include <vector>
#include <picohttpparser.h>

namespace asio = boost::asio;			// NOLINT.
namespace beast = boost::beast;			// NOLINT.
namespace http = beast::http;			// NOLINT.
namespace this_coro = asio::this_coro;	// NOLINT.

int main() try
{
	// Inputs.
	char buf[4096];
	size_t buflen = 4096, prevbuflen = 0;

	// Outputs headers.
	struct phr_header headers[100] {};
	size_t num_headers = 100;
	int minor_version = 0;

	// Outputs.
	const char *method, *path;
	size_t method_len = 0, path_len = 0;

	int pret = phr_parse_request(buf, buflen, &method, &method_len, &path, &path_len, &minor_version, headers, &num_headers, prevbuflen);
	fmt::print("Ret: '{}'.\n", pret);

	return 0;
}
catch(std::exception & e) { fmt::print("Exception: '{}'.\n", e.what()); }
catch(...) { fmt::print("Exception: 'unknown'.\n"); }
