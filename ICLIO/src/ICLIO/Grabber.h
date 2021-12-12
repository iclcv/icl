/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLIO/src/ICLIO/Grabber.h                              **
** Module : ICLIO                                                  **
** Authors: Christof Elbrechter, Michael Goetting, Robert          **
**          Haschke, Viktor Richter                                **
**                                                                 **
**                                                                 **
** GNU LESSER GENERAL PUBLIC LICENSE                               **
** This file may be used under the terms of the GNU Lesser General **
** Public License version 3.0 as published by the                  **
**                                                                 **
** Free Software Foundation and appearing in the file LICENSE.LGPL **
** included in the packaging of this file.  Please review the      **
** following information to ensure the license requirements will   **
** be met: http://www.gnu.org/licenses/lgpl-3.0.txt                **
**                                                                 **
** The development of this software was supported by the           **
** Excellence Cluster EXC 277 Cognitive Interaction Technology.    **
** The Excellence Cluster EXC 277 is a grant of the Deutsche       **
** Forschungsgemeinschaft (DFG) in the context of the German       **
** Excellence Initiative.                                          **
**                                                                 **
********************************************************************/

#pragma once

#include <ICLUtils/CompatMacros.h>
#include <ICLUtils/SteppingRange.h>
#include <ICLUtils/Function.h>
#include <ICLUtils/Uncopyable.h>
#include <ICLUtils/Configurable.h>
#include <ICLUtils/ProgArg.h>
#include <ICLUtils/Uncopyable.h>
#include <ICLCore/ImgBase.h>
#include <ICLIO/GrabberDeviceDescription.h>
#include <ICLIO/ImageUndistortion.h>

#include <string>
#include <vector>
#include <set>

namespace icl {
  namespace io{

    /** \cond */
    namespace{
      template <class T> inline T grabber_get_null(){ return 0; }
      template <> inline core::format grabber_get_null<core::format>(){ return (core::format)-1; }
      template <> inline core::depth grabber_get_null<core::depth>(){ return (core::depth)-1; }
      template <> inline icl::utils::Size grabber_get_null<icl::utils::Size>(){ return icl::utils::Size::null; }

      struct grabber_get_xxx_dummy{
          grabber_get_xxx_dummy(){
            grabber_get_null<core::format>();
            grabber_get_null<core::depth>();
            grabber_get_null<icl::utils::Size>();
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
        Currently, the image parameters 'core::depth', 'size' and 'core::format'
        can be adapted seperately by setting desired parameters. Once
        desired parameters are set, the can be reset to the grabber's
        default by calling grabber::ignoreDesired<T> where one of the
        types core::depth, core::format or icl::utils::Size is used as type T.


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
        parameters and core::depth (usually, the image parameters are somehow
        related to the output of the underlying image source).
        If the grabber is available, one should think about adapting
        the grabber to inherit the icl::GrabberHandle class that adds
        the ability of instantiating one Grabber several times without
        having to handle double device accesses explicitly.


        \section PROP Properties

        The Grabber implements the Configurable interface that is used
        to implement dynamically settable properties. Each Grabber
        must have at least the two properties 'core::format' and 'size'. These
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
    class ICLIO_API Grabber : public utils::Uncopyable, public utils::Configurable{
        /// internal data class
        struct Data;

        /// hidden data
        Data *data;

      protected:
        /// internally set a desired format
        virtual void setDesiredFormatInternal(core::format fmt);

        /// internally set a desired format
        virtual void setDesiredSizeInternal(const utils::Size &size);

        /// internally set a desired format
        virtual void setDesiredDepthInternal(core::depth d);

        /// returns the desired format
        virtual core::format getDesiredFormatInternal() const;

        /// returns the desired format
        virtual core::depth getDesiredDepthInternal() const;

        /// returns the desired format
        virtual utils::Size getDesiredSizeInternal() const;

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
        const core::ImgBase *grab(core::ImgBase **dst=0);

        /// returns whether the desired parameter for the given type is used
        /** This method is only available for the type core::depth,icl::utils::Size and core::format*/
        template<class T>
        bool desiredUsed() const{ return false; }

        /// sets desired parameters (only available for core::depth,utils::Size and core::format)
        template<class T>
        void useDesired(const T &t){ (void)t;}

        /// sets up the grabber to use all given desired parameters
        void useDesired(core::depth d, const utils::Size &size, core::format fmt);

        /// set the grabber to ignore the desired param of type T
        /** This method is only available for core::depth,utils::Size and core::format */
        template<class T>
        void ignoreDesired() {
          useDesired<T>(grabber_get_null<T>());
        }

        /// sets up the grabber to ignore all desired parameters
        void ignoreDesired();

        /// returns the desired value for the given type T
        /** This method is only available for core::depth,utils::Size and core::format */
        template<class T>
        T getDesired() const { return T(); }

        /// @}
        /// @{ @name static string conversion functions

        /// translates a SteppingRange into a string representation
        static std::string translateSteppingRange(const utils::SteppingRange<double>& range);

        /// creates a SteppingRange out of a string representation
        static utils::SteppingRange<double> translateSteppingRange(const std::string &rangeStr);

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

        /// enables the undistortion plugin for the grabber using radial and tangential distortion parameters
        void enableUndistortion(const ImageUndistortion &udist);

        /// enables undistortion from given programm argument.
        /** where first argument is the filename of the xml file and second is the size of picture*/
        void enableUndistortion(const utils::ProgArg &pa);

        /// enables undistortion for given warp map
        void enableUndistortion(const core::Img32f &warpMap);

        /// sets how undistortion is interpolated (supported modes are interpolateNN and interpolateLIN)
        /** Please note, that this method has no effect if the undistortion was not enabled before
           using one of the Grabber::enableUndistortion methods. Furthermore, the setting is lost
           if the undistortion is deactivated using Grabber::disableUndistortion */
        void setUndistortionInterpolationMode(core::scalemode mode);

        /// disables distortion
        void disableUndistortion();

        /// returns whether distortion is currently enabled
        bool isUndistortionEnabled() const;

        /// returns the internal warp map or NULL if undistortion is not enabled
        const core::Img32f *getUndistortionWarpMap() const;
        /// @}

        /// new image callback type
        typedef utils::Function<void,const core::ImgBase*> callback;

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
        virtual void notifyNewImageAvailable(const core::ImgBase *image);

      protected:


        /// main interface method, that is implemented by the actual grabber instances
        /** This method is defined in the grabber implementation. It acquires a new image
           using the grabbers specific image acquisition back-end */
        virtual const core::ImgBase *acquireImage(){ return NULL; }

        /// Utility function that allows for much easier implementation of grabUD
        /** called by the grabbers grab() method **/
        const core::ImgBase *adaptGrabResult(const core::ImgBase *src, core::ImgBase **dst);

      private:
        /// callback for changed configurable properties
        void processPropertyChange(const utils::Configurable::Property &prop);

    };

    /** \cond */
    template<> inline void Grabber::useDesired<core::format>(const core::format &t) { setDesiredFormatInternal(t); }
    template<> inline void Grabber::useDesired<core::depth>(const core::depth &t) { setDesiredDepthInternal(t); }
    template<> inline void Grabber::useDesired<utils::Size>(const utils::Size &t) { setDesiredSizeInternal(t); }

    template<> inline core::depth Grabber::getDesired<core::depth>() const { return getDesiredDepthInternal(); }
    template<> inline utils::Size Grabber::getDesired<utils::Size>() const { return getDesiredSizeInternal(); }
    template<> inline core::format Grabber::getDesired<core::format>() const { return getDesiredFormatInternal(); }

    template<> inline bool Grabber::desiredUsed<core::format>() const{ return (int)getDesired<core::format>() != -1; }
    template<> inline bool Grabber::desiredUsed<core::depth>() const{ return (int)getDesired<core::depth>() != -1; }
    template<> inline bool Grabber::desiredUsed<utils::Size>() const{ return getDesired<utils::Size>() != utils::Size::null; }

    class ICLIO_API GrabberRegister : utils::Uncopyable {
      private:
        utils::Mutex mutex;

        struct GrabberFunctions{
          utils::Function<Grabber*,const std::string&> init;
          utils::Function<const std::vector<GrabberDeviceDescription> &,std::string,bool> list;
        };

        // grabber functions map
        typedef std::map<std::string, GrabberFunctions> GFM;
        GFM gfm;

        // grabber bus reset functions map
        typedef std::map<std::string, utils::Function<void,bool> > GBRM;
        GBRM gbrm;

        // grabber device descriptions map
        typedef std::set<std::string> GDS;
        GDS gds;

        // private constructor
        GrabberRegister(){}

      public:
        static GrabberRegister* getInstance();

        void registerGrabberType(const std::string &grabberid,
                                   utils::Function<Grabber *, const std::string &> creator,
                                   utils::Function<const std::vector<GrabberDeviceDescription> &,std::string,bool> device_list);

        void registerGrabberBusReset(const std::string &grabberid,
                                   utils::Function<void, bool> reset_function);

        void addGrabberDescription(const std::string &grabber_description);

        Grabber* createGrabber(const std::string &grabberid, const std::string &param);

        std::vector<std::string> getRegisteredGrabbers();

        std::vector<std::string> getGrabberInfos();

        const std::vector<GrabberDeviceDescription>& getDeviceList(std::string id, std::string hint="", bool rescan=true);

        void resetGrabberBus(const std::string &id, bool verbose);
    };

    /** \endcond */

    /// registration macro for grabbers
    /** @see \ref REG */
    #define REGISTER_GRABBER(NAME,CREATE_FUNC,DEVICE_LIST_FUNC,DESCRIPTION)                                        \
    struct StaticGrabberRegistrationFor_##NAME{                                                                    \
      StaticGrabberRegistrationFor_##NAME(){                                                                       \
        icl::io::GrabberRegister::getInstance() -> registerGrabberType(#NAME, CREATE_FUNC, DEVICE_LIST_FUNC);      \
        icl::io::GrabberRegister::getInstance() -> addGrabberDescription(DESCRIPTION);                             \
      }                                                                                                            \
    } staticGrabberRegistrationFor_##NAME;

    #define REGISTER_GRABBER_BUS_RESET_FUNCTION(NAME,BUS_RESET_FUNC)                                               \
    struct StaticGrabberBusResetRegistrationFor_##NAME{                                                            \
      StaticGrabberBusResetRegistrationFor_##NAME(){                                                               \
        icl::io::GrabberRegister::getInstance() -> registerGrabberBusReset(#NAME, BUS_RESET_FUNC);                 \
      }                                                                                                            \
    } staticGrabberBusResetRegistrationFor_##NAME;

  } // namespace io
} // namespace icl
