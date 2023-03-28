#include <boost/beast/http/fields.hpp>
#include <boost/beast/http/parser.hpp>
#include <boost/beast/http/type_traits.hpp>
#include <fmt/core.h>
#include <iostream>
#include <stdexcept>
#include <boost/beast/http/empty_body.hpp>
#include <boost/beast/http/message.hpp>

namespace asio = boost::asio;			// NOLINT.
namespace beast = boost::beast;			// NOLINT.

class my_fields
{
public:

	// struct FieldsWriter
	// {
	// 	// The type of buffers returned by `get`
	// 	struct const_buffers_type;

	// 	// Constructor for requests
	// 	FieldsWriter(Fields const& f, unsigned version, verb method);

	// 	// Constructor for responses
	// 	FieldsWriter(Fields const& f, unsigned version, unsigned status);

	// 	// Returns the serialized header buffers
	// 	const_buffers_type get();
	// };

    struct writer;

	struct value_type;

protected:
    beast::string_view get_method_impl() const;
    beast::string_view get_target_impl() const;
    beast::string_view get_reason_impl() const;
    bool get_chunked_impl() const;
    bool get_keep_alive_impl(unsigned version) const;
    bool has_content_length_impl() const;
    void set_method_impl(beast::string_view s);
    void set_target_impl(beast::string_view s);
    void set_reason_impl(beast::string_view s);
    void set_chunked_impl(bool value);
    void set_content_length_impl(boost::optional<std::uint64_t>);
    void set_keep_alive_impl(unsigned version, bool keep_alive);
};

class x_fields: public beast::http::fields
{
	public: using beast::http::fields::value_type;
	public: using beast::http::fields::allocator_type;
	public: using beast::http::fields::const_iterator;
	public: using beast::http::fields::iterator;
	public: using beast::http::fields::writer;
	public: using beast::http::fields::basic_fields;
	public: using off_t = std::uint16_t;

	using beast::http::fields::basic_fields::
};

int main() try
{
	// beast::http::fields fields;

	// bool x = beast::http::is_fields<beast::http::fields>::value;
	// bool y = beast::http::is_fields<my_fields>::value;

	// beast::http::request<beast::http::empty_body, beast::http::fields> request_x;

	// beast::http::fields::value_type x{}

	// beast::http::parser<1, beast::http::empty_body, beast::http::fields> parser_x;
	beast::http::parser<1, beast::http::empty_body, x_fields> parser_y;

	// beast::http::request<beast::http::empty_body, my_fields> request_y;

	return 0;
}
catch(const std::exception & e)
{
	fmt::print("Exception: '{}'.\n", e.what());
}
