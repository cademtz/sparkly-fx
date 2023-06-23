target_sources(xsdk-base PRIVATE Main.cpp)
add_subdirectory(Base)
add_subdirectory(Hooks)
add_subdirectory(Modules)
add_subdirectory(SDK)

add_subdirectory(Streams)