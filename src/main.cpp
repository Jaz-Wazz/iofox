#include <boost/buffers/const_buffer.hpp>
#include <boost/http_proto/request_parser.hpp>
#include <boost/http_proto/request.hpp>
#include <boost/http_proto/response.hpp>
#include <boost/http_proto/serializer.hpp>
#include <boost/http_proto/status.hpp>
#include <boost/http_proto/string_body.hpp>
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
	std::string str = "sassisaininya";
	http_proto::string_body sb {str};
	serializer.start(response, sb);

	while(!serializer.is_done())
	{
		auto it = serializer.prepare().value().begin();
		auto size = (it->size() < 10) ? it->size() : 10;
		fmt::print("---- DATA BLOCK ----\n{}\n", std::string(static_cast<const char *>(it->data()), size));
		serializer.consume(size);
	}

	return 0;
}
catch(const std::exception & e)
{
	fmt::print("Exception: '{}'.\n", e.what());
}
