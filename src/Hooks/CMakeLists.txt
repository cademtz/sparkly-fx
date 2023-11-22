target_sources(xsdk-base PRIVATE
    ClientHook.cpp
    Hooks.cpp
    OverlayHook.cpp
    PaintHook.cpp
    WindowHook.cpp
    ModelRenderHook.cpp
)

add_subdirectory(fx)