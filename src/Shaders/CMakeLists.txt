target_sources(xsdk-base PRIVATE
    shaders.cpp
    shaders-menu.cpp
)

# Build HLSL shaders
add_custom_target(shaders)

set(HLSL_SHADER_FILES
    depth_linear.hlsl
    depth_logarithm.hlsl
    depth_to_rgb.hlsl
    normal_to_rgb.hlsl
)

set(SHADER_FOLDER ${CMAKE_BINARY_DIR}/$<CONFIG>/shaders)
# This doesn't work idk why
#make_directory(${SHADER_FOLDER})
add_custom_target(build-time-make-directory ALL COMMAND ${CMAKE_COMMAND} -E make_directory ${SHADER_FOLDER})

foreach(FILE ${HLSL_SHADER_FILES})
    cmake_path(REMOVE_EXTENSION FILE OUTPUT_VARIABLE FILE_WE)
    get_source_file_property(shadertype ${FILE} ShaderType)
    get_source_file_property(shadermodel ${FILE} ShaderModel)
    add_custom_command(
        TARGET shaders
        COMMAND fxc.exe /nologo /Emain /T ps_3_0 $<IF:$<CONFIG:DEBUG>,/Od,/O1> /Zi /Fo "${SHADER_FOLDER}/${FILE_WE}.cso" "${FILE}"
        MAIN_DEPENDENCY ${FILE}
        COMMENT "HLSL ${FILE}"
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        VERBATIM
    )
endforeach(FILE)
add_dependencies(${PROJECT_NAME} shaders)