add_executable(sandbox
  main.cpp

  EditorLayer.cpp
)

target_link_libraries(sandbox PRIVATE galaxy-engine)
