#pragma once

// windows
#include <winnls.h>
#include <winnt.h>

namespace iofox::windows
{
	enum class lang: LANGID
	{
		english = MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US)
	};

	constexpr void set_asio_locale(iofox::windows::lang code)
	{
		SetThreadUILanguage(static_cast<LANGID>(code));
	}
}
