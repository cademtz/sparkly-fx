cmake_minimum_required(VERSION 3.27)

project(xsdk-base)

enable_language(ASM_MASM) # Necessary to build AsmTools.asm, written in MASM syntax
add_compile_options(/MP) # Enable multi-threaded compiling
add_compile_options($<IF:$<CONFIG:Debug>,/MTd,/MT>) # Statically linked runtime

add_library(xsdk-base SHARED)
target_compile_features(xsdk-base PUBLIC cxx_std_17)
target_include_directories(xsdk-base PRIVATE src/ imgui/ ${CMAKE_CURRENT_SOURCE_DIR}/)
target_compile_definitions(xsdk-base PRIVATE UNICODE)
add_subdirectory(src)

set(XSDK_BUILD_EXAMPLES 0 CACHE STRING "Build example modules and tests that can be used in-game.")

# Dependencies
add_subdirectory(minhook)

target_link_libraries(xsdk-base PRIVATE minhook)