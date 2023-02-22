#include <array>
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
#include <boost/buffers/buffer.hpp>
#include <boost/buffers/buffered_base.hpp>
#include <boost/buffers/circular_buffer.hpp>
#include <boost/buffers/const_buffer.hpp>
#include <boost/buffers/flat_buffer.hpp>
#include <boost/buffers/mutable_buffer.hpp>
#include <boost/buffers/sink.hpp>
#include <boost/http_proto/detail/header.hpp>
#include <boost/http_proto/error_types.hpp>
#include <boost/http_proto/message_base.hpp>
#include <boost/http_proto/parser.hpp>
#include <boost/http_proto/request_parser.hpp>
#include <boost/http_proto/request.hpp>
#include <boost/http_proto/response.hpp>
#include <boost/http_proto/response_parser.hpp>
#include <boost/http_proto/serializer.hpp>
#include <boost/http_proto/status.hpp>
#include <boost/http_proto/string_body.hpp>
#include <boost/http_proto/version.hpp>
#include <cstring>
#include <ctype.h>
#include <exception>
#include <fmt/core.h>
#include <fmt/format.h>
#include <iostream>
#include <stdexcept>
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

#include <boost/buffers.hpp>
#include <boost/http_proto.hpp>

std::string data
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

void consume_to_parser(http_proto::response_parser & parser, std::string message)
{
	if(auto buffer = *parser.prepare().begin(); buffer.data() != nullptr && buffer.end() != 0)
	{
		std::memcpy(buffer.data(), message.data(), message.size());
		parser.commit(message.size());
		fmt::print("[consumer] - write to buffer [ptr: {}, size: {}] -> data [size: {}].\n", buffer.data(), buffer.size(), message.size());
	}
}

struct my_sink: boost::buffers::sink
{
	boost::buffers::sink::results on_write(boost::buffers::const_buffer b, bool more)
	{
		fmt::print("sink.\n");
		return {};
	};
};

void some_tests()
{
	http_proto::response_parser parser;
	parser.start();

	// boost::buffers::is_sink<boost::buffers::buffered_base>::value;
	my_sink s;
	parser.set_body(s);

	consume_to_parser(parser, "HTTP/1.1 200 OK\r\n");
	consume_to_parser(parser, "cache-control: private\r\n");
	consume_to_parser(parser, "content-type: text/html\r\n");
	consume_to_parser(parser, "Content-Length: 8\r\n");
	consume_to_parser(parser, "\r\n");
	consume_to_parser(parser, "1234");
	consume_to_parser(parser, "5678");

	http_proto::error_code ec;
	parser.parse(ec);
	fmt::print("[parser] - parse, status: '{}', complited: '{}', got header: '{}'.\n", ec.message(), parser.is_complete(), parser.got_header());
	if(!ec) fmt::print("Buffer: \n{}\n", parser.get().buffer());
	if(!ec) fmt::print("Body: \n{}\n", parser.body());

	parser.parse(ec);
	fmt::print("[parser] - parse, status: '{}', complited: '{}', got header: '{}'.\n", ec.message(), parser.is_complete(), parser.got_header());
	if(!ec) fmt::print("Buffer: \n{}\n", parser.get().buffer());
	if(!ec) fmt::print("Body: \n{}\n", parser.body());
}

int main() try
{
	io::windows::set_asio_locale(io::windows::lang::english);
	some_tests();
	return 0;
}
catch(const std::exception & e)
{
	fmt::print("Exception: '{}'.\n", e.what());
}
