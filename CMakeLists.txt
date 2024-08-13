cmake_minimum_required(VERSION 3.11)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_COMPILE_WARNING_AS_ERROR ON)

project(lochash LANGUAGES CXX)

# setup testing
include(CTest)
enable_testing()

if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
    set_property(GLOBAL PROPERTY USE_FOLDERS ON)
endif()

include(FetchContent)

include_directories("${gtest_SOURCE_DIR}/include"
    "${PROJECT_SOURCE_DIR}/include"
)

if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mavx -msse4.2 -mfpmath=sse")
endif()

if(CMAKE_COMPILER_IS_GNUCXX)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mavx2 -mfpmath=sse")
endif()

add_subdirectory(tests)
