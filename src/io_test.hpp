#pragma once
#include <array>
#include <boost/static_string/static_string.hpp>
#include <cstddef>
#include <cstring>
#include <fmt/core.h>
#include <optional>
#include <stdexcept>
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
		prv std::size_t								headers_size	= 4;
		prv phr_header								headers[4]		= {};

		pbl void push(std::string_view str)
		{
			buffer.append(str.begin(), str.end());
			headers_size = sizeof(headers) / sizeof(headers[0]);
			int ret = phr_parse_request
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
			if(ret == -1) throw std::runtime_error("bad_parse");
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

		pbl void print_headers()
		{
			for(auto header : headers)
			{
				if(header.name != nullptr && header.value != nullptr)
				{
					fmt::print
					(
						"Header: '{}' -> '{}'.\n",
						std::string_view(header.name, header.name_len),
						std::string_view(header.value, header.value_len)
					);
				}
			}
		}
	};
}

#undef pbl
#undef prv
