cmake_minimum_required (VERSION 3.8)
project("TKEngine")

macro(link_CESDK projName)
    add_dependencies(${projName} CECore)
    target_link_libraries(${projName} CECore)
endmacro(link_CESDK)

set(SolutionDir ${PROJECT_SOURCE_DIR})

if( CMAKE_BUILD_TYPE STREQUAL "Debug" )
    set(CEOutPutDir ${PROJECT_SOURCE_DIR}/_Out/Debug)
else()
    set(CEOutPutDir ${PROJECT_SOURCE_DIR}/_Out/Release)
endif()

message("CEOutPutDir:${CEOutPutDir}")

message("Build SDK")
add_subdirectory("TKSystem")

add_executable(demo Test/base_learning_01/main.cpp)
