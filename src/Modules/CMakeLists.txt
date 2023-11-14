target_sources(xsdk-base PRIVATE
    Draw.cpp
    Menu.cpp
)

if ($CACHE{XSDK_BUILD_EXAMPLES})
    add_subdirectory(tests)
endif()
