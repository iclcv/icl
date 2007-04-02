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
     Grabber() {
       m_eDesiredDepth = depth8u;
       m_oDesiredParams = ImgParams(Size(320,240),formatRGB);
     }
     virtual ~Grabber() {}

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
         - format&size (syntax like 320x240&FORMAT_ID)
         
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

     /// get type of property or parameter
     /** This is a new minimal configuration interface: When implementing generic
         video device configuration utilities, the programmer need information about
         the parameters and properties received by getPropertyList() and 
         getParamList(). With the getType(const string&) function you can intercept
         all possible params and properties, and receive a type string which defines
         of which type the given parameter or property was: \n
         (for detailed description of the types, see also the get Info function)
         Types are:
         - "range" the property/param is a double value in a given range 
         - "value-list" the property/param double value in a list of possible values
         - "menu" the property/param is a string value in a list of possible values
         - ... (propably some other types are defined later on)
     */
     virtual std::string getType(const std::string &name){
       (void)name; return "undefined";
     }
     
     /// get information of a property or parameters valid values values
     /** This is the second function of the minimal configuration interface: If 
         received a specific parameter or property type with getType(), it's
         possible to get the corresponding range, value-list or menu with this
         funcitons. The Syntax of the returned strings are:
         - "[A,B]:C"  for a range with min=A, max=B and stepping = C
         - "{A,B,C,...}" for a value-list and A,B,C are ascii doubles
         - "{A,B,C,...}" for a menu and A,B,C are strings
     */
     virtual std::string getInfo(const std::string &name){
       (void)name; return "undefined";
     }
     /* END-NEW */

     /// returns current desired image params (size and format)
     const ImgParams &getDesiredParams()const{
       return m_oDesiredParams;
     }
     
     /// returns current desired image size (default is "320x240"
     const Size &getDesiredSize()const{
       return m_oDesiredParams.getSize();
     }
     
     /// returns current desired image format (default is formatRGB)
     format getDesiredFormat() const{
       return m_oDesiredParams.getFormat();
     }

     /// returns current desired image depth (default is depth8u)
     depth getDesiredDepth() const{
       return m_eDesiredDepth;
     }

     /// sets current desired image parameters
     void setDesiredParams(const ImgParams &p){
       m_oDesiredParams = p;
     }

     /// sets current desired image size
     void setDesiredSize(const Size &s){
       m_oDesiredParams.setSize(s);
     }
     
     /// sets current desired image format
     void setDesiredFormat(format f){
       m_oDesiredParams.setFormat(f);
     }
     
     /// returns current desired image depth
     void setDesiredDepth(depth d){
       m_eDesiredDepth = d;
     }
     
    private:
     /// internal storage of desired image parameters
     ImgParams m_oDesiredParams;
     
     /// internal storage of desired image depth
     depth m_eDesiredDepth;
  }; // class
 
} // namespace icl

#endif
