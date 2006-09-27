/*
  Writer.h

  Written by: Michael Götting (2006)
              University of Bielefeld
              AG Neuroinformatik
              mgoettin@techfak.uni-bielefeld.de
*/

#ifndef ICLWRITER_H
#define ICLWRITER_H

#include "ImgI.h" 

namespace icl {

/// The ICLGrabber base class
/**
   bla bla bla
**/

class Writer
  {
  public:
    Writer();
    virtual ~Writer();
    virtual void write(ImgI *poDst)=0;
    
  }; //class
 
}//namespace icl

#endif
