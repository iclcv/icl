# ---- Include ICL macros first ----
INCLUDE(ICLHelperMacros)

# ---- Examples ----
BUILD_APP(NAME create-marker
          SOURCES create-marker.cpp
          LIBRARIES ICLMarkers)
