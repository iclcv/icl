icl_check_external_package(XCF "xcf/xcf.hpp;xmltio/xmltio.hpp" "xcf;xmltio" lib include /opt/xcf HAVE_XCF_COND TRUE)
if(HAVE_XCF_COND)
  set(XCF_LIBS_l xcf xqilla log4cxx Memory xmltio Ice IceUtil xerces-c)
endif()
