# ---- Include ICL macros first ----
INCLUDE(ICLHelperMacros)

# ---- Examples ----
BUILD_APP(NAME viewer
          SOURCES viewer.cpp
          LIBRARIES ICLMarkers)
