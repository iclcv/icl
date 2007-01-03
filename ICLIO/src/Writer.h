/*
  Writer.h

  Written by: Michael Götting (2006)
              University of Bielefeld
              AG Neuroinformatik
              mgoettin@techfak.uni-bielefeld.de
*/

#ifndef ICLWRITER_H
#define ICLWRITER_H

namespace icl {
  class ImgBase;

  /// The ICLGrabber base class
  class Writer {
  public:
     Writer() {}
     virtual ~Writer() {}
     virtual void write(const ImgBase *poDst)=0;
  }; //class
 
}//namespace icl

#endif
