include_guard()
include("config/cpm.cmake")

CPMAddPackage(
	NAME "Boost"
	VERSION "1.87.0"
	URL "https://github.com/boostorg/boost/releases/download/boost-1.87.0/boost-1.87.0-cmake.tar.xz"
	PATCHES "../patches/boost.patch"
	CUSTOM_CACHE_KEY "263a30dc8c4d78dc7c709175eea1838c88c766b9"
	OPTIONS
		"BOOST_ENABLE_CMAKE ON"
		"BOOST_SKIP_INSTALL_RULES ON"
		"BUILD_SHARED_LIBS OFF"
		"CMAKE_DISABLE_FIND_PACKAGE_ICU ON"
		"BOOST_INCLUDE_LIBRARIES asio\\\;beast\\\;url"
)

if(WIN32)
	target_link_libraries("boost_asio" INTERFACE ws2_32 wsock32)
	target_compile_definitions("boost_asio" INTERFACE _WIN32_WINNT=0x0601)
endif()

target_compile_definitions("boost_asio" INTERFACE
	BOOST_ASIO_NO_DEPRECATED
	BOOST_SYSTEM_USE_UTF8
)

file(GLOB_RECURSE boost_asio_headers
	"${Boost_SOURCE_DIR}/libs/asio/include/*.hpp"
	"${Boost_SOURCE_DIR}/libs/asio/include/*.ipp"
)

file(GLOB_RECURSE boost_beast_headers
	"${Boost_SOURCE_DIR}/libs/beast/include/*.hpp"
	"${Boost_SOURCE_DIR}/libs/beast/include/*.ipp"
)

target_sources("boost_asio" INTERFACE
	FILE_SET "HEADERS"
	BASE_DIRS "${Boost_SOURCE_DIR}/libs/asio/include"
	FILES "${boost_asio_headers}"
)

target_sources("boost_beast" INTERFACE
	FILE_SET "HEADERS"
	BASE_DIRS "${Boost_SOURCE_DIR}/libs/beast/include"
	FILES "${boost_beast_headers}"
)
