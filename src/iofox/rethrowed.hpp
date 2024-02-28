#pragma once

// stl
#include <exception>

namespace io
{
	constexpr struct
	{
		void operator()(std::exception_ptr exception) const
		{
			if(exception) std::rethrow_exception(exception);
		}
	} rethrowed;
}
