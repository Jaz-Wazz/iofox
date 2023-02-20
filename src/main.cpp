#include <boost/asio/buffer.hpp>
#include <boost/asio/co_spawn.hpp>
#include <boost/asio/connect.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/read.hpp>
#include <boost/asio/read_until.hpp>
#include <boost/asio/streambuf.hpp>
#include <boost/asio/this_coro.hpp>
#include <boost/asio/write.hpp>
#include <boost/buffers/const_buffer.hpp>
#include <boost/http_proto/request_parser.hpp>
#include <boost/http_proto/request.hpp>
#include <boost/http_proto/response.hpp>
#include <boost/http_proto/serializer.hpp>
#include <boost/http_proto/status.hpp>
#include <boost/http_proto/string_body.hpp>
#include <boost/http_proto/version.hpp>
#include <ctype.h>
#include <exception>
#include <fmt/core.h>
#include <iostream>
#include <string>
#include <vector>
#include <iofox.hpp>

namespace http_proto = boost::http_proto;	// NOLINT.
namespace asio = boost::asio;				// NOLINT.
namespace this_coro = asio::this_coro; 		// NOLINT.

void hexdump(const void * ptr, std::size_t size)
{
	const unsigned char * buf = static_cast<const unsigned char *>(ptr);
	for (int i = 0; i < size; i += 16)
	{
		fmt::print("{:06x}: ", i);
		for (int j = 0; j < 16; j++) if (i + j < size) fmt::print("{:02x} ", buf[i + j]); else fmt::print("   ");
		fmt::print(" ");
		for (int j = 0; j < 16; j++) if (i + j < size) fmt::print("{:c}", isprint(buf[i + j]) ? buf[i + j] : '.');
		fmt::print("\n");
	}
}

int main() try
{
	http_proto::response response
	{
		"HTTP/1.1 200 OK\r\n"
		"cache-control: private\r\n"
		"content-type: text/html; charset=utf-8\r\n"
		"strict-transport-security: max-age=15552000\r\n"
		"x-frame-options: SAMEORIGIN\r\n"
		"content-length: 8\r\n"
		"\r\n"
		"somebody"
	};

	hexdump(response.buffer().data(), response.buffer().size());
	return 0;
}
catch(const std::exception & e)
{
	fmt::print("Exception: '{}'.\n", e.what());
}
