# ---- Include ICL macros first ----
INCLUDE(ICLHelperMacros)

# ---- Examples ----
BUILD_DEMO(NAME bilateral-filter-op
           SOURCES bilateral-filter-op.cpp
		   LIBRARIES ICLUtils ICLFilter ICLQt)

