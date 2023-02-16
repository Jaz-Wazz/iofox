#include <boost/http_proto/request_parser.hpp>
#include <boost/http_proto/request.hpp>
#include <boost/http_proto/response.hpp>
#include <boost/http_proto/serializer.hpp>
#include <boost/http_proto/status.hpp>
#include <exception>
#include <fmt/core.h>
#include <iostream>
#include <string>

namespace http_proto = boost::http_proto; // NOLINT.

int main() try
{
	http_proto::response response;
	response.set_start_line(http_proto::status::ok);
	response.set("Tails", "9");
	response.set("Paws", "4");

	http_proto::serializer serializer;
	// serializer.start_stream(response);
	// serializer.consume(10);
	serializer.start(response);

	// serializer.consume(5);

	for(auto buffer : serializer.prepare().value())
	{
		fmt::print("---- DATA BLOCK ----\n{}\n", std::string(static_cast<const char *>(buffer.data()), buffer.size()));
	}

	// std::string request =
	// "GET /path/to/sas.html HTTP/1.1\r\n"
	// "Host: develop.http-proto.cpp.al\r\n"
	// "User-Agent: sas\r\n"
	// "\r\n";

	// http_proto::request_parser parser;



	fmt::print("sas.\n");
	return 0;
}
catch(const std::exception & e)
{
	fmt::print("Exception: '{}'.\n", e.what());
}
