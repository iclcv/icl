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
#include "Exception.h"

namespace icl {

/// Common interface class for all grabbers
  class Grabber : public ICLException
  {
  public:
     Grabber() {}
     virtual ~Grabber() throw () {}
     virtual ImgI* grab(ImgI *poDst)=0;
  }; //class
 
}//namespace icl

#endif
