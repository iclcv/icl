# ---- Include ICL macros first ----
INCLUDE(ICLHelperMacros)

# ---- Examples ----
BUILD_DEMO(NAME convolution-op
           SOURCES convolution-op.cpp
           LIBRARIES ICLMarkers)
