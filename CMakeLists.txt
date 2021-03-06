cmake_minimum_required(VERSION 3.8)

if(WIN32)

set(VCPKG_INSTALLATION_ROOT "${CMAKE_SOURCE_DIR}\\vcpkg"
	CACHE STRING "Vcpkg installation directory")

set(CMAKE_TOOLCHAIN_FILE "${VCPKG_INSTALLATION_ROOT}\\scripts\\buildsystems\\vcpkg.cmake"
    CACHE STRING "Vcpkg toolchain file")

endif(WIN32)

project(cjson)

set(CMAKE_C_STANDARD 11)

if(UNIX)
# Set the Compilation options
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Werror -Wextra -ansi")

# Set the compilation options of the test program
set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -fsanitize=address -O0 -fno-omit-frame-pointer -coverage")

# Code coverage
option(CMAKE_C_COVERAGE "Enable Code Coverage" ON)
if(CMAKE_C_COVERAGE AND ${CMAKE_BUILD_TYPE} STREQUAL "Debug")
  set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/CMakeModules)
  include(CodeCoverage)
  setup_target_for_coverage_gcovr_xml(NAME code-coverage EXCLUDE "./test*")
endif()
endif(UNIX)

if(WIN32)
set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS true)
endif(WIN32)

add_library(cjson SHARED cjson.c)

# Test
if(${CMAKE_BUILD_TYPE} STREQUAL "Debug")
  add_executable(test_cjson test_cjson.c)
  target_link_libraries(test_cjson cjson)
  include(CTest)
  add_test(NAME "test_cjson" COMMAND ${PROJECT_BINARY_DIR}/test_cjson)
endif()
