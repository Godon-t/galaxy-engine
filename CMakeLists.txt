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

# Optionnel : ajout du sandbox
if(BUILD_SANDBOX)
  	add_subdirectory(sandbox)

	add_custom_command(
		OUTPUT  "${CMAKE_BINARY_DIR}/launch-sandbox.sh"
		COMMAND "${CMAKE_COMMAND}" -DOUT=${CMAKE_BINARY_DIR}/launch-sandbox.sh
								-DEXE="$<TARGET_FILE:sandbox>"
								-DARGS="${CMAKE_SOURCE_DIR}/project/.gproj"
								-P "${CMAKE_SOURCE_DIR}/cmake/gen_launch.cmake"
		DEPENDS sandbox
		COMMENT "Génération du script launch-sandbox.sh"
		VERBATIM
	)

	add_custom_target(launch_sandbox ALL
		DEPENDS "${CMAKE_BINARY_DIR}/launch-sandbox.sh"
	)
endif()


