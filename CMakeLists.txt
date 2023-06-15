cmake_minimum_required(VERSION 3.27)

project(xsdk-base)

add_library(xsdk-base SHARED)
target_compile_features(xsdk-base PUBLIC cxx_std_17)
set_property(TARGET xsdk-base PROPERTY
  MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>"
)
target_include_directories(xsdk-base PRIVATE src/ imgui/ ${CMAKE_CURRENT_SOURCE_DIR}/)
add_subdirectory(src)

# Dependencies
add_subdirectory(minhook)

# Register Zydis dependency.
# Disable build of tools, examples, and documentation.
option(ZYDIS_BUILD_TOOLS "" OFF)
option(ZYDIS_BUILD_EXAMPLES "" OFF)
option(ZYDIS_BUILD_DOXYGEN "" OFF)
set(DOXYGEN_EXECUTABLE "")
add_subdirectory(zydis)

set_property(TARGET Zydis PROPERTY
  MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>"
)
set_property(TARGET Zycore PROPERTY
  MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>"
)
set_property(TARGET minhook PROPERTY
  MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>"
)

target_link_libraries(xsdk-base PRIVATE minhook Zydis)