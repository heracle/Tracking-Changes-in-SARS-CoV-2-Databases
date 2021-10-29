cmake_minimum_required(VERSION 3.11)

# set the project name
project(SequenceComparison)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_FLAGS "-std=c++17 -O3 -lcurl")

set(CURL_LIBRARY "-lcurl") 
find_package(CURL REQUIRED)

set(SOURCES src/main.cpp 
            src/cli/config.cpp
            src/common/logger.cpp)

include_directories(src/external_libraries)


# target_link_libraries(src/external_libraries/spdlog/fmt/ PRIVATE fmt::fmt-header-only)

# add the executable
add_executable(run ${SOURCES})


# http://derekmolloy.ie/hello-world-introductions-to-cmake/

# gcc --version 8.2.0
# cmake --version 3.11.0