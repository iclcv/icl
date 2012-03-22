icl_check_external_package(GTEST gtest/gtest.h gtest lib include /vol/nivision/share HAVE_GTEST_COND TRUE)
if(HAVE_GTEST_COND)
  set(GTEST_LIBS_l gtest)
endif()