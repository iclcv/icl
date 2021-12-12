# ---- Include ICL macros first ----
INCLUDE(ICLHelperMacros)

# ---- Examples ----
BUILD_DEMO(NAME png_write_test
           SOURCES png_write_test.cpp
           LIBRARIES ICLIO)
