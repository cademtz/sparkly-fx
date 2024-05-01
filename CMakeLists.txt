cmake_minimum_required(VERSION 3.27)

project(xsdk-base)

enable_language(ASM_MASM) # Necessary to build AsmTools.asm, written in MASM syntax
add_compile_options(/MP) # Enable multi-threaded compiling
add_compile_options($<IF:$<CONFIG:Debug>,/MTd,/MT>) # Statically linked runtime
add_link_options(/Debug) # Always output debug symbols

add_library(xsdk-base SHARED)
target_compile_features(xsdk-base PUBLIC cxx_std_20)
target_include_directories(xsdk-base PRIVATE src/ imgui/ ${CMAKE_CURRENT_SOURCE_DIR}/)
target_compile_definitions(xsdk-base PRIVATE UNICODE)
add_subdirectory(src)

if(CMAKE_GENERATOR_PLATFORM STREQUAL "Win32")
    set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME "xsdk-base")
else()
    set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME "xsdk-base64")
endif()

set(XSDK_BUILD_EXAMPLES 0 CACHE STRING "Build example modules and tests that can be used in-game.")

# Dependencies
add_subdirectory(minhook)

target_link_libraries(xsdk-base PRIVATE minhook)

# ===== sparkly-fx dependencies =====
add_subdirectory(miniz)
target_link_libraries(xsdk-base PRIVATE miniz)
target_include_directories(xsdk-base PRIVATE libspng/spng qoi ffmpipe/include json/include)
target_compile_definitions(xsdk-base PRIVATE SPNG_USE_MINIZ SPNG_STATIC)
target_sources(xsdk-base PRIVATE libspng/spng/spng.c ffmpipe/src/ffmpipe.cpp)