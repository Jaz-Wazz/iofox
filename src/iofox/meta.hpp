#pragma once

// boost_beast
#include <boost/beast/http/empty_body.hpp>
#include <boost/beast/http/string_body.hpp>
#include <boost/beast/http/vector_body.hpp>

// stl
#include <type_traits>
#include <vector>

namespace iofox::meta
{
	template <typename T>		struct is_vector					: std::false_type {};
	template <typename... T>	struct is_vector<std::vector<T...>>	: std::true_type {};
	template <typename T>		concept vector_one_byte = is_vector<T>::value && sizeof(typename T::value_type) == 1;

	template <typename>				struct make_body_type_impl				{ using type = void;													};
	template <>						struct make_body_type_impl<void>		{ using type = boost::beast::http::empty_body;							};
	template <>						struct make_body_type_impl<std::string>	{ using type = boost::beast::http::string_body;							};
	template <vector_one_byte T>	struct make_body_type_impl<T>			{ using type = boost::beast::http::vector_body<typename T::value_type>;	};
	template <typename T> using make_body_type = typename make_body_type_impl<std::remove_reference_t<T>>::type;
}
