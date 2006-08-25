/*
  ICLException.cpp

  Written by: Michael Götting (2006)
              University of Bielefeld
              AG Neuroinformatik
              mgoettin@techfak.uni-bielefeld.de
*/

#include "Exception.h"

namespace icl {

  ICLException::ICLException( string msg ) 
  {
    m_sMessage = msg;    
  }
  
  void ICLException::report() {
    FUNCTION_LOG("");
    
    cout << "ICL Exception: " << m_sMessage << endl;
  }

  const char* ICLException::what() const throw() {
    return m_sMessage.c_str();
  }
} // namespace icl
