#include <iofox/send.hpp>

template auto iofox::http::send<std::string>
(
	const std::string_view url,
	const iofox::http::request<void> & request,
	const std::chrono::steady_clock::duration timeout
)
-> iofox::coro<iofox::http::response<std::string>>;
