#pragma once

// boost_system
#include <boost/system/error_code.hpp>

// boost_asio
#include <boost/asio/error.hpp>

// boost_beast
#include <boost/beast/core/error.hpp>
#include <boost/beast/http/error.hpp>

namespace io::error
{
	constexpr bool is_common_timeout(const boost::system::error_code & ec)
	{
		if(ec == boost::beast::error::timeout) return true;
		if(ec == boost::asio::error::timed_out) return true;
		if(ec.category() == boost::asio::error::system_category && ec.value() == 121) return true;
		return false;
	};

	constexpr bool is_common_disconnect(const boost::system::error_code & ec)
	{
		if(ec == boost::asio::error::connection_aborted) return true;
		if(ec == boost::asio::error::connection_reset) return true;
		if(ec == boost::asio::error::connection_refused) return true;
		if(ec == boost::beast::http::error::end_of_stream) return true;
		return false;
	};
}
