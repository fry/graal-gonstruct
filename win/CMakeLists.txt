if(WINDRES)
  add_custom_command(
    OUTPUT ${PROJECT_SOURCE_DIR}/win/gonstruct.res
    WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/win
    COMMAND ${WINDRES} "-igonstruct.rc" "--input-format=rc" "-ogonstruct.res" "-Ocoff")

  add_custom_target(
    generate_resource
    DEPENDS ${PROJECT_SOURCE_DIR}/win/gonstruct.res)
endif(WINDRES)

