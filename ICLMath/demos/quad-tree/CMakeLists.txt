# ---- Include ICL macros first ----
INCLUDE(ICLHelperMacros)

# ---- Examples ----
BUILD_DEMO(NAME quad-tree
           SOURCES quad-tree.cpp
           LIBRARIES ICLMarkers)
