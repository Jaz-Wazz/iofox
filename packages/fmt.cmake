include_guard()
include("config/cpm.cmake")

CPMAddPackage(
	GITHUB_REPOSITORY "fmtlib/fmt"
	VERSION "11.1.3"
	GIT_TAG "11.1.3"
)

set_property(TARGET "fmt" PROPERTY EXPORT_COMPILE_COMMANDS OFF)
