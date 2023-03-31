#include "iofox/iofox.hpp"
#include <exception>
#include <fmt/core.h>
#include <iosfwd>
#include <iostream>
#include <spanstream>
#include <string>

int main() try
{
	char buffer[128] {};
	std::spanstream stream {buffer};

	auto span_buf = stream.rdbuf();

	for(std::string cmd; std::getline(std::cin, cmd);)
	{
		if(cmd == "print")
		{
			io::log::print_hex_dump(buffer, sizeof(buffer));
		}
		if(cmd == "write")
		{
			stream << 11 << 22 << 33;
			stream.flush();
		}
		if(cmd == "read")
		{
			int i;
			stream >> i;
			fmt::print("readed: '{}'.\n", i);
		}
	}

	return 0;
}
catch(const std::exception & e)
{
	fmt::print("Exception: '{}'.\n", e.what());
}
