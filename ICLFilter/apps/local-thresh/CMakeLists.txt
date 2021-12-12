# ---- Include ICL macros first ----
INCLUDE(ICLHelperMacros)

# ---- Examples ----
BUILD_APP(NAME local-thresh
          SOURCES local-thresh.cpp
          LIBRARIES ICLMarkers)
