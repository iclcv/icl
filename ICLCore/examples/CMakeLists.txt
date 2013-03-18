# ---- Macro definition ----
MACRO(EXAMPLE NAME)
  SET(BINARY "${NAME}-example")
  LIST(APPEND EXAMPLES ${BINARY})
  ADD_EXECUTABLE(${BINARY} ${ARGN})
  TARGET_LINK_LIBRARIES(${BINARY} ICLCore)
ENDMACRO()

# ---- Examples ----
EXAMPLE(img
        img.cpp)

# ---- Install specifications ----
INSTALL(TARGETS ${EXAMPLES}
        RUNTIME DESTINATION share/${INSTALL_PATH_PREFIX}/examples)
