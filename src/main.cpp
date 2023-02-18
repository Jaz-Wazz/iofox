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
#include <vector>

namespace http_proto = boost::http_proto; // NOLINT.

auto serialize(http_proto::response response) -> std::string
{
	std::string ret;
	http_proto::serializer serializer;
	serializer.start(response);
	for(auto buffer : serializer.prepare().value()) ret.append(static_cast<const char *>(buffer.data()), buffer.size());
	return ret;
}

int main() try
{
	http_proto::response response;
	response.set_start_line(http_proto::status::ok);
	response.set("Tails", "9");
	response.set("Paws", "4");

	fmt::print("Response serialized:\n{}\n", serialize(response));
	fmt::print("Response buffer:\n{}\n", response.buffer());
	return 0;
}
catch(const std::exception & e)
{
	fmt::print("Exception: '{}'.\n", e.what());
}
