cmake_minimum_required(VERSION 3.11)

# Pieces of code taken from:
# http://derekmolloy.ie/hello-world-introductions-to-cmake/
# https://github.com/ratschlab/metagraph/blob/master/metagraph/CMakeLists.txt

# set the project name
project(SequenceComparison)

set(CMAKE_CXX_STANDARD 17 )
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
# -Wall -Wextra print all warning at compile time. 
# TODO move back to O3
set(CMAKE_CXX_FLAGS "-std=c++17 -O0 -lcurl -Wall -Wextra -lhdf5 -lhdf5_cpp")

set(CURL_LIBRARY "-lcurl") 
find_package(CURL REQUIRED)

# todo replace with a GLOB (similar to the way it is done for the unit tests)
set(SOURCES 
            src/ds/ps_treap.cpp
            src/ds/static_types/static_base.cpp
            src/ds/static_types/static_acc_id.cpp
            src/ds/static_types/static_location.cpp

            src/ds/tnode_types/tnode_base.cpp
            src/ds/tnode_types/tnode_location.cpp
            src/ds/tnode_types/tnode_acc_id.cpp

            src/ds/database.cpp
            src/ds/ctc.cpp

            src/cli/config.cpp
            src/cli/create.cpp
            src/cli/append.cpp
            src/cli/stats.cpp
            src/cli/query.cpp

            src/queries/freq_bp.cpp

            src/common/json_helper.cpp
            src/common/h5_helper.cpp
            src/common/logger.cpp
            src/common/constants.cpp
            src/main.cpp)


include_directories(src/external_libraries)

# include rapid_json
include_directories(src/external_libraries/rapid_json/include/rapidjson/)

#------------------
# Print all variables
#------------------

# macro(print_all_variables)
#     message(STATUS "print_all_variables------------------------------------------{")
#     get_cmake_property(_variableNames VARIABLES)
#     foreach (_variableName ${_variableNames})
#         message(STATUS "${_variableName}=${${_variableName}}")
#     endforeach()
#     message(STATUS "print_all_variables------------------------------------------}")
# endmacro()

# print_all_variables()


#------------------
# Executable
#------------------


# add the executable
add_executable(run ${SOURCES})

#-------------------
# Unit Tests
#-------------------

# delete main and cli from the core files.
# list(FILTER seqcmp-core EXCLUDE REGEX ".*\\._.*")
list(FILTER SOURCES EXCLUDE REGEX ".*main.cpp")
list(FILTER SOURCES EXCLUDE REGEX ".*/cli/.*")
add_library(seqcmp-core STATIC ${SOURCES})

add_subdirectory(${PROJECT_SOURCE_DIR}/src/external_libraries/googletest EXCLUDE_FROM_ALL)
target_compile_options(gtest_main PRIVATE -w)
target_compile_options(gtest PRIVATE -w)

set(DEATH_TEST_FLAG "") 
if(NO_DEATH_TESTS)
  message(STATUS "Disabling death tests in gtest")
  set(DEATH_TEST_FLAG -D_NO_DEATH_TEST)
endif()

file(GLOB unit_tests_files
  "unit_tests/*.cpp"
  "unit_tests/*/*.cpp"
  "unit_tests/*/*/*.cpp"
  "unit_tests/*/*/*/*.cpp"
)

link_directories(
  ${CMAKE_BINARY_DIR}
)

add_executable(unit_tests ${unit_tests_files})
target_include_directories(unit_tests PRIVATE "${PROJECT_SOURCE_DIR}")
target_compile_definitions(unit_tests PRIVATE TEST_DATA_DIR="${PROJECT_SOURCE_DIR}/tests/data")
target_link_libraries(unit_tests gtest_main gtest gmock seqcmp-core)

target_compile_options(unit_tests PRIVATE -Wno-undefined-var-template ${DEATH_TEST_FLAG})

# Profile with gprof
set(CMAKE_CXX_FLAGS_PROFILE "-pg -DNDEBUG -O2 -g")
set(CMAKE_EXE_LINKER_FLAGS_PROFILE "-pg -g")
set(CMAKE_SHARED_LINKER_FLAGS_PROFILE "-pg -g")

#-------------------
# Integration Tests
#-------------------

# find_package(Python3 3.8.5 REQUIRED COMPONENTS Interpreter)

