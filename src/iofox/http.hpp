#pragma once

// boost_beast
#include <boost/beast/http/message.hpp>

// stl
#include <initializer_list>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>

// fmt
#include <fmt/core.h>

// iofox
#include <iofox/meta.hpp>

#define pbl public:
#define prv private:

namespace iofox::http
{
	template <typename T = void> class request: public boost::beast::http::request<iofox::meta::make_body_type<T>>
	{
		prv using header_list = std::initializer_list<std::pair<std::string, std::string>>;

		pbl using boost::beast::http::request<meta::make_body_type<T>>::operator=;
		pbl using boost::beast::http::request<meta::make_body_type<T>>::operator[];

		pbl request(std::string method = "GET", std::string target = "/", header_list headers = {})
		{
			this->method_string(method);
			this->target(target);
			for(auto && [header, value] : headers) this->insert(header, value);
		}

		pbl void debug_dump()
		{
			fmt::print("[request] - {}\n", (std::stringstream() << *this).str());
		}
	};

	template <typename T = void> class response: public boost::beast::http::response<meta::make_body_type<T>>
	{
		prv using header_list = std::initializer_list<std::pair<std::string, std::string>>;
		pbl using boost::beast::http::response<meta::make_body_type<T>>::operator=;
		pbl using boost::beast::http::response<meta::make_body_type<T>>::operator[];

		pbl response(unsigned int result = 200, header_list headers = {})
		{
			this->result(result);
			for(auto && [header, value] : headers) this->insert(header, value);
		}

		pbl void check_code(int expected_code)
		{
			using base = boost::beast::http::response<meta::make_body_type<T>>;
			if(base::result_int() != expected_code) throw std::runtime_error("unexpected_code");
		}

		pbl void check_header(const std::string_view header)
		{
			using base = boost::beast::http::response<meta::make_body_type<T>>;
			if(base::operator[](header).empty()) throw std::runtime_error("unexpected_header");
		}

		pbl void check_contains_body()
		{
			using base = boost::beast::http::response<meta::make_body_type<T>>;
			if(base::chunked() && !base::body().empty()) return;
			if(base::has_content_length() && base::operator[]("Content-Length") != "0" && !base::body().empty()) return;
			throw std::runtime_error("unexpected_empty_body");
		}

		pbl void check_not_contains_body()
		{
			using base = boost::beast::http::response<meta::make_body_type<T>>;
			if(base::operator[]("Content-Length") != "0" || !base::body().empty())
			{
				throw std::runtime_error("unexpected_body");
			}
		}

		pbl void check_content_type(std::string_view expected_type)
		{
			using base = boost::beast::http::response<meta::make_body_type<T>>;
			if(base::at("Content-Type") != expected_type) throw std::runtime_error("unexpected_content_type");
		}

		pbl void debug_dump()
		{
			fmt::print("[response] - {}\n", (std::stringstream() << *this).str());
		}
	};
}

#undef pbl
#undef prv
