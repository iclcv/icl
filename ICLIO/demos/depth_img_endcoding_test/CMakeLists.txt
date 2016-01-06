# ---- Include ICL macros first ----
INCLUDE(ICLHelperMacros)

# ---- Examples ----
BUILD_DEMO(NAME depth_img_endcoding_test
           SOURCES depth_img_endcoding_test.cpp
		   LIBRARIES ICLIO ICLQt)

