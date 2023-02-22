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

void test_inherit_parser()
{
	http_proto::response response {data};
	hexdump(response.buffer().data(), response.buffer().size());
};

void test_inherit_parser_base()
{
	// http_proto::message_base response {http_proto::detail::kind::response, data};
	// hexdump(response.buffer().data(), response.buffer().size());
};

void some_tests()
{
	http_proto::response_parser parser;
	parser.start();

	for(auto buffer : parser.prepare())
	{
		fmt::print("[write buffer] - data: '{}', size: '{}'.\n", buffer.data(), buffer.size());
		if(buffer.data() != nullptr && buffer.size() != 0)
		{
			for(int i = 0; i < data.size(); i++) static_cast<char *>(buffer.data())[i] = data[i];
			parser.commit(data.size());
		}
	}

	http_proto::error_code ec;
	parser.parse(ec);
	fmt::print("Parse done: '{}', Is complited: '{}', Got header: '{}'.\n", ec.message(), parser.is_complete(), parser.got_header());
	if(!ec) fmt::print("Buffer: \n{}\n", parser.get().buffer());
	if(!ec) fmt::print("Body: \n{}\n", parser.body());

	for(auto buffer : parser.prepare())
	{
		std::string data = "1231231234243";
		fmt::print("[write buffer] - data: '{}', size: '{}'.\n", buffer.data(), buffer.size());
		if(buffer.data() != nullptr && buffer.size() != 0)
		{
			for(int i = 0; i < data.size(); i++) static_cast<char *>(buffer.data())[i] = data[i];
			parser.commit(data.size());
		}
	}

	parser.parse(ec);
	fmt::print("Parse done: '{}', Is complited: '{}', Got header: '{}'.\n", ec.message(), parser.is_complete(), parser.got_header());
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
