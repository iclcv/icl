project(ICLFilter)

#sources for lib
set(LIB_SOURCES 
	"src/AffineOp.cpp"
	"src/BinaryArithmeticalOp.cpp"
	"src/BinaryCompareOp.cpp"
	"src/BinaryLogicalOp.cpp"
	"src/ChamferOp.cpp"
	"src/ConvolutionKernel.cpp"
	"src/ConvolutionOp.cpp"
	"src/DynamicConvolutionOp.cpp"
	"src/GaborOp.cpp"
	"src/GradientImage.cpp"
	"src/ImageSplitter.cpp"
	"src/InplaceArithmeticalOp.cpp"
	"src/InplaceLogicalOp.cpp"
	"src/IntegralImgOp.cpp"
	"src/LocalThresholdOp.cpp"
	"src/LUTOp3Channel.cpp"
	"src/LUTOp.cpp"
	"src/MedianOp.cpp"
	"src/MirrorOp.cpp"
	"src/MorphologicalOp.cpp"
	"src/NeighborhoodOp.cpp"
	"src/OpROIHandler.cpp"
	"src/ProximityOp.cpp"
	"src/ThresholdOp.cpp"
	"src/UnaryArithmeticalOp.cpp"
	"src/UnaryCompareOp.cpp"
	"src/UnaryLogicalOp.cpp"
	"src/UnaryOp.cpp"
	"src/BinaryOp.cpp"
	"src/UnaryOpPipe.cpp"
	"src/WeightChannelsOp.cpp"
	"src/WeightedSumOp.cpp"
	"src/WarpOp.cpp"
	"src/FFTOp.cpp"
	"src/IFFTOp.cpp"
)
#create lib
add_library(${PROJECT_NAME} SHARED ${LIB_SOURCES})
#add_library(${PROJECT_NAME} STATIC ${LIB_SOURCES})

set(LIBRARY_OUTPUT_PATH lib CACHE PATH "Output directory for libraries" )

#linker
if(HAVE_IPP_COND)
target_link_libraries(${PROJECT_NAME} ippcore ippi ipps ippsr ippcv ippm ippcc guide iomp5)
endif()
target_link_libraries(${PROJECT_NAME} pthread)
if(HAVE_MKL_COND)
target_link_libraries(${PROJECT_NAME} mkl_intel mkl_intel_thread mkl_core iomp5)
endif()
target_link_libraries(${PROJECT_NAME} ICLCC ICLCore ICLUtils)

link_directories("${LIBRARY_OUTPUT_PATH}")

if(HAVE_QT_COND)
#add_executable(icl-arithmetical-op-test examples/arithmetical-op-test.cpp)
#target_link_libraries(icl-arithmetical-op-test ${PROJECT_NAME})

#add_executable(icl-champfer-op-test examples/icl-champfer-op-test.cpp)
#target_link_libraries(icl-champfer-op-test ${PROJECT_NAME})

#add_executable(icl-inplace-op-test examples/inplace-op-test.cpp)
#target_link_libraries(icl-inplace-op-test ${PROJECT_NAME})

#add_executable(icl-median-op-test examples/median-op-test.cpp)
#target_link_libraries(icl-median-op-test ${PROJECT_NAME})

#add_executable(icl-threshold-op-test examples/threshold-op-test.cpp)
#target_link_libraries(icl-threshold-op-test ${PROJECT_NAME})

#add_executable(icl-weight-channels-op-test examples/weight-channels-op-test.cpp)
#target_link_libraries(icl-weight-channels-op-test ${PROJECT_NAME})

#add_executable(icl-weighted-sum-op-test examples/weighted-sum-op-test.cpp)
#target_link_libraries(icl-weighted-sum-op-test ${PROJECT_NAME})

#add_executable(icl-morphological-op-test examples/morphological-op-test.cpp)
#target_link_libraries(icl-morphological-op-test ${PROJECT_NAME})
endif()

if(HAVE_IPP_COND)
#add_executable(icl-wiener-op-test examples/wiener-op-test.cpp)
#target_link_libraries(icl-wiener-op-test ${PROJECT_NAME})
endif()

if(HAVE_QT_COND)
#add_executable(icl-interactive-canny-demo examples/interactive-canny-demo.cpp)
#target_link_libraries(icl-interactive-canny-demo ${PROJECT_NAME})

#add_executable(icl-local-threshold-op-test examples/local-threshold-op-test.cpp)
#target_link_libraries(icl-local-threshold-op-test ${PROJECT_NAME})

#add_executable(icl-gabor-op-online-example examples/gabor-op-online-example.cpp)
#target_link_libraries(icl-gabor-op-online-example ${PROJECT_NAME})

#add_executable(icl-convolution-op-test examples/convolution-op-test.cpp)
#target_link_libraries(icl-convolution-op-test ${PROJECT_NAME})

#add_executable(icl-warp-op-test examples/warp-op-test.cpp)
#target_link_libraries(icl-warp-op-test ${PROJECT_NAME})

#add_executable(icl-interactive-affine-op-example examples/interactive-affine-op-example.cpp)
#target_link_libraries(icl-interactive-affine-op-example ${PROJECT_NAME})

#add_executable(icl-filter-array examples/filter-array.cpp)
#target_link_libraries(icl-filter-array ${PROJECT_NAME})

#add_executable(icl-fft-demo examples/fft-demo.cpp)
#target_link_libraries(icl-fft-demo ${PROJECT_NAME})
endif()


set(EXECUTABLE_OUTPUT_PATH bin CACHE PATH "Output directory for applications" )

install (TARGETS ${PROJECT_NAME} #icl-arithmetical-op-test icl-champfer-op-test 
		#icl-inplace-op-test icl-median-op-test icl-threshold-op-test 
		#icl-weight-channels-op-test icl-weighted-sum-op-test 
		#icl-morphological-op-test
	 RUNTIME DESTINATION bin
 	 LIBRARY DESTINATION lib)
if(HAVE_IPP_COND)
	install (TARGETS #icl-wiener-op-test
	 RUNTIME DESTINATION bin)
endif()
if(HAVE_QT_COND)
	install (TARGETS #icl-interactive-canny-demo icl-interactive-canny-demo
		#icl-local-threshold-op-test icl-gabor-op-online-example icl-convolution-op-test 
		#icl-warp-op-test icl-interactive-affine-op-example icl-filter-array 
		#icl-fft-demo
	 RUNTIME DESTINATION bin)
endif()
