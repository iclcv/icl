# ---- Include ICL macros first ----
INCLUDE(ICLHelperMacros)

# ---- Examples ----
BUILD_APP(NAME create
          SOURCES create.cpp
          LIBRARIES ICLMarkers)
