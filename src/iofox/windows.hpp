#pragma once

// windows
#include <winnls.h>
#include <winnt.h>

namespace io::windows
{
	enum class lang: LANGID
	{
		english = MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US)
	};

	constexpr void set_asio_locale(io::windows::lang code)
	{
		SetThreadUILanguage(static_cast<LANGID>(code));
	}
}
