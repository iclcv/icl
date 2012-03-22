icl_check_external_package(OPENCV "cv.h;cvaux.h;cxcore.h;highgui.h" "opencv_core;opencv_highgui;opencv_ml;opencv_features2d;opencv_imgproc;opencv_calib3d" lib include/opencv /usr/local HAVE_OPENCV_COND FALSE)

if(HAVE_OPENCV_COND)
  set(OPENCV_LIBS_l opencv_core opencv_highgui opencv_ml opencv_features2d opencv_imgproc opencv_calib3d)
  add_definitions( -DHAVE_OPENCV)
  set(HAVE_OPENCV_COND TRUE)
else()
  message(STATUS "OPENCV detected: FALSE")
endif()

