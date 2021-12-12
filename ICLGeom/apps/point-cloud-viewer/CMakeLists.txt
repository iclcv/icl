# ---- Include ICL macros first ----
INCLUDE(ICLHelperMacros)

# ---- Examples ----
BUILD_APP(NAME point-cloud-viewer
          SOURCES point-cloud-viewer.cpp
          LIBRARIES ICLMarkers)
