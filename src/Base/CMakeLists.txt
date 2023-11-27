target_sources(xsdk-base PRIVATE
    AltimorNetvar.cpp
    AsmTools.cpp
    Base.cpp
    Entity.cpp
    Event.cpp
    imgui_impl_win32.cpp
    imgui.cpp
    Interfaces.cpp
    Netvars.cpp
    Sig.cpp
    vstdlib_imports.cpp
    AsmTools64.asm
)

add_subdirectory(Wrappers)