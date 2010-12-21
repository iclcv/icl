/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2010 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : include/ICLIO/Grabber.h                                **
** Module : ICLIO                                                  **
** Authors: Christof Elbrechter, Michael Götting, Robert Haschke   **
**                                                                 **
**                                                                 **
** Commercial License                                              **
** ICL can be used commercially, please refer to our website       **
** www.iclcv.org for more details.                                 **
**                                                                 **
** GNU General Public License Usage                                **
** Alternatively, this file may be used under the terms of the     **
** GNU General Public License version 3.0 as published by the      **
** Free Software Foundation and appearing in the file LICENSE.GPL  **
** included in the packaging of this file.  Please review the      **
** following information to ensure the GNU General Public License  **
** version 3.0 requirements will be met:                           **
** http://www.gnu.org/copyleft/gpl.html.                           **
**                                                                 **
** The development of this software was supported by the           **
** Excellence Cluster EXC 277 Cognitive Interaction Technology.    **
** The Excellence Cluster EXC 277 is a grant of the Deutsche       **
** Forschungsgemeinschaft (DFG) in the context of the German       **
** Excellence Initiative.                                          **
**                                                                 **
*********************************************************************/

#ifndef ICLGRABBER_H
#define ICLGRABBER_H

#include <string>
#include <ICLCore/ImgParams.h>
#include <ICLCore/Types.h>
#include <ICLCC/Converter.h>
#include <vector>
#include <ICLUtils/SteppingRange.h>
#include <ICLUtils/Uncopyable.h>
#include <ICLIO/DistFromProgArgUtil.h>
#include <ICLIO/GrabberDeviceDescription.h>

namespace icl {
  /** \cond */
  class ImgBase;
  class WarpOp;
  /** \endcond */
  
  /// Common interface class for all grabbers \ingroup GRABBER_G
  class Grabber : public Uncopyable{
    public:
    /// Basic Grabber constructor
    /** sets desired params to:
        - depth = depth8u
        - size = 320x240
        - foramt = formatRGB 
    **/
    Grabber() : 
    m_oDesiredParams (Size(320,240), formatRGB),
    m_eDesiredDepth (depth8u), 
    m_poImage (0),
    m_bIgnoreDesiredParams(false),m_warp(0),m_distortionBuffer(0){}


    
    /// Destructor
    virtual ~Grabber();

    /// grab function grabs an image (destination image is adapted on demand)
    /** This new grab function is one of the main parts of the ICL Grabber interface. Its 
        underlying philosophy is as follows:
        - if ppoDst is NULL, a constant image (owned by the grabber) is returned. 
        If the desired parameters are used, the returned image will have the 
        desired depth and image params, which is ensured
        by an appropriate conversion from the originally grabbed image if neccessary.
        - if ppoDst is valid, but it points to a NULL-Pointer (ppoDst!=NULL but *ppoDst==NULL),
        a new image is created exacly at (*ppoDst). This image is owned by the calling 
        aplication and not by the Grabber.
        - if ppoDst is valid, and it points to a valid ImgBase*, this ImgBase* is exploited
        as possible. If it's depth differs from the currently "desired" depth value, it is 
        released, and a new image with the "desired" params and depth is created at (*ppoDst).
        Otherwise, the ImgBase* at *poDst is adapted in format, channel count and size
        to the "desired" params or to the depth and parameters of the image that was 'produced'
        by the Grabber, before it is filled with data and returned
        <b>Note:</b> Most of the time you'll simply used ppoDst's default value NULL. In this
        case you only have read-access to the returned ImgBase, which is most of the time
        completely sufficient.
        <b>Note also:</b> For an easy handling of ppoDst and the grabber optionally used
        desired paremeters, you can simple call the Grabbers adaptGrabResult method.

        @see Grabber::adaptGrabResult
        @param ppoDst destination image (pointer-to-pointer)
        @return grabbed image (if ppoDst != 0 and depth matches) equal to *ppoDst
     **/
     virtual const ImgBase* grabUD(ImgBase **ppoDst=0) = 0;


    /// grab function calls the Grabber-specific grabUD function and applies distortion if necessary
    /** @see grabUD **/
     const ImgBase *grab(ImgBase **ppoDst=0);

     /// @{ @name get/set properties  

     /// interface for the setter function for video device properties 
     /** All video device properties can be set using this function. As different video devices  
         have different property sets, there are no specialized functions to set special parameters.
         To get a list of all possible properties and their corresponding data ranges or value lists,
         call \code getPropertyList()  and getInfo() \endcode
         Yet, the following properties are compulsory for grabbers:
         - size (syntax for value: e.g. "320x240")
         - format (value depends on the underlying devices formats specifications) 
         (If your grabber does only provided one format, e.g. RGB24 or one specifiy size, you
         should create a menu property for format and for size, where each menu has only one 
         valid entry.

         Other parameters, implemented for most video devices are: 
         - "frame rate"
         - "exposure"
         - "shutter speed"
         - "gain"
         - ...
         
         Look into the documentation of the special grabber classes or explore the device paremeters
         with the <em>camcfg</em> utility application, located in ICLQt/examples
         
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
     
     /// base implementation for property check (seaches in the property list)
     /** This function may be reimplemented in an optimized way in
         particular subclasses.**/
     virtual bool supportsProperty(const std::string &property);

     
     /// writes all available properties into a file
     /** @param filename destination xml-filename
         @param writeDesiredParams if this flag is true, current grabbers desired params are written to the
                                   config file as well 
         @param skipUnstable some common grabber parameters e.g. trigger-settings cause problems when
                             they are read from configuration files, hence these parameters are skipped at default*/
     virtual void saveProperties(const std::string &filename, bool writeDesiredParams=true, bool skipUnstable=true);

     /// reads a camera config file from disc
     /** @ see saveProperties */
     virtual void loadProperties(const std::string &filename, bool loadDesiredParams=true, bool skipUnstable=true);

     /// get type of property 
     /** This is a new minimal configuration interface: When implementing generic
         video device configuration utilities, the programmer needs information about
         the properties received by getPropertyList(). With the getType(const string&)
         function, you can explore
         all possible params and properties, and receive a type string which defines
         of which type the given property was: \n
         (for detailed description of the types, see also the get Info function)
         Types are:
         - "range" the property is a double value in a given range 
         - "value-list" the property is a double value in a list of possible values
         - "menu" the property  is a string value in a list of possible values
         - "command" property param has no additional parameters (this feature is 
           used e.g. for triggered abilities of grabbing devices, like 
           "save user settings" for the PWCGrabber 
         - "info" the property is an unchangable internal value (it cannot be set actively)
         - ... (propably some other types are defined later on)
     */
     virtual std::string getType(const std::string &name){
       (void)name; return "undefined";
     }
     
     /// get information of a properties valid values
     /** This is the second function of the minimal configuration interface: If 
         received a specific property type with getType(), it's
         possible to get the corresponding range, value-list or menu with this
         funcitons. The Syntax of the returned strings are:
         - "[A,B]:C"  for a range with min=A, max=B and stepping = C
         - "{A,B,C,...}" for a value-list and A,B,C are ascii doubles
         - "{A,B,C,...}" for a menu and A,B,C are strings
         - nothing for "info"-typed properties
         <b>Note:</b> The received string can be translated into C++ data
         with some static utility function in this Grabber class.
         @see translateSteppingRange
         @see translateDoubleVec
         @see translateStringVec
     */
     virtual std::string getInfo(const std::string &name){
       (void)name; return "undefined";
     }

     /// returns the current value of a property or a parameter
     virtual std::string getValue(const std::string &name){
       (void)name; return "undefined";
     }

     /// Returns whether this property may be changed internally
     /** For example a video grabber's current stream position. This can be changed
         from outside, but it is changed when the stream is played. The isVolatile
         function should return a msec-value that describes how often the corresponding
         feature might be updated internally or just 0, if the corresponding
         feature is not volatile at all. The default implementation of isVolatile
         returns 0 for all features. So if there is no such feature in your grabber,
         this function must not be adapted at all. "info"-typed Properties might be
         volatile as well */
     virtual int isVolatile(const std::string &propertyName){
       (void)propertyName; return false;
     }
     
     /// @} 
     /// @{ @name static string conversion functions 

     /// translates a SteppingRange into a string representation
     static std::string translateSteppingRange(const SteppingRange<double>& range);

     /// creates a SteppingRange out of a string representation
     static SteppingRange<double> translateSteppingRange(const std::string &rangeStr);

     /// translates a vector of doubles into a string representation
     static std::string translateDoubleVec(const std::vector<double> &doubleVec);

     /// creates a vector of doubles out of a string representation
     static std::vector<double> translateDoubleVec(const std::string &doubleVecStr);

     /// translates a vector of strings into a single string representation
     static std::string translateStringVec(const std::vector<std::string> &stringVec);

     /// creates a vector of strins out of a single string representation
     static std::vector<std::string> translateStringVec(const std::string &stringVecStr);


     /// @} 
     /// @{ @name functions for get/set desired params

     /// returns current desired image params (size and format)
     virtual const ImgParams &getDesiredParams()const{
       return m_oDesiredParams;
     }
     
     /// returns current desired image size (default is "320x240"
     virtual const Size &getDesiredSize()const{
       return m_oDesiredParams.getSize();
     }
     
     /// returns current desired image format (default is formatRGB)
     virtual format getDesiredFormat() const{
       return m_oDesiredParams.getFormat();
     }

     /// returns current desired image depth (default is depth8u)
     virtual depth getDesiredDepth() const{
       return m_eDesiredDepth;
     }

     /// sets current desired image parameters
     virtual void setDesiredParams(const ImgParams &p){
       m_oDesiredParams = p;
     }

     /// sets current desired image size
     virtual void setDesiredSize(const Size &s){
       m_oDesiredParams.setSize(s);
     }
     
     /// sets current desired image format
     virtual void setDesiredFormat(format f){
       m_oDesiredParams.setFormat(f);
     }
     
     /// returns current desired image depth
     virtual void setDesiredDepth(depth d){
       m_eDesiredDepth = d;
     }
     
     /// set up ignore-desired params flag
     virtual void setIgnoreDesiredParams(bool flag){
       m_bIgnoreDesiredParams = flag;
     }
     
     /// @see setIgnoreDesiredParams
     virtual void setUseDesiredParams(bool flag){
       setIgnoreDesiredParams(!flag);
     }

     /// returns wheterh desired params are ignored or used
     virtual bool getIgnoreDesiredParams() const {
       return m_bIgnoreDesiredParams;
     }

     /// returns wheterh desired params are used or ignored
     virtual bool getUseDesiredParams() const {
       return !m_bIgnoreDesiredParams;
     }
     
     /// @}

     
     /// @{ @name distortion functions
     
     /// enabled the distortion plugin for the grabber using radial distortion parameters
     /** distortion is calculated as follows: 
         \code    
         Point32f distort(int xi, int yi){
           const double &x0 = params[0];
           const double &y0 = params[1];
           const double &f = params[2]/100000000.0;
           const double &s = params[3];
         
           float x = s*(xi-x0);
           float y = s*(yi-y0);
           float p = 1 - f * (x*x + y*y);
           return Point32f(p*x + x0,p*y + y0);
         }
         \endcode
         
         - Good params for pwc camera are { 354.5, 185, 25.7, 1.00904}
         - optimal parameters can be found using the ICL-application
           'icl-calib-radial-distortion'

     */
     void enableDistortion(double params[4],const Size &size, scalemode m=interpolateLIN);

     /// enables distortion for given warp map
     void enableDistortion(const Img32f &warpMap, scalemode m=interpolateLIN);
     
     /// disables distortion
     void disableDistortion();
     
     /// returns whether distortion is currently enabled
     bool isDistortionEnabled() const { return !!m_warp; }
     
     /// @}

    protected:
     /// *NEW* Utility function that allows for much easier implementation of grabUD
     /** Most of the time, grab/grabUD is called without a given destination ImgBase**.
         Furthermore, most grabbers do only provide one specific image format/size-
         combination natively. Most other combinations of desired depth, -format and -size
         have to be converted using the Grabbers protected m_oConverter and m_oImage instances.
         Since it turned out, that this functionality had to be implemeneted in almost each
         Grabber implementation, ICL's Grabber does now provide this convenience function 
         which implements the following functionality:
         <pre>
         // pseudocode:
         const ImgBase *Grabber::adaptGrabResult(const ImgBase *src, ImgBase **dst){
           if( desired parameters are used ){
             if( src has by chance the desired depth, -size and -format ){
               if ( dst is not NULL ){
                  copy src to dst by adapting dst apropriately and return *dst
               } else {
                  return src directly
               }
             } else { // src has to be converted 
               if (dst is not NULL ){
                  adapt dst to the desired parameters, convert src to *dst and return *dst
               } else { // dst was not given at all
                  adapt m_poImage to the desired parameters, convert src into it and return it 
               }
             }
           } else { // desired parameters are not used
             if ( dst is not NULL ) {
                copy src deeply to dst by adapting dst and return *dst
             } else {
                return src directly
             }
           }
         }
         </pre>
         
         adaptGrabResult can be used very easily within your Grabber implementations
         grabUD method.
         \code
         struct MyGrabber{
            Img8u buffer;
            const ImgBase* grabUD(ImgBase **dst){
               // first, you have to create your image
               buffer.setSize(Size::VGA);
               buffer.setFormat(formatRGB);
               buffer.fill(URandI(255));
              
               // adaptGrabResult will do the rest for you!
               return adaptGrabResult(&buffer,dst);
            }
         };
         \endcode
         
         
     */
     const ImgBase *adaptGrabResult(const ImgBase *src, ImgBase **dst); //!< utility method (todo: move to Grabber base class)

     /// internally used by the load- and saveProperties
     /** If any property shall not be save or loaded from configuration file, it must be filtered out by this f*/
     virtual std::vector<std::string> get_io_property_list() { return getPropertyList(); }
     
     /// prepare depth and params of output image according to desired settings
     ImgBase* prepareOutput (ImgBase **ppoDst);

     /// internal storage of desired image parameters
     ImgParams m_oDesiredParams;
     
     /// internal storage of desired image depth
     depth m_eDesiredDepth;

     /// converter used for conversion to desired output depth/params 
     /** This instance of the Converter class can be used in derived classes
         to adapt a grabbed image the desired params 
     */
     Converter m_oConverter;

     /// interal output image instance used if ppoDst is zero in grab()
     /** This image can be used in derived classes if the ImgBase** that
         was passed to the grab(..) function was NULL. In this case, this
         ImgBase should be used and returned. This will help to avoide
         runtime memory allocation and deallocations. 
     */
     ImgBase  *m_poImage;

     /// Flag to indicate whether desired parametes should be ignored
     bool m_bIgnoreDesiredParams;

     /// for distortion
     WarpOp *m_warp;
     
     /// for distortion
     ImgBase *m_distortionBuffer;
  }; 
 


} // namespace icl

#endif
