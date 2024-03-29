cmake_minimum_required(VERSION "3.27.4")
project("iofox")

file(GLOB_RECURSE src "src/*" "test/*")
add_executable(${PROJECT_NAME} ${src})
set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 23)
set_property(TARGET ${PROJECT_NAME} PROPERTY CMAKE_CXX_STANDARD_REQUIRED ON)
set_property(TARGET ${PROJECT_NAME} PROPERTY CMAKE_CXX_EXTENSIONS OFF)

find_package("Boost" REQUIRED "system" "url")
find_package("fmt" REQUIRED)
find_package("OpenSSL" REQUIRED)

Include("FetchContent")
add_compile_definitions("CATCH_CONFIG_NO_COLOUR_WIN32")
FetchContent_Declare("Catch2" GIT_REPOSITORY "https://github.com/catchorg/Catch2.git" GIT_TAG "v3.5.3")
FetchContent_MakeAvailable("Catch2")

target_include_directories(${PROJECT_NAME} PUBLIC "src")
target_link_libraries(${PROJECT_NAME} PRIVATE Boost::system Boost::url OpenSSL::SSL OpenSSL::Crypto fmt::fmt Catch2::Catch2WithMain)
target_compile_definitions(${PROJECT_NAME} PRIVATE BOOST_ASIO_NO_DEPRECATED BOOST_ASIO_HAS_CO_AWAIT)

if(WIN32)
	target_link_libraries(${PROJECT_NAME} PRIVATE ws2_32 wsock32)
	target_compile_definitions(${PROJECT_NAME} PRIVATE _WIN32_WINNT=0x0601)
endif()

include("Catch")
enable_testing()
catch_discover_tests(${PROJECT_NAME})
