icl_check_external_package(GTEST gtest/gtest.h libgtest.dylib lib include /usr/local HAVE_GTEST_COND TRUE)
if(HAVE_GTEST_COND)
  set(LIBDC_LIBS_l gtest)
endif()
