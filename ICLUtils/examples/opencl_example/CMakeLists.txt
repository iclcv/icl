# ---- Include ICL macros first ----
INCLUDE(ICLHelperMacros)

# ---- Examples ----
BUILD_EXAMPLE(NAME opencl_example
              SOURCES opencl_example.cpp
              LIBRARIES ICLUtils ICLQt)
