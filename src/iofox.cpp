#include <iofox/send.hpp>

template auto io::http::send<std::string>
(
	const std::string_view url,
	const io::http::request<void> & request,
	const std::chrono::steady_clock::duration timeout
)
-> io::coro<io::http::response<std::string>>;
