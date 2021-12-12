# ---- Include ICL macros first ----
INCLUDE(ICLHelperMacros)

# ---- Examples ----
BUILD_DEMO(NAME warp-op
           SOURCES warp-op.cpp
           LIBRARIES ICLMarkers)
