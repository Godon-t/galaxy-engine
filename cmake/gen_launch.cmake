# cmake/gen_launch.cmake
file(WRITE "${OUT}" "#!/bin/bash\n${EXE}\n")
# Marquer exécutable sous UNIX
if(UNIX)
  file(CHMOD "${OUT}" FILE_PERMISSIONS
       OWNER_READ OWNER_WRITE OWNER_EXECUTE
       GROUP_READ  GROUP_EXECUTE
       WORLD_READ  WORLD_EXECUTE)
endif()
