#ifndef ICLGRABBER_H
#define ICLGRABBER_H

#include <string>
#include <iclImgParams.h>
#include <iclTypes.h>
#include <vector>

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

     /// **NEW** grab function grabs an image (destination image is adapted on demand)
     /** This new grab function is one or the main parts of the ICLs Grabber interface. Its 
        underlying philosophy is as follows:
        - if ppoDst is NULL, a constant image (owned by the grabber) is retuned. Its params and
          depth are adapted to the currently set "desiredParams" and "desiredDepth" (see the 
          parent class icl::Grabber for more details), and the image data is received from the
          camera is converted interanlly to this params and depth.
        - if ppoDst is valid, but it points to a NULL-Pointer (ppoDst!=NULL but *ppoDst==NULL),
          a new image is created at exacly at (*ppoDst). This image is owned by the calling 
          aplication and not by the Grabber.
        - if ppoDst is valid, and it points a a valid ImgBase*, the this ImgBase* is exploited
          as possible. If its depth distincts from the current "desiredDepth" value, it is 
          released, and a new image with the "desired" params and depth is created at (*ppoDst).
          Otherwise, the the ImgBase* at *poDst is adapted in format, channel count and size
          to the "desired" params, before it is filled with data returned
         @param ppoDst destination image (pointer-to-pointer
         @return grabbed image (if ppoDst != 0) equal to *ppoDst
     **/
     virtual const ImgBase* grab(ImgBase **ppoDst=0){ 
       return 0;
     } 

     /// interface for the setter function for video device parameters
     /** In constrast to the setProperty function, this function sets up more <em>critical</em>
         parameters like the grabbed images size and format. The difference to properties, that
         are set via setProperty(), is that setParams() may have to force the underlying grabbing 
         engine to stop or to reinitialize its buffers and convert-engine due to the format or 
         size changes. As there are potential very much params that could make sense in this
         context, this functions as well as the setProperty function are implemented in the 
         most general string-string-manner.\n
         Yet, the following parameters are compulsory:
         - size (syntax for value: e.g. "320x240")
         - format (value depends on the underlying devices formats specifications) 
         
         To get a list of all supported params, call 
         \code getParamList()  \endcode
         
         @param param parameter name to set
         @param value destination parameter value (internally parsed to the desired type)
     **/
     virtual void setParam(const std::string &param, const std::string &value){
       (void)param; (void)value;
     }
     
     /// interface for the setter function for video device properties 
     /** All video device properties can be set using this function. As differenct video devices  
         have different property sets, there are no specialized functions to set special parameters.
         The set of video device parameters consists of two parts:
         To get a list of all possible properties and their corresponding data ranges or value lists,
         call \code getPropertyList() \endcode
         @param property identifier of the property
         @param value value of the property (the value is parsed into the desired type)
     */
     virtual void setProperty(const std::string &property, const std::string &value){
       (void)property; (void)value;
     }

     /// returns a list of properties, that can be set using setProperty
     /** @return list of supported property names **/
     virtual std::vector<std::string> getPropertyList(){
       return std::vector<std::string>();
     }
     
     /// returns a list of supported params, that can be set using setParams
     /** @return list of supported parameters names */
     virtual std::vector<std::string> getParamList(){
       return std::vector<std::string>();
     }


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
     
    private:
     ImgParams m_oDesiredParams;
     depth m_eDesiredDepth;
  }; // class
 
} // namespace icl

#endif
