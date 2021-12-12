# ---- Include ICL macros first ----
INCLUDE(ICLHelperMacros)

# ---- Examples ----
BUILD_DEMO(NAME multi-cam-marker-demo
           SOURCES multi-cam-marker-demo.cpp
           LIBRARIES ICLMarkers)
