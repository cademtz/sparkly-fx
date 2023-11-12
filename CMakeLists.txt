cmake_minimum_required(VERSION 3.27)

project(xsdk-base)

add_library(xsdk-base SHARED)
target_compile_features(xsdk-base PUBLIC cxx_std_17)
set_property(TARGET xsdk-base PROPERTY
  MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>"
)
target_include_directories(xsdk-base PRIVATE src/ imgui/ ${CMAKE_CURRENT_SOURCE_DIR}/)
add_subdirectory(src)

if (${CMAKE_BUILD_TYPE} AND ${CMAKE_BUILD_TYPE} STREQUAL "Release")
  set(XSDK_BUILD_EXAMPLES_DEFAULT OFF)
else()
  set(XSDK_BUILD_EXAMPLES_DEFAULT ON)
endif()

set(XSDK_BUILD_EXAMPLES ${XSDK_BUILD_EXAMPLES_DEFAULT} CACHE STRING "Build example code, such as test features that can be used in the menu.")

# Dependencies
add_subdirectory(minhook)
set_property(TARGET minhook PROPERTY
  MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>"
)

target_link_libraries(xsdk-base PRIVATE minhook)