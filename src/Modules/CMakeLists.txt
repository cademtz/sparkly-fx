target_sources(xsdk-base PRIVATE
    Draw.cpp
    Menu.cpp
)

if (${XSDK_BUILD_EXAMPLES})
    add_subdirectory(tests)
endif()

add_subdirectory(fx)