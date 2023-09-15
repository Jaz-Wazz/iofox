#pragma once

#include <boost/beast/http/message.hpp>
#include <fmt/core.h>
#include <iofox/core.hpp>
#include <initializer_list>
#include <string_view>
#include <type_traits>
#include <stdexcept>
#include <sstream>
#include <utility>
#include <string>

#define prv private:
#define pbl public:
#define beast boost::beast

namespace io::http
{
	// Basic response object.
	template <typename T = void> class response: public beast::http::response<meta::make_body_type<T>>
	{
		prv using header_list = std::initializer_list<std::pair<std::string, std::string>>;
		pbl using beast::http::response<meta::make_body_type<T>>::operator=;
		pbl using beast::http::response<meta::make_body_type<T>>::operator[];

		pbl response(unsigned int result = 200, header_list headers = {})
		{
			this->result(result);
			for(auto && [header, value] : headers) this->insert(header, value);
		}

		pbl template <typename X = T, typename std::enable_if<std::is_same_v<X, std::string>, int>::type = 0>
		response(unsigned int result, header_list headers, const std::string & body): response(result, std::move(headers))
		{ this->body() = body; }

		pbl template <typename X = T, typename std::enable_if<std::is_same_v<X, std::string>, int>::type = 0>
		response(unsigned int result, header_list headers, std::string && body): response(result, std::move(headers))
		{ this->body() = std::move(body); }

		pbl template <typename X = T, typename std::enable_if<meta::vector_one_byte<X>, int>::type = 0>
		response(unsigned int result, header_list headers, const std::vector<typename X::value_type> & body): response(result, std::move(headers))
		{ this->body() = body; }

		pbl template <typename X = T, typename std::enable_if<meta::vector_one_byte<X>, int>::type = 0>
		response(unsigned int result, header_list headers, std::vector<typename X::value_type> && body): response(result, std::move(headers))
		{ this->body() = std::move(body); }

		pbl void check_code(int expected_code)
		{
			using base = beast::http::response<meta::make_body_type<T>>;
			if(base::result_int() != expected_code) throw std::runtime_error("unexpected_code");
		}

		pbl void check_header(const std::string_view header)
		{
			using base = beast::http::response<meta::make_body_type<T>>;
			if(base::operator[](header).empty()) throw std::runtime_error("unexpected_header");
		}

		pbl void check_contains_body()
		{
			using base = beast::http::response<meta::make_body_type<T>>;

			if(base::chunked() && !base::body().empty())
			{
				return;
			}

			if(base::has_content_length() && base::operator[]("Content-Length") != "0" && !base::body().empty())
			{
				return;
			}

			throw std::runtime_error("unexpected_empty_body");
		}

		pbl void check_not_contains_body()
		{
			using base = beast::http::response<meta::make_body_type<T>>;
			if(base::operator[]("Content-Length") != "0" || !base::body().empty())
			{
				throw std::runtime_error("unexpected_body");
			}
		}

		pbl void check_content_type(const std::string_view expected_type)
		{
			using base = beast::http::response<meta::make_body_type<T>>;
			if(base::at("Content-Type") != expected_type) throw std::runtime_error("unexpected_content_type");
		}

		pbl void debug_dump()
		{
			fmt::print("[response] - {}\n", (std::stringstream() << *this).str());
		}
	};
}

#undef prv
#undef pbl
#undef beast
