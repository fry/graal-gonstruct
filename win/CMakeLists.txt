if(WINDRES)
  message("Building gonstruct.res")
  execute_process(
    WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/win
    COMMAND $(WINDRES) "-i gonstruct.rc" "--input-format=rc" "-o gonstruct.res" "-O coff")
endif(WINDRES)

