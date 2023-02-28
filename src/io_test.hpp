#pragma once
#include <array>
#include <boost/static_string/static_string.hpp>
#include <cstddef>
#include <cstring>
#include <optional>
#include <string_view>
#include <picohttpparser.h>

#define pbl public:
#define prv private:

namespace io_test
{
	class parser
	{
		boost::static_strings::static_string<128>	buffer;
		prv const char *							method_data		= nullptr;
		prv std::size_t								method_size		= 0;
		prv const char *							path_data		= nullptr;
		prv std::size_t								path_size		= 0;
		prv int										minor_version	= -1;
		prv std::size_t								headers_size	= 0;
		prv phr_header								headers[4];

		pbl int push(std::string_view str)
		{
			buffer.append(str.begin(), str.end());
			return phr_parse_request
			(
				buffer.data(),
				buffer.size(),
				&method_data,
				&method_size,
				&path_data,
				&path_size,
				&minor_version,
				headers,
				&headers_size,
				0
			);
		}

		pbl auto method() -> std::optional<std::string_view>
		{
			if(method_size > 0) return {{method_data, method_size}}; else return {};
		}

		pbl auto path() -> std::optional<std::string_view>
		{
			if(path_size > 0) return {{path_data, path_size}}; else return {};
		}

		pbl auto version() -> std::optional<int>
		{
			if(minor_version > -1) return minor_version; else return {};
		}
	};
}

#undef pbl
#undef prv
