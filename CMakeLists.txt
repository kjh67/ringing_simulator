cmake_minimum_required(VERSION 3.14)
project(
    Liris
    VERSION 0.1.0
    LANGUAGES CXX
)

# Include LibXml2, for XML file parsing (needed for method library)
find_package(LibXml2 REQUIRED)
include_directories(include ${LIBXML2_INCLUDE_DIR})

# Ringing Utils setup
add_library(
    ringing_utils 
    src/ringing_utils/comp.cpp
    src/ringing_utils/method.cpp
    src/ringing_utils/row.cpp
    src/ringing_utils/touch.cpp
)

# File utils setup
add_library(
    file_utils
    src/file_utils/mcf_file.cpp
)
target_link_libraries(file_utils ringing_utils ${LIBXML2_LIBRARIES})

# Simulator setup
add_library(
    simulator
)
target_link_libraries(simulator ringing_utils file_utils ${LIBXML2_LIBRARIES})0



# ADD OPTION FOR WHICH INTERFACE TO BUILD (one for each interface that exists), WHETHER TO enable testing

# Executable setup
option(build-win "Flag to build executable application for Windows")
option(build-linux "Flag to build executable application for Linux")


add_executable(liris)
target_link_libraries(liris ringing_utils file_utils simulator ${LIBXML2_LIBRARIES})

add_subdirectory(tests)
