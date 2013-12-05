IF(QT4_FOUND)
#  ADD_SUBDIRECTORY(corner-detection-css)
  ADD_SUBDIRECTORY(flood-filler)
  ADD_SUBDIRECTORY(mean-shift)
  ADD_SUBDIRECTORY(region-detection)
  ADD_SUBDIRECTORY(simple-blob-searcher)
  ADD_SUBDIRECTORY(vector-tracker)
ENDIF()

IF(QT4_FOUND AND IPP_FOUND)
  ADD_SUBDIRECTORY(hough-line)
  ADD_SUBDIRECTORY(template-matching)
ENDIF()

