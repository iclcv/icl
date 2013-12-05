SET(CMAKE_MODULE_PATH "${ICL_CMAKE_MODULE_PATH}")
MESSAGE(STATUS "CMake modules will be installed to: ${CMAKE_MODULE_PATH}")

INSTALL(FILES FindEigen3.cmake
              FindGLEW.cmake
	      FindIPP.cmake
	      FindLibDC.cmake
	      FindLibFreenect.cmake
	      FindMesaSR.cmake
	      FindMKL.cmake
	      FindOpenCL.cmake
	      FindOpenCV.cmake
	      FindOpenNI.cmake
	      FindPylon.cmake
	      FindV4L.cmake
	      FindXine.cmake
	      InstallFilesRecursive.cmake
       DESTINATION ${CMAKE_MODULE_PATH})
