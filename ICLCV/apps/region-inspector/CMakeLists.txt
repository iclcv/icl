# ---- Include ICL macros first ----
INCLUDE(ICLHelperMacros)

# ---- Examples ----
BUILD_APP(NAME region-inspector
          SOURCES region-inspector.cpp
          LIBRARIES ICLMarkers)

