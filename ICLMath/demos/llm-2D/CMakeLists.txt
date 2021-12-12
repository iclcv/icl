# ---- Include ICL macros first ----
INCLUDE(ICLHelperMacros)

# ---- Examples ----
BUILD_DEMO(NAME llm-2D
           SOURCES llm-2D.cpp
           LIBRARIES ICLMarkers)
