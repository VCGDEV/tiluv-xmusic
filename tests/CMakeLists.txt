# Uses the system GoogleTest when installed (Fedora: gtest-devel, Debian: libgtest-dev),
# otherwise downloads it.
find_package(GTest QUIET)
if(NOT GTest_FOUND)
    include(FetchContent)
    FetchContent_Declare(googletest
        URL https://github.com/google/googletest/archive/refs/tags/v1.17.0.tar.gz
        DOWNLOAD_EXTRACT_TIMESTAMP TRUE
    )
    set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(googletest)
endif()

include(GoogleTest)

file(GLOB TEST_SOURCES CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)
add_executable(tiluv-tests ${TEST_SOURCES})
target_link_libraries(tiluv-tests PRIVATE tiluv-core GTest::gtest_main)
target_compile_definitions(tiluv-tests PRIVATE TILUV_ASSET_DIR="${CMAKE_SOURCE_DIR}/assets")
tiluv_warnings(tiluv-tests)

gtest_discover_tests(tiluv-tests)
