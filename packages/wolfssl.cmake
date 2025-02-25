include_guard()
include("config/cpm.cmake")

CPMAddPackage(
	GITHUB_REPOSITORY "wolfSSL/wolfssl"
	VERSION "5.7.6"
	GIT_TAG "v5.7.6-stable"
	OPTIONS
		"BUILD_SHARED_LIBS OFF"
		"WOLFSSL_EXAMPLES OFF"
		"WOLFSSL_CRYPT_TESTS OFF"
		"WOLFSSL_ASIO ON"
		"WOLFSSL_DEBUG OFF"
)

target_include_directories("wolfssl" PUBLIC "${wolfssl_SOURCE_DIR}/wolfssl")
target_compile_definitions("wolfssl" PUBLIC HAVE_EMPTY_AGGREGATES=0 WOLFSSL_NO_ASN_STRICT)
