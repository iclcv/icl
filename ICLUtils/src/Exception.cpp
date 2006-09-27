/*
  ICLException.cpp

  Written by: Michael Götting (2006)
              University of Bielefeld
              AG Neuroinformatik
              mgoettin@techfak.uni-bielefeld.de
*/

#include "Exception.h"

namespace icl {

  void ICLException::report() {
     FUNCTION_LOG("");
     
     std::cout << "ICL Exception: " << m_sMessage << std::endl;
  }

} // namespace icl
