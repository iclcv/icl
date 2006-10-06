/*
  Grabber.h

  Written by: Michael Götting (2006)
              University of Bielefeld
              AG Neuroinformatik
              mgoettin@techfak.uni-bielefeld.de
*/

#ifndef ICLGRABBER_H
#define ICLGRABBER_H

namespace icl {
  class ImgI;

  /// Common interface class for all grabbers
  class Grabber {
  public:
     Grabber() {}
     virtual ~Grabber() {}
     virtual ImgI* grab(ImgI *poDst)=0;
  }; // class
 
} // namespace icl

#endif
