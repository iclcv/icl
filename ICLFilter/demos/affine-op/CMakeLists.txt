# ---- Include ICL macros first ----
INCLUDE(ICLHelperMacros)

# ---- Examples ----
BUILD_DEMO(NAME affine-op
           SOURCES affine-op.cpp
           LIBRARIES ICLMarkers)
