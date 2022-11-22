#include <boost/outcome.hpp>
#include <boost/outcome/result.hpp>
#include <fmt/core.h>
#include <system_error>

namespace outcome = BOOST_OUTCOME_V2_NAMESPACE; // NOLINT

enum class err { success, error_a, error_b, error_c };
template <> struct std::is_error_code_enum<err> : true_type {};

class err_category: public std::error_category
{
	public: virtual const char * name() const noexcept override final { return "err"; }

	public: virtual std::string message(int c) const override final
	{
		switch (static_cast<err>(c))
		{
			case err::success: return "success";
			case err::error_a: return "error type a";
			case err::error_b: return "error type b";
			case err::error_c: return "error type c";
			default: return "unknown";
		}
	}
};

inline std::error_code make_error_code(err e)
{
	return {static_cast<int>(e), err_category()};
}

outcome::result<std::string, std::error_code> foo()
{
	// return "string";
	// return std::errc::address_family_not_supported;
	return err::error_a;
}

int main()
{
	if(auto x = foo())
	{
		fmt::print("Success: '{}'.\n", x.value());
	}
	else
	{
		fmt::print("Error: '{}'.\n", x.error().message());
	}

	return 0;
}
