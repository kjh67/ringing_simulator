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

enable_testing()


#
# TEST FILE SETUP
#

add_executable(
    ringing_simulator_tests
    ringing_utils/row.test.cc
)

target_link_libraries(
    ringing_utils
    file_utils
    simulator
    GTest::gtest_main
    ${LIBXML2_LIBRARIES}
)

include(GoogleTest)
gtest_discover_tests(ringing_simulator_tests)
