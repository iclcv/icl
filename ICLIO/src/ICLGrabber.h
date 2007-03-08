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
  class ImgBase;

  /// Common interface class for all grabbers
  class Grabber {
  public:
     Grabber() {}
     virtual ~Grabber() {}

     /** grab directly into the specified destination image 
         or return an internally buffered image

         If specified, the destination image poDst is directly filled
         with a new image. The parameters of poDst, i.e. depth, size,
         channels and format is not changed in this case, but the
         grabbed image is converted to these parameters if neccessary.
         Return value is poDst again.

         If no destination is specified (or NULL pointer given), the
         return value is a pointer to an internally stored image. 
         <b>Ownership for this image remains with the Grabber class.</b>
     */
     virtual const ImgBase* grab(ImgBase *poDst=0)=0;
  }; // class
 
} // namespace icl

#endif
