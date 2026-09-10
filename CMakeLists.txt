cmake_minimum_required(VERSION 3.16)
project(GalaxyProject)

# set(CMAKE_CXX_FLAGS_DEBUG "-g -O0 -fsanitize=address -fstack-protector -D_FORTIFY_SOURCE=2")
# set(CMAKE_CXX_FLAGS_DEBUG "-g -O0")
# set(CMAKE_C_FLAGS_DEBUG "-g -O0")
set(CMAKE_BUILD_TYPE Debug)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(BUILD_SANDBOX "Build the sandbox application" ON)
option(GALAXY_ENABLE_ASSERTS "Enable assertions" ON)

#list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/external/rpavlik-cmake-modules-fe2273")

# Ajout du sous-projet moteur
add_subdirectory(galaxy-engine)

if(BUILD_SANDBOX)
    add_subdirectory(sandbox)

    add_custom_target(run_sandbox
        COMMAND $<TARGET_FILE:sandbox> "${CMAKE_SOURCE_DIR}/project/.gproj"
        DEPENDS sandbox
        WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
        USES_TERMINAL
    )
endif()


