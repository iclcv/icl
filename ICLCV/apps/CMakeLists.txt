IF(QT_FOUND)
  ADD_SUBDIRECTORY(region-inspector)
  ADD_SUBDIRECTORY(surf-detector)
  ADD_SUBDIRECTORY(crop)
ENDIF()

IF(QT_FOUND AND OpenCV_FOUND)
  ADD_SUBDIRECTORY(lens-undistortion-calibration)
ENDIF()
