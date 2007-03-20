#ifndef ICLGRABBER_H
#define ICLGRABBER_H

#include <string>
#include <iclImgParams.h>
#include <iclTypes.h>

/*
  Grabber.h

  Written by: Michael Götting (2006)
              University of Bielefeld
              AG Neuroinformatik
              mgoettin@techfak.uni-bielefeld.de
*/

namespace icl {
  class ImgBase;

  /// Common interface class for all grabbers
  class Grabber {
  public:
     Grabber() {}
     virtual ~Grabber() {}

     /**  DEPRECATED !!

         grab directly into the specified destination image 
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

     /// new------------------------------------------------
     virtual const ImgBase* grab(ImgBase **ppoDst=0){ return 0;} // later = 0
     const ImgParams &getDesiredParams()const{
       return m_oDesiredParams;
     }
     const Size &getDesiredSize()const{
       return m_oDesiredParams.getSize();
     }
     format getDesiredFormat() const{
       return m_oDesiredParams.getFormat();
     }
     depth getDesiredDepth() const{
       return m_eDesiredDepth;
     }

     void setDesiredParams(const ImgParams &p){
       m_oDesiredParams = p;
     }
     void setDesiredSize(const Size &s){
       m_oDesiredParams.setSize(s);
     }
     void setDesiredFormat(format f){
       m_oDesiredParams.setFormat(f);
     }
     void setDesiredDepth(depth d){
       m_eDesiredDepth = d;
     }
     
     virtual void setParam(const std::string &param, const std::string &value){
       (void)param; (void)value;
     }
     
    private:
     ImgParams m_oDesiredParams;
     depth m_eDesiredDepth;
  }; // class
 
} // namespace icl

#endif
