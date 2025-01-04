#
# GOOGLETEST SETUP
#

# Min C++ Standard requirement for GoogleTest
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)
FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
)

FetchContent_MakeAvailable(googletest)



#
# TEST FILE SETUP
#

add_executable(
    liris_tests
    ringing_utils/row.test.cc
    file_utils/method_lib.test.cc
)

target_link_libraries(
    liris_tests
    ringing_utils
    file_utils
    GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(liris_tests)
