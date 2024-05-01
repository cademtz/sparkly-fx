target_sources(xsdk-base PRIVATE
    Draw.cpp
    InputModule.cpp
    GameEjector.cpp
    BaseModule.cpp
)

if ($CACHE{XSDK_BUILD_EXAMPLES})
    add_subdirectory(tests)
endif()

add_subdirectory(fx)