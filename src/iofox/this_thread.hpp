#pragma once

// stl
#include <stdexcept>
#include <string_view>

// windows
#ifdef _WIN32
#include <winnls.h>
#include <winnt.h>
#endif

namespace iofox::this_thread
{
	inline void set_language(std::string_view code)
	{
		if(code != "en_us") throw std::invalid_argument("unsupported_language_code");
		#ifdef _WIN32
		SetThreadUILanguage(MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US));
		#endif
	}
}
