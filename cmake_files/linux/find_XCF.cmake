icl_check_external_package(XCF "xcf/xcf.hpp;xmltio/xmltio.hpp" "xcf;xmltio;Memory" lib include HAVE_XCF_COND TRUE TRUE)
#if(HAVE_XCF_COND)
#  set(XCF_LIBS_l xcf xqilla log4cxx Memory xmltio Ice IceUtil xerces-c)
#endif()
