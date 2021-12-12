# ---- Include ICL macros first ----
INCLUDE(ICLHelperMacros)

# ---- Examples ----
BUILD_APP(NAME simple-point-cloud-viewer
          SOURCES simple-point-cloud-viewer.cpp
          LIBRARIES ICLMarkers)
