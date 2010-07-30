if(WINDRES)
  message(${WINDRES})
  add_custom_command(
    OUTPUT ${PROJECT_SOURCE_DIR}/gonstruct.res
    WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/win
    COMMAND ${WINDRES} "-igonstruct.rc" "--input-format=rc" "-o${PROJECT_SOURCE_DIR}/gonstruct.res" "-Ocoff")

  add_custom_target(
    generate_resource
    DEPENDS ${PROJECT_SOURCE_DIR}/gonstruct.res)
endif(WINDRES)

