#pragma once

// boost_beast
#include <boost/beast/http/empty_body.hpp>
#include <boost/beast/http/string_body.hpp>
#include <boost/beast/http/vector_body.hpp>

// stl
#include <string>
#include <vector>

namespace iofox::meta
{
	template <class T>		struct transform_body { using type = T; };
	template <>				struct transform_body<void> { using type = boost::beast::http::empty_body; };
	template <class... T>	struct transform_body<std::basic_string<T...>> { using type = boost::beast::http::basic_string_body<T...>; };
	template <class... T>	struct transform_body<std::vector<T...>> { using type = boost::beast::http::vector_body<T...>; };
	template <class T> using transform_body_v = transform_body<T>::type;
}
