#add_custom_target(postinstall chmod -f g+w ${CMAKE_INSTALL_PREFIX}/${LIBRARY_OUTPUT_PATH} || { echo "unable to change# permissions for lib" \; exit 1\; }
#					COMMAND chmod -f g+w ${CMAKE_INSTALL_PREFIX}/lib/pkgconfig || { "echo unable to# change permissions for lib/pkgconfig" \; exit 1\; }
#					COMMAND chmod -f g+w ${CMAKE_INSTALL_PREFIX}/${EXECUTABLE_OUTPUT_PATH} || { echo "unable to change permissions for bin" \; exit 1\; }
#					COMMAND chmod -f g+w ${CMAKE_INSTALL_PREFIX}/${DOC_DIR} || { echo "unable to change permissions for doc" \; exit 1\; })

execute_process(COMMAND chmod -f g+w ${CMAKE_INSTALL_PREFIX}/${LIBRARY_OUTPUT_PATH} OUTPUT_QUIET ERROR_QUIET RESULT_VARIABLE ICL_CHMOD_LIB)
execute_process(COMMAND chmod -f g+w ${CMAKE_INSTALL_PREFIX}/lib/pkgconfig OUTPUT_QUIET ERROR_QUIET RESULT_VARIABLE ICL_CHMOD_PKGCFG)
execute_process(COMMAND chmod -f g+w ${CMAKE_INSTALL_PREFIX}/${EXECUTABLE_OUTPUT_PATH} OUTPUT_QUIET ERROR_QUIET RESULT_VARIABLE ICL_CHMOD_BIN)
execute_process(COMMAND chmod -f g+w ${CMAKE_INSTALL_PREFIX}/${DOC_DIR} OUTPUT_QUIET ERROR_QUIET RESULT_VARIABLE ICL_CHMOD_DOC)



# [args1...]]
#                  [COMMAND <cmd2> [args2...] [...]]
#                  [WORKING_DIRECTORY <directory>]
#                  [TIMEOUT <seconds>]
#                  [RESULT_VARIABLE <variable>]
#                  [OUTPUT_VARIABLE <variable>]
#                  [ERROR_VARIABLE <variable>]
#                  [INPUT_FILE <file>]
#                  [OUTPUT_FILE <file>]
#                  [ERROR_FILE <file>]
#                  [OUTPUT_QUIET]
#                  [ERROR_QUIET]
#                  [OUTPUT_STRIP_TRAILING_WHITESPACE]
#                  [ERROR_STRIP_TRAILING_WHITESPACE])