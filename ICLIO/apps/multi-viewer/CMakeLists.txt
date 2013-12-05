# ---- Include ICL macros first ----
INCLUDE(ICLHelperMacros)

# ---- Examples ----
BUILD_APP(NAME multi-viewer
           SOURCES multi-viewer.cpp
           LIBRARIES ICLMarkers)

