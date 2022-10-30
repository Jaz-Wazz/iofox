#pragma once
#define BOOST_ASIO_HAS_CO_AWAIT
#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/asio/experimental/as_tuple.hpp>
#include <boost/beast.hpp>
#include <fmt/core.h>
#include <exception>
#include <variant>

namespace hf::detail::winapi::descriptors
{
	constexpr struct
	{
		operator HANDLE() const
		{
			return CreateFileA("CONIN$", GENERIC_WRITE | GENERIC_READ, 0, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
		}
	} cin;

	constexpr struct
	{
		operator HANDLE() const
		{
			return CreateFileA("CONOUT$", GENERIC_WRITE | GENERIC_READ, 0, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
		}
	} cout;
}

namespace hf::detail::winapi
{
	inline INPUT_RECORD console_take_event()
	{
		static DWORD null_out;
		INPUT_RECORD record;
		bool result = ReadConsoleInputA(descriptors::cin, &record, 1, &null_out);
		if(!result) throw std::runtime_error {"ReadConsoleInputA"};
		return record;
	}

	inline void console_set_cursor(short x, short y)
	{
		bool result = SetConsoleCursorPosition(descriptors::cout, {x, y});
		if(!result) throw std::runtime_error {"SetConsoleCursorPosition"};
	}

	inline void console_fill(char c, unsigned long length, short x, short y)
	{
		static unsigned long null_out;
		bool result = FillConsoleOutputCharacterA(descriptors::cout, c, length, {x, y}, &null_out);
		if(!result) throw std::runtime_error {"FillConsoleOutputCharacterA"};
	}

	inline short console_line_length()
	{
		CONSOLE_SCREEN_BUFFER_INFO buf_info;
		bool result = GetConsoleScreenBufferInfo(descriptors::cout, &buf_info);
		if(!result) throw std::runtime_error {"GetConsoleScreenBufferInfo"};
		return buf_info.dwMaximumWindowSize.X;
	}
}

namespace hf
{
	constexpr struct
	{
		void operator ()(std::exception_ptr e) const { if(e) std::rethrow_exception(e); }
	} rethrowed;

	struct console_event
	{
		// Constructor.
		constexpr explicit console_event(auto event): variant(event) {}

		// Events.
		struct key_up
		{
			constexpr key_up(char key): key(key) {}
			const char key;
		};

		struct key_down
		{
			constexpr key_down(char key): key(key) {}
			const char key;
		};

		struct resize
		{
			constexpr resize(short width, short height): width(width), height(height) {}
			const short width;
			const short height;
		};

		// Abstract as.
		template <typename T> constexpr std::optional<T> as_event()
		{
			if(auto x = std::get_if<T>(&variant)) return *x; else return {};
		}

		// Target as.
		constexpr std::optional<key_up> as_key_up_event() { return as_event<key_up>(); }
		constexpr std::optional<key_down> as_key_down_event() { return as_event<key_down>(); }
		constexpr std::optional<resize> as_resize_event() { return as_event<resize>(); }

		// Private.
		private: std::variant<key_up, key_down, resize> variant;
	};

	struct console_event_dispatcher
	{
		// Static class restrictions.
		console_event_dispatcher() = delete;
		~console_event_dispatcher() = delete;
		console_event_dispatcher & operator=(console_event_dispatcher &) = delete;

		// Async waiter.
		static boost::asio::awaitable<console_event> async_wait()
		{
			static boost::asio::windows::object_handle input {co_await boost::asio::this_coro::executor, detail::winapi::descriptors::cin};
			for(;;)
			{
				// Wait event.
				co_await input.async_wait(boost::asio::use_awaitable);
				INPUT_RECORD record = detail::winapi::console_take_event();

				// Union decomposition.
				auto && event_type		= record.EventType;
				auto && event_key		= record.Event.KeyEvent;
				auto && event_resize	= record.Event.WindowBufferSizeEvent;

				// Ignored: [Focus event, Menu event - depricated] [Mouse event - not supported].
				if(event_type == FOCUS_EVENT || event_type == MENU_EVENT || event_type == MOUSE_EVENT) continue;

				// Key event.
				if(event_type == KEY_EVENT)
				{
					if(event_key.bKeyDown == true)	co_return console_event::key_down {event_key.uChar.AsciiChar};
					if(event_key.bKeyDown == false)	co_return console_event::key_up {event_key.uChar.AsciiChar};
				}

				// Resize event.
				if(event_type == WINDOW_BUFFER_SIZE_EVENT)
				{
					co_return console_event::resize {event_resize.dwSize.X, event_resize.dwSize.Y};
				}
			}
		}
	};

	// struct terminal
	// {
	// 	// Constructor.
	// 	explicit terminal(boost::asio::io_context & ctx): console_event_dispatcher(ctx) {}

	// 	// Writer.
	// 	template <typename... T> void write(fmt::format_string<T...> format_string, T&&... args)
	// 	{
	// 		// Prepare new line.
	// 		std::string new_line = fmt::format(format_string, std::forward<decltype(args)>(args)...);

	// 		// Erase command from screen.
	// 		detail::winapi::console_fill('\0', command.length(), 0, y);
	// 		detail::winapi::console_set_cursor(0, y);

	// 		// Write new line, instead command.
	// 		fmt::print("{}", new_line);

	// 		// Return command to screen, after new line.
	// 		detail::winapi::console_set_cursor(0, ++y);
	// 		fmt::print("{}", command);

	// 		// Update "y".
	// 		y += new_line.length() / detail::winapi::console_line_length();
	// 	}

	// 	// Reader.
	// 	boost::asio::awaitable<std::string> async_read()
	// 	{
	// 		for(;;)
	// 		{
	// 			// Wait key press.
	// 			auto console_event = co_await console_event_dispatcher.async_wait();
	// 			if(auto event = console_event.as_key_down_event())
	// 			{
	// 				// If default key. (Not Return)
	// 				if(event->key != '\r')
	// 				{
	// 					// Print key.
	// 					fmt::print("{}", event->key);

	// 					// Add key to command.
	// 					command += event->key;
	// 				}
	// 				else
	// 				{
	// 					// Erase command from screen.
	// 					detail::winapi::console_fill('\0', command.length(), 0, y);
	// 					detail::winapi::console_set_cursor(0, y);

	// 					// Return command.
	// 					std::string ret = command;
	// 					command.clear();
	// 					co_return ret;
	// 				}
	// 			}
	// 		}
	// 	}

	// 	// Private.
	// 	private: console_event_dispatcher console_event_dispatcher;
	// 	private: std::string command;
	// 	private: short y = 0;
	// };

	// Perform foo.
	boost::asio::awaitable<void> https_async_perform(const char * host, auto & request, auto & response)
	{
		static boost::asio::ip::tcp::resolver resolver {co_await boost::asio::this_coro::executor};
		static boost::asio::ssl::context ctx_ssl {ctx_ssl.tlsv13_client};

		// Namespaces.
		namespace asio = boost::asio;
		namespace beast = boost::beast;

		// Dns resolve.
		auto resolver_results = co_await resolver.async_resolve(host, "https", asio::use_awaitable);

		// Connect.
		asio::ssl::stream<asio::ip::tcp::socket> stream {co_await boost::asio::this_coro::executor, ctx_ssl};
		co_await asio::async_connect(stream.lowest_layer(), resolver_results, asio::use_awaitable);

		// Handshake.
		SSL_set_tlsext_host_name(stream.native_handle(), host);
		co_await stream.async_handshake(stream.client, asio::use_awaitable);

		// Write.
		co_await beast::http::async_write(stream, request, asio::use_awaitable);

		// Read.
		beast::flat_buffer buffer;
		co_await beast::http::async_read(stream, buffer, response, asio::use_awaitable);

		// Disconnect.
		auto [error] = co_await stream.async_shutdown(asio::experimental::as_tuple(asio::use_awaitable));
		if(error != asio::ssl::error::stream_truncated) throw error;
	}
}
