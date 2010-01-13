#include <ICLUtils/Exception.h>
#include <ICLUtils/Macros.h>
/*
  ICLException.cpp

  Written by: Michael Götting (2006)
              University of Bielefeld
              AG Neuroinformatik
              mgoettin@techfak.uni-bielefeld.de
*/


namespace icl {

  void ICLException::report() {
     FUNCTION_LOG("");
     
     std::cout << "ICL Exception: " << what() << std::endl;
  }

} // namespace icl
