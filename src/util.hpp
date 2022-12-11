#pragma once
#include <string>

namespace util::json
{
	inline auto format(const auto & str) -> std::string
	{
		// Copy string.
		std::string x = str;

		// Iterate each char.
		for(int i = 0, tabs = 0, text = false; i <= x.length(); i++)
		{
			// Detect text blocks.
			if(x[i] == '"') text = !text;

			// Skip format in text block.
			if(text) continue;

			// Formating.
			if(x[i] == ',')
			{
				// Move content after ',' to new line and tabulate.
				x.insert(i + 1, "\n");
				x.insert(i + 2, tabs, '\t');

				// Move index.
				i += 1 + tabs;
			}
			if(x[i] == '{' || x[i] == '[')
			{
				// Increasure tabs.
				tabs++;

				// Move content after '{' to new line and tabulate.
				x.insert(i + 1, "\n");
				x.insert(i + 2, tabs, '\t');

				// Move index.
				i += 1 + tabs;
			}
			if(x[i] == '}' || x[i] == ']')
			{
				// Decreasure tabs.
				tabs--;

				// Move '}' to new line and tabulate.
				x.insert(i, "\n");
				x.insert(i + 1, tabs, '\t');

				// Move index.
				i += 1 + tabs;
			}
			if(x[i] == ':' && (x[i + 1] == '{' || x[i + 1] == '['))
			{
				// Move '{' to new line and tabulate.
				x.insert(i + 1, "\n");
				x.insert(i + 2, tabs, '\t');

				// Move index.
				i += 1 + tabs;
			}
			if(x[i] == ':' && (x[i + 1] != '{' || x[i + 1] != '['))
			{
				// Add space after ':'.
				x.insert(i + 1, " ");

				// Move index.
				i += 1;
			}
		}

		// Return string.
		return x;
	}
}
