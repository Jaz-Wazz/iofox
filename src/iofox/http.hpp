#pragma once

// boost_beast
#include <boost/beast/http/fields.hpp>
#include <boost/beast/http/message.hpp>

// stl
#include <initializer_list>
#include <sstream>
#include <string_view>
#include <utility>

// fmt
#include <fmt/core.h>

// iofox
#include <iofox/meta.hpp>

#define pbl public:
#define prv private:

namespace iofox::http
{
	using headers = std::initializer_list<std::pair<std::string_view, std::string_view>>;

	template <bool is_request, class body_type = void, class fields_type = boost::beast::http::fields>
	class message: public boost::beast::http::message<is_request, meta::transform_body<body_type>, fields_type>
	{
		pbl void debug_dump()
		{
			fmt::print("[http_message] - {}\n", (std::stringstream() << *this).str());
		}
	};

	template <class body_type = void, class fields_type = boost::beast::http::fields>
	class request: public message<true, body_type, fields_type>
	{
		pbl request(std::string_view method = "GET", std::string_view target = "/", headers headers = {})
		{
			this->method_string(method);
			this->target(target);
			for(auto && [header, value] : headers) this->insert(header, value);
		}
	};

	template <class body_type = void, class fields_type = boost::beast::http::fields>
	class response: public message<false, body_type, fields_type>
	{
		pbl response(unsigned int result = 200, headers headers = {})
		{
			this->result(result);
			for(auto && [header, value] : headers) this->insert(header, value);
		}
	};
}

#undef pbl
#undef prv
