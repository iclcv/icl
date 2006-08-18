/*
  Grabber.h

  Written by: Michael Götting (2006)
              University of Bielefeld
              AG Neuroinformatik
              mgoettin@techfak.uni-bielefeld.de
*/

#ifndef ICLGRABBER_H
#define ICLGRABBER_H

#include "ImgI.h" 
#include "IO.h"

using namespace std;

namespace icl {

/// The ICLGrabber base class
/**
   bla bla bla
**/

class Grabber
  {
  public:
    Grabber();
    virtual ~Grabber();
    virtual ImgI* grab(ImgI *poDst)=0;
    
  }; //class
 
}//namespace icl

#endif
