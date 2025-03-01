#pragma once

// boost_beast
#include <boost/beast/http/message.hpp>
#include <boost/beast/http/string_body.hpp>

// stl
#include <initializer_list>
#include <string_view>
#include <utility>

namespace http
{
	using headers = std::initializer_list<std::pair<std::string_view, std::string_view>>;

	struct request_header: boost::beast::http::request_header<>
	{
		using boost::beast::http::request_header<>::request_header;

		request_header(std::string_view method = "GET", std::string_view path = "/", http::headers headers = {})
		{
			method_string(method), target(path);
			for(auto && [header, value] : headers) insert(header, value);
		}
	};

	struct response_header: boost::beast::http::response_header<>
	{
		using boost::beast::http::response_header<>::response_header;

		response_header(unsigned int code = 200, http::headers headers = {})
		{
			result(code);
			for(auto && [header, value] : headers) insert(header, value);
		}
	};

	struct request: boost::beast::http::request<boost::beast::http::string_body>
	{
		using boost::beast::http::request<boost::beast::http::string_body>::request;

		request(std::string_view method = "GET", std::string_view path = "/", http::headers headers = {})
		{
			method_string(method), target(path);
			for(auto && [header, value] : headers) insert(header, value);
		}
	};

	struct response: boost::beast::http::response<boost::beast::http::string_body>
	{
		using boost::beast::http::response<boost::beast::http::string_body>::response;

		response(unsigned int code = 200, http::headers headers = {})
		{
			result(code);
			for(auto && [header, value] : headers) insert(header, value);
		}
	};
}
