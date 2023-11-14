cmake_minimum_required(VERSION 3.27)

project(xsdk-base)

add_library(xsdk-base SHARED)
target_compile_features(xsdk-base PUBLIC cxx_std_17)
set_property(TARGET xsdk-base PROPERTY
  MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>"
)
target_include_directories(xsdk-base PRIVATE src/ imgui/ ${CMAKE_CURRENT_SOURCE_DIR}/)
add_subdirectory(src)

set(XSDK_BUILD_EXAMPLES 0 CACHE STRING "Build example modules and tests that can be used in-game.")

# Dependencies
add_subdirectory(minhook)
set_property(TARGET minhook PROPERTY
  MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>"
)

target_link_libraries(xsdk-base PRIVATE minhook)

# ===== sparkly-fx dependencies =====
add_subdirectory(miniz)
set_property(TARGET miniz PROPERTY
  MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>"
)

target_link_libraries(xsdk-base PRIVATE miniz)
target_include_directories(xsdk-base PRIVATE libspng/spng qoi)
target_compile_definitions(xsdk-base PRIVATE SPNG_USE_MINIZ SPNG_STATIC)
target_sources(xsdk-base PRIVATE libspng/spng/spng.c)