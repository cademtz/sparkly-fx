target_sources(xsdk-base PRIVATE
    AltimorNetvar.cpp
    AsmTools.cpp
    Base.cpp
    Entity.cpp
    Event.cpp
    imgui_impl_win32.cpp
    imgui_impl_dx9.cpp
    imgui.cpp
    Interfaces.cpp
    Netvars.cpp
    Sig.cpp
)

add_subdirectory(Wrappers)