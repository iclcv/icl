SET(CMAKE_MODULE_PATH "${ICL_CMAKE_MODULE_PATH}")
MESSAGE(STATUS "CMake modules will be installed to: ${CMAKE_MODULE_PATH}")

INSTALL(FILES FindIPP.cmake
              FindLibDC.cmake
              FindMesaSR.cmake
              FindOpenCL.cmake
              FindOpenCV.cmake
              FindOpenNI.cmake
              FindPylon.cmake
              FindV4L.cmake
              FindWinProtobuf.cmake
              InstallFilesRecursive.cmake
       DESTINATION ${CMAKE_MODULE_PATH})
