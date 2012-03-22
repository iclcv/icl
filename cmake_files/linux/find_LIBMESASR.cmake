icl_check_external_package(LIBMESASR libMesaSR.h mesasr lib include /usr HAVE_LIBMESASR_COND TRUE)

if(HAVE_LIBMESASR_COND)
  set(LIBMESASR_LIBS_l mesasr)
else()
  message(STATUS "LIBMESASR detected: FALSE")
endif()

