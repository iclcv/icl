/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2012 CITEC, University of Bielefeld          **
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
#include <vector>

#include <ICLCore/ImgBase.h>
#include <ICLUtils/SteppingRange.h>
#include <ICLUtils/Function.h>
#include <ICLUtils/Uncopyable.h>
#include <ICLUtils/ProgArg.h>
#include <ICLIO/GrabberDeviceDescription.h>
#include <ICLIO/ImageUndistortion.h>

namespace icl {

  /** \cond */
  namespace{
    template <class T> inline T grabber_get_null(){ return 0; }
    template <> inline icl::format grabber_get_null<icl::format>(){ return (icl::format)-1; }
    template <> inline icl::depth grabber_get_null<icl::depth>(){ return (icl::depth)-1; }
    template <> inline icl::Size grabber_get_null<icl::Size>(){ return icl::Size::null; }

    struct grabber_get_xxx_dummy{
      grabber_get_xxx_dummy(){
        grabber_get_null<icl::format>();
        grabber_get_null<icl::depth>();
        grabber_get_null<icl::Size>();
      }
    };
  }
  template <class T> class GrabberHandle;
  class GenericGrabber;
  /** \endcond */
  
 /// Common interface class for all grabbers \ingroup GRABBER_G
  /** The Grabber is ICL's common interface for image acquisition
      tools. A large set of Grabbers is available and wrapped
      by the GenericGrabber class. We strongly recommend to 
      use the GenericGrabber class for image acquisition within
      applications.
      
      The Grabber itself has a very short interface for the user:
      usually, a grabber is instantiated and its grab() method is
      called to aquire the next available image.
      
      
      \section DES Desired parameters

      In addition, the Grabber supports a set of so called 
      'desired-parameters'. These can be set to overwrite the
      image parameters that are used by the underlying implementation.
      A FileGrabber e.g. will by default return images that have
      the same parameter that the grabbed image file provides. However,
      in some situations, the user might want to adapt these parameters
      E.g. if the image parameters that are provided by the grabber
      are not suitable for an algorithm. If this is the case, the
      Grabber's desired parameters can be set using the
      Grabber::setDesired-template.\n
      Currently, the image parameters 'depth', 'size' and 'format'
      can be adapted seperately by setting desired parameters. Once
      desired parameters are set, the can be reset to the grabber's
      default by calling grabber::ignoreDesired<T> where one of the
      types icl::depth, icl::format or icl::Size is used as type T.
      
      
      \section UND Image Undistortion
      
      The Grabber does also provide an interface to set up 
      image undistortion parameters. The can be estimated
      with ICL's distortion calibration tool. The undistortion
      operation is accelerated using an internal warp-table.
      By these means, image undistion is directly applied on the
      grabbed images, which lets the user then work with 
      undistored images.

      
      \section IM Implementing Grabbers
      
      In order to implement a new Grabber class, some steps are necessary.
      First, the new Grabber needs to be implemented. This must
      implement the Grabber::acquireImage method, that uses an underlying
      image source to acquire a single new image. This can have any
      parameters and depth (usually, the image parameters are somehow
      related to the output of the underlying image source).
      If the grabber is available, one should think about adapting
      the grabber to inherit the icl::GrabberHandle class that adds
      the ability of instantiating one Grabber several times without
      having to handle double device accesses explicitly.
      
      
      \section PROP Properties
      
      The Grabber implements the Configurable interface that is used
      to implement dynamically settable properties. Each Grabber
      must have at least the two properties 'format' and 'size'. These
      are handled in a special way by the automatically created Grabber-
      property-GUIs available in the ICLQt package.


      \section CB Callback Based Image Akquisition
      
      As a very new experimental features, ICL's Grabber interface provides
      methods to register callback functions to the grabber that are 
      then called automatically whenever a new image is available. This
      feature needs to be implemented explicitly for each grabber backend and
      does sometimes not even make sense. Furthermore, it' could lead to 
      some strange behaviour of the whole application, because the internal
      image akquisition process is suddenly linked to the further image
      processing steps directly. This feature should not be used for 
      writing applications that are scheduled by the speed of the internal
      image aquisition loop. Therefore, images should never be processed
      in the callback functions that are registred.
      
      So far only a few grabbers provide this feature at all. If it
      is not provided, the registered callbacks will never be called.
  */
  class Grabber : public Uncopyable{
    /// internal data class
    struct Data;
    
    /// hidden data
    Data *data;

    protected:
    /// internally set a desired format
    virtual void setDesiredFormatInternal(format fmt);

    /// internally set a desired format
    virtual void setDesiredSizeInternal(const Size &size);

    /// internally set a desired format
    virtual void setDesiredDepthInternal(depth d);

    /// returns the desired format
    virtual format getDesiredFormatInternal() const;

    /// returns the desired format
    virtual depth getDesiredDepthInternal() const;

    /// returns the desired format
    virtual Size getDesiredSizeInternal() const;

    public:

    /// grant private method access to the grabber handle template
    template<class X> friend class GrabberHandle;

    /// grant private method access to the GenericGrabber class
    friend class GenericGrabber;
    
    ///
    Grabber();
    
    /// Destructor
    virtual ~Grabber();

    /// grab function calls the Grabber-specific acquireImage-method and applies distortion if necessary
    /** If dst is not NULL, it is exploited and filled with image data **/
    const ImgBase *grab(ImgBase **dst=0);
    
    /// returns whether the desired parameter for the given type is used
    /** This method is only available for the type icl::depth,icl::Size and icl::format*/
    template<class T>
    bool desiredUsed() const{ return false; }

    /// sets desired parameters (only available for depth,Size and format)
    template<class T>
    void useDesired(const T &t){ (void)t;}
    
    /// sets up the grabber to use all given desired parameters
    void useDesired(depth d, const Size &size, format fmt);
    
    /// set the grabber to ignore the desired param of type T
    /** This method is only available for depth,Size and format */
    template<class T>
    void ignoreDesired() { 
      useDesired<T>(grabber_get_null<T>());
    }

    /// sets up the grabber to ignore all desired parameters
    void ignoreDesired();

    /// returns the desired value for the given type T
    /** This method is only available for depth,Size and format */
    template<class T>
    T getDesired() const { return T(); }
     
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
     
     /// @{ @name distortion functions
     
     /// enables the undistorion
     void enableUndistortion(const std::string &filename);
     
     ///enables the undistortion plugin for the grabber using radial and tangential distortion parameters
     void enableUndistortion(const ImageUndistortion &udist);

     /// enables undistortion from given programm argument. 
     /** where first argument is the filename of the xml file and second is the size of picture*/
     void enableUndistortion(const ProgArg &pa);

     /// enables undistortion for given warp map
     void enableUndistortion(const Img32f &warpMap);
     
     /// sets how undistortion is interpolated (supported modes are interpolateNN and interpolateLIN)
     /** Please note, that this method has no effect if the undistortion was not enabled before
         using one of the Grabber::enableUndistortion methods. Furthermore, the setting is lost
         if the undistortion is deactivated using Grabber::disableUndistortion */
     void setUndistortionInterpolationMode(scalemode mode);
     
     /// disables distortion
     void disableUndistortion();
     
     /// returns whether distortion is currently enabled
     bool isUndistortionEnabled() const;
     
     /// returns the internal warp map or NULL if undistortion is not enabled
     const Img32f *getUndistortionWarpMap() const;
     /// @}

     /// new image callback type
     typedef Function<void,const ImgBase*> callback;
     
     /// registers a callback that is called each time, a new image is available
     /** This feature must not be implemented by specific grabber implementations. And
         it is up to the implementation whether the image that is passed to the
         callback has the "desired parameters" or not. Most likely, an internal
         image buffer is passed, which does not have the desired paremters. The output image
         is also usually not undistorted. */
     virtual void registerCallback(callback cb);
     
     /// removes all registered image callbacks
     virtual void removeAllCallbacks();
     
     /// this function can be implemented by subclasses in order to notify, that a new image is available
     /** When this function is called, it will automatically call all callbacks with the given image. */
     virtual void notifyNewImageAvailable(const ImgBase *image);
    protected:


     /// main interface method, that is implemented by the actual grabber instances
     /** This method is defined in the grabber implementation. It acquires a new image
         using the grabbers specific image acquisition back-end */
     virtual const ImgBase *acquireImage() = 0;

     /// Utility function that allows for much easier implementation of grabUD
     /** called by the grabbers grab() method **/
     const ImgBase *adaptGrabResult(const ImgBase *src, ImgBase **dst); 

     /// internally used by the load- and saveProperties
     /** If any property shall not be save or loaded from configuration file, it must be filtered out by this f*/
     virtual std::vector<std::string> get_io_property_list() { return getPropertyList(); }
  }; 
  
  /** \cond */
  template<> inline void Grabber::useDesired<format>(const format &t) { setDesiredFormatInternal(t); }
  template<> inline void Grabber::useDesired<depth>(const depth &t) { setDesiredDepthInternal(t); }
  template<> inline void Grabber::useDesired<Size>(const Size &t) { setDesiredSizeInternal(t); }

  template<> inline depth Grabber::getDesired<depth>() const { return getDesiredDepthInternal(); }
  template<> inline Size Grabber::getDesired<Size>() const { return getDesiredSizeInternal(); }
  template<> inline format Grabber::getDesired<format>() const { return getDesiredFormatInternal(); }

  template<> inline bool Grabber::desiredUsed<format>() const{ return (int)getDesired<format>() != -1; }
  template<> inline bool Grabber::desiredUsed<depth>() const{ return (int)getDesired<depth>() != -1; }
  template<> inline bool Grabber::desiredUsed<Size>() const{ return getDesired<Size>() != Size::null; }

  /** \endcond */
 


} // namespace icl

#endif
