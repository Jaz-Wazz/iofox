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
	// Basic response header object.
	class response_header: public beast::http::response_header<>
	{
		prv using base = beast::http::response_header<>;
		prv using header_list = std::initializer_list<std::pair<std::string, std::string>>;

		pbl using base::operator=;
		pbl using base::operator[];

		pbl response_header(unsigned int result = 200, header_list headers = {})
		{
			this->result(result);
			for(auto && [header, value] : headers) this->insert(header, value);
		}

		pbl response_header(beast::http::response_header<> && header): base(std::move(header)) {}
	};
}

#undef prv
#undef pbl
#undef beast
