# ---- Include ICL macros first ----
INCLUDE(ICLHelperMacros)

# ---- Examples ----
BUILD_DEMO(NAME canny-op
           SOURCES canny-op.cpp
           LIBRARIES ICLMarkers)
