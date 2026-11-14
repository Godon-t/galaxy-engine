include_directories(external/imgui/include/)

# set(ALL_LIBS
# 	imgui
# )

add_subdirectory(external/imgui)

add_executable(sandbox
  main.cpp

  EditorLayer.cpp
  external/imguizmo/ImGuizmo.cpp
  editor/ImGui/ImGuiContext.cpp
)

target_link_libraries(sandbox PRIVATE galaxy-engine imgui)
