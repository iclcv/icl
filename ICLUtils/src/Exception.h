/*
  ICLException.h

  Written by: Michael Götting (2006)
              University of Bielefeld
              AG Neuroinformatik
              mgoettin@techfak.uni-bielefeld.de
*/

#ifndef ICLEXCEPTION_H
#define ICLEXCEPTION_H

#include <iostream>
#include "Macros.h"
#include <exception>

using namespace std;

namespace icl {
  
  class ICLException : public std::exception
  {
  private:
    string m_sMessage;
    
  public:
    ICLException( string msg );
    virtual ~ICLException() throw() {};
    
    void report();
  };
 
}

#endif // ICLEXCEPTION
