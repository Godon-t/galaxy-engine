cmake_minimum_required(VERSION 3.16)
project(GalaxyProject)

if(MSVC)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(BUILD_EDITOR "Build the editor application" ON)
option(GALAXY_ENABLE_ASSERTS "Enable assertions" ON)

#list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/external/rpavlik-cmake-modules-fe2273")

# Ajout du sous-projet moteur
add_subdirectory(galaxy-engine)

if(BUILD_EDITOR)
    add_subdirectory(editor)

    add_custom_target(run_editor
        COMMAND $<TARGET_FILE:editor> "${CMAKE_SOURCE_DIR}/project/.gproj"
        DEPENDS editor
        WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
        USES_TERMINAL
    )

    file(COPY_FILE 
        ${CMAKE_SOURCE_DIR}/editor/imgui.ini.template
        ${CMAKE_SOURCE_DIR}/imgui.ini
    )
endif()


