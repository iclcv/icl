if(ICL_64BIT)
  icl_check_external_package(PYLON "pylon/PylonIncludes.h;pylon/TransportLayer.h" "pylonbase" lib64 include /opt/pylon HAVE_PYLON_COND TRUE)
else() 
  icl_check_external_package(PYLON "pylon/PylonIncludes.h" "pylonbase" lib include /opt/pylon HAVE_PYLON_COND TRUE)
endif()

if(HAVE_PYLON_COND)
  set (PYLON_LIBS_l "pylonbase;pylonutility;pylongigesupp")
endif()
