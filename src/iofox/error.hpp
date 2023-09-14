#pragma once

#include <boost/system/error_code.hpp>
#include <boost/beast/core/error.hpp>
#include <boost/beast/http/error.hpp>
#include <boost/asio/error.hpp>

#define beast boost::beast
#define asio boost::asio

namespace io::error
{
	inline auto is_common_timeout(const boost::system::error_code & error_code)
	{
		// Check "The socket was closed due to a timeout".
		if(error_code == beast::error::timeout) return true;

		// Check "A connection attempt failed because the connected party did not properly respond after a period of time,
		// or established connection failed because connected host has failed to respond".
		if(error_code == asio::error::timed_out) return true;

		// Check "The semaphore timeout period has expired".
		if(error_code.category() == asio::error::system_category && error_code.value() == 121) return true;

		// Other errors not timeout.
		return false;
	};

	inline auto is_common_disconnect(const boost::system::error_code & error_code)
	{
		// Check "An established connection was aborted by the software in your host machine".
		if(error_code == asio::error::connection_aborted) return true;

		// Check "An existing connection was forcibly closed by the remote host".
		if(error_code == asio::error::connection_reset) return true;

		// Check "No connection could be made because the target machine actively refused it".
		if(error_code == asio::error::connection_refused) return true;

		// Check "End of stream".
		if(error_code == beast::http::error::end_of_stream) return true;

		// Other errors not disconnect.
		return false;
	};
}

#undef beast
#undef asio
