# ---- Include ICL macros first ----
INCLUDE(ICLHelperMacros)

# ---- Examples ----
BUILD_EXAMPLE(NAME model-fitting
              SOURCES model-fitting.cpp
              LIBRARIES ICLQt)
