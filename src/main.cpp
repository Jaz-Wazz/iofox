#include <fmt/core.h>
#include <ctype.h>
#include <string>

void hexdump(void * ptr, std::size_t size)
{
	unsigned char * buf = static_cast<unsigned char *>(ptr);
	for (int i = 0; i < size; i += 16)
	{
		fmt::print("{:06x}: ", i);
		for (int j = 0; j < 16; j++) if (i + j < size) fmt::print("{:02x} ", buf[i + j]); else fmt::print("   ");
		fmt::print(" ");
		for (int j = 0; j < 16; j++) if (i + j < size) fmt::print("{:c}", isprint(buf[i + j]) ? buf[i + j] : '.');
		fmt::print("\n");
	}
}

int main() try
{
	std::string str = "fwewegwiohegwioefwaefh2t2h3or2u3ty92q3yh2q93uh2q3irg";
	hexdump(str.data(), str.size());
	return 0;
}
catch(const std::exception & e)
{
	fmt::print("Exception: '{}'.\n", e.what());
}
