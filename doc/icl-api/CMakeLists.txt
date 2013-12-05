CONFIGURE_FILE("doxyfile.in"
               "${CMAKE_BINARY_DIR}/doc/icl-api/doxyfile"
               @ONLY)

ADD_CUSTOM_TARGET(api doxygen)
