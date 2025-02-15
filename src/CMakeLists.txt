cmake_minimum_required(VERSION 3.24)
set(CMAKE_CXX_STANDARD 17)

project(low-budget-grafana)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set_property(GLOBAL PROPERTY PREDEFINED_TARGETS_FOLDER "cmake")

if (NOT MSVC)
    list(APPEND CMAKE_CXX_STANDARD_LIBRARIES "-lstdc++ -lm")
endif()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/bin)

option(TEST "TEST" OFF)

if(TEST)
    enable_testing()
endif()

add_subdirectory(thirdparty)
add_subdirectory(app)
add_subdirectory(lib)
include_directories(lib)
