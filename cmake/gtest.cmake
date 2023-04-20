include(FetchContent)
FetchContent_Declare(
	googletest
	GIT_REPOSITORY https://gitee.com/mirrors/googletest.git
	GIT_TAG v1.12.0
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)
