# ---- Include ICL macros first ----
INCLUDE(ICLHelperMacros)

# ---- Examples ----
BUILD_DEMO(NAME llm-1D
           SOURCES llm-1D.cpp
           LIBRARIES ICLMarkers)
