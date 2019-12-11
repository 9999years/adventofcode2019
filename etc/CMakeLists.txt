cmake_minimum_required(VERSION 3.10)
project(day_9_intcode_4)
set(CMAKE_CXX_STANDARD 17)

# https://github.com/google/googletest/tree/master/googletest
configure_file(CMakeLists.txt.in deps/CMakeLists.txt)
execute_process(COMMAND ${CMAKE_COMMAND} -G "${CMAKE_GENERATOR}" .
  RESULT_VARIABLE result
  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/deps )
if(result)
  message(FATAL_ERROR "CMake step for googletest failed: ${result}")
endif()
execute_process(COMMAND ${CMAKE_COMMAND} --build .
  RESULT_VARIABLE result
  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/deps )
if(result)
  message(FATAL_ERROR "Build step for googletest failed: ${result}")
endif()
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

find_package( Boost 1.65 COMPONENTS program_options REQUIRED )
include_directories( ${Boost_INCLUDE_DIR} )

add_subdirectory(${CMAKE_CURRENT_BINARY_DIR}/googletest-src
                 ${CMAKE_CURRENT_BINARY_DIR}/googletest-build
                 EXCLUDE_FROM_ALL)
add_subdirectory(${CMAKE_CURRENT_BINARY_DIR}/abseil-src
                 ${CMAKE_CURRENT_BINARY_DIR}/abseil-build
                 EXCLUDE_FROM_ALL)

add_executable(day_9_intcode_4 ${CMAKE_CURRENT_BINARY_DIR}/../../day_9_intcode_4.cpp)
target_compile_options(day_9_intcode_4
    PRIVATE -Werror -Wall -Wextra)
target_link_libraries(day_9_intcode_4 PUBLIC gmock_main gtest_main absl::strings ${Boost_LIBRARIES})
add_test(NAME day_9_intcode_4_test COMMAND day_9_intcode_4)
