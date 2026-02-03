# CMake entry point
find_package(OpenGL REQUIRED)



if( CMAKE_BINARY_DIR STREQUAL CMAKE_SOURCE_DIR )
    message( FATAL_ERROR "Please select another Build Directory ! (and give it a clever name, like bin_Visual2012_64bits/)" )
endif()
if( CMAKE_SOURCE_DIR MATCHES " " )
	message( "Your Source Directory contains spaces. If you experience problems when compiling, this can be the cause." )
endif()
if( CMAKE_BINARY_DIR MATCHES " " )
	message( "Your Build Directory contains spaces. If you experience problems when compiling, this can be the cause." )
endif()



# Compile external dependencies 
add_subdirectory (external)

if(INCLUDE_DISTRIB)
	add_subdirectory(distrib)
endif(INCLUDE_DISTRIB)



include_directories(
    external/glfw-3.4/include/
    external/glm-0.9.7.1/
    external/glew-1.13.0/include/
    external/imgui/include/
    .
)

set(ALL_LIBS
    ${OPENGL_LIBRARY}
    glfw
    GLEW_1130
    assimp
	yaml-cpp
	imgui
	spdlog::spdlog_header_only
)

add_definitions(
	-DTW_STATIC
	-DTW_NO_LIB_PRAGMA
	-DTW_NO_DIRECT3D
	-DGLEW_STATIC
	-D_CRT_SECURE_NO_WARNINGS
	-DIMGUI_IMPL_OPENGL_LOADER_GLEW
)

add_library(galaxy-engine
	src/platform/linux/LinuxWindow.cpp
	src/engine/core/Application.cpp

	src/engine/core/Log.cpp
	src/engine/core/Layer.cpp
	src/engine/core/TimeLogger.cpp
	src/engine/core/LayerStack.cpp

	src/engine/event/InputManager.cpp

	src/engine/ImGui/ImGuiLayer.cpp

	src/engine/data/Transform.cpp

	src/engine/project/Project.cpp
	src/engine/project/UUID.cpp
	
	src/engine/nodes/NodeHelper.cpp
	src/engine/nodes/Node.cpp
	src/engine/nodes/Node3D.cpp
	src/engine/nodes/Root.cpp
	src/engine/nodes/visitors/Serializer.cpp
	src/engine/nodes/visitors/Deserializer.cpp
	src/engine/nodes/rendering/MeshInstance.cpp
	src/engine/nodes/rendering/EnvironmentNode.cpp
	src/engine/nodes/rendering/Sprite3D.cpp
	src/engine/nodes/rendering/CornellBox.cpp
	src/engine/nodes/rendering/Camera.cpp
	src/engine/nodes/rendering/lighting/SpotLight.cpp
	src/engine/nodes/rendering/lighting/PointLight.cpp
	src/engine/nodes/rendering/lighting/GINode.cpp

	src/engine/rendering/OpenglHelper.cpp
	src/engine/rendering/Program.cpp
	src/engine/rendering/renderer/Renderer.cpp
	src/engine/rendering/renderer/Backend.cpp
	src/engine/rendering/renderer/Frontend.cpp
	src/engine/rendering/renderer/LightManager.cpp
	src/engine/rendering/CameraManager.cpp
	src/engine/rendering/GPUInstances/VisualInstance.cpp
	src/engine/rendering/GPUInstances/Texture.cpp
	src/engine/rendering/GPUInstances/FrameBuffer.cpp
	src/engine/rendering/GPUInstances/DebugLines.cpp
	src/engine/rendering/GPUInstances/UBOInstance.cpp
	
	src/engine/resource/ResourceSerializer.cpp
	src/engine/resource/Image.cpp
	src/engine/resource/Mesh.cpp
	src/engine/resource/Material.cpp
	src/engine/resource/Environment.cpp

	src/engine/common/YamlTranslation.cpp
)

target_compile_definitions(galaxy-engine PUBLIC
  RESOURCE_ROOT="${CMAKE_CURRENT_SOURCE_DIR}/"
)

target_compile_definitions(galaxy-engine PRIVATE GALAXY_ENGINE_BUILD)

target_include_directories(galaxy-engine PUBLIC src)
target_include_directories(galaxy-engine PUBLIC src/engine)
target_include_directories(galaxy-engine PUBLIC src/engine/core)

target_precompile_headers(galaxy-engine PUBLIC src/engine/core/pch.hpp)

target_include_directories(galaxy-engine PUBLIC external/glm-0.9.7.1)
target_include_directories(galaxy-engine PUBLIC external/stbi)

# TODO: move to right spot
add_subdirectory(external/imgui)


target_link_libraries(galaxy-engine ${ALL_LIBS})
# Xcode and Visual working directories
#include(CreateLaunchers)
set_target_properties(galaxy-engine PROPERTIES XCODE_ATTRIBUTE_CONFIGURATION_BUILD_DIR "${CMAKE_CURRENT_SOURCE_DIR}/")
#create_target_launcher(galaxy-engine WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/")



SOURCE_GROUP(shaders REGULAR_EXPRESSION ".*/.*shader$" )


