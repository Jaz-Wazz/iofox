#pragma once

#include <boost/beast/http/message.hpp>
#include <initializer_list>
#include <utility>
#include <string>

#define prv private:
#define pbl public:
#define beast boost::beast

namespace io::http
{
	// Basic request header object.
	class request_header: public beast::http::request_header<>
	{
		prv using base = beast::http::request_header<>;
		prv using header_list = std::initializer_list<std::pair<std::string, std::string>>;

		pbl using base::operator=;
		pbl using base::operator[];

		pbl request_header(std::string method = "GET", std::string target = "/", header_list headers = {})
		{
			this->method_string(method);
			this->target(target);
			for(auto && [header, value] : headers) this->insert(header, value);
		}

		pbl request_header(beast::http::request_header<> && header): base(std::move(header)) {}
	};
}

#undef prv
#undef pbl
#undef beast
