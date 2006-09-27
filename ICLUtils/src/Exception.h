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

namespace icl {
  
  class ICLException : public std::exception
  {
  private:
    std::string m_sMessage;
    
  public:
    ICLException( std::string msg ) throw() : m_sMessage(msg) {}
    virtual ~ICLException() throw() {};
    virtual const char* what() const throw() {return m_sMessage.c_str();}
    void report();
  };
 
}

#endif // ICLEXCEPTION
