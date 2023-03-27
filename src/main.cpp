#include <algorithm>
#include <fmt/core.h>
#include <string>
#include <ranges>
#include <string_view>

#define prv private:
#define pbl public:

namespace cc
{
	class block
	{
		pbl int width;
		pbl int height;
		pbl std::string content;

		pbl block(int width, int height, std::string content)
		: width(width), height(height), content(std::move(content)) {}

		pbl auto str() -> std::string
		{
			std::string ret;

			ret += "┌";
			for(int i = 0; i < width + 2; i++) ret += "─";
			ret += "┐\n";

			for(auto chunk : content | std::views::chunk(width))
			{
				for(auto sub_chunk : chunk | std::views::split('\n'))
				{
					ret += fmt::format("│ {:{}} │\n", std::string_view(sub_chunk), width);
				}

				// std::string_view line {c};
				// std::ranges::replace(c, '\n', "dd");

				// ret += fmt::format("│ {:{}} │\n", std::string_view(c), width);
			}

			return ret;
		}
	};
}

#undef prv
#undef pbl

int main() try
{
	cc::block block {10, 10, "con\ntentt q34u3ht qo3ht8q4oqtu h4oq87r q23hro8 7rgq2ir u"};
	fmt::print("{}\n", block.str());

	return 0;
}
catch(const std::exception & e)
{
	fmt::print("Exception: '{}'.\n", e.what());
}
