include_directories(external/imgui/include/)
include_directories(external/imguizmo)

# set(ALL_LIBS
# 	imgui
# )

add_subdirectory(external/imgui)

add_executable(editor
  main.cpp
  
  EditorLayer.cpp
  src/ImGui/ImGuiContext.cpp
  
  external/imguizmo/ImGuizmo.cpp
)

target_link_libraries(editor PRIVATE galaxy-engine imgui)
