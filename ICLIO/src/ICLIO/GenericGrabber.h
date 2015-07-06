/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLIO/src/ICLIO/GenericGrabber.h                       **
** Module : ICLIO                                                  **
** Authors: Christof Elbrechter, Viktor Richter                    **
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

#include <ICLUtils/Macros.h>
#include <ICLUtils/Exception.h>
#include <ICLUtils/Lockable.h>
#include <ICLUtils/ProgArg.h>
#include <ICLUtils/ConfigurableProxy.h>
#include <ICLIO/Grabber.h>
#include <string>

namespace icl {
  namespace io{

    /** \cond */
    class ConfigurableRemoteServer;
    /** \endcond */
    
    /// Common interface class for all grabbers \ingroup GRABBER_G
    /** The generic grabber provides an interface for a multi-platform
        compatible grabber.
        Image processing applications should use this Grabber
        class. The GenericGrabber also provides camera
        configuration via ConfigurableProxy interface.
    */
    class ICLIO_API GenericGrabber : public utils::Uncopyable, public utils::ConfigurableProxy{

        Grabber *m_poGrabber; //!< internally wrapped grabber instance

        GrabberDeviceDescription m_poDesc; //!< description of current Grabber

        mutable utils::Mutex m_mutex; //! << internal protection for re-initialization

        ConfigurableRemoteServer *m_remoteServer;

      public:

        /// Initialized the grabber from given prog-arg
        /** The progarg needs two sub-parameters */
        GenericGrabber(const utils::ProgArg &pa) throw (utils::ICLException):m_poGrabber(0),m_remoteServer(0){
          init(pa);
        }

        /// Create a generic grabber instance with given device priority list
        /** internally this function calls the init function immediately*/
        GenericGrabber(const std::string &devicePriorityList,
                       const std::string &params,
                       bool notifyErrors = true) throw (utils::ICLException):m_poGrabber(0),m_remoteServer(0){
          init(devicePriorityList,params,notifyErrors);
        }


        /// Empty default constructor, which creates a null-instance
        /** null instances of grabbers can be adapted using the init-function*/
      GenericGrabber():m_poGrabber(0),m_remoteServer(0){}

        /// initialization function to change/initialize the grabber back-end
        /** @param devicePriorityList Comma separated list of device tokens (no white spaces).
                                    something like "dc,pwc,file,unicap" with arbitrary order
                                    undesired devices can be left out. In particular you can
                                    also give only a single desired device type e.g. "pwc".
                                    The following device types are supported:
                                    - <b>v4l</b> Video for Linux 2 based grabber
                                    - <b>dc</b> dc grabber
                                    - <b>dc800</b> dc grabber but with 800MBit iso-speed
                                    - <b>file</b> file grabber
                                    - <b>demo</b> demo grabber (moving red spot)
                                    - <b>create</b> create grabber (create an image using ICL's create function)
                                    - <b>sr</b> SwissRanger camera (mesa-imaging)
                                    - <b>video</b> Xine based video grabber (grabbing videos frame by frame)
                                    - <b>cvcam</b> OpenCV based camera grabber (supporting video 4 linux devices)
                                    - <b>cvvideo</b> OpenCV based video grabber
                                    - <b>sm</b> Qt-based Shared-Memory grabber (using QSharedMemoryInstance)
                                    - <b>myr</b> Uses Myrmex tactile input device as image source
                                    - <b>kinectd</b> Uses libfreenect to grab Microsoft-Kinect's core::depth images
                                    - <b>kinectc</b> Uses libfreenect to grab Microsoft-Kinect's rgb color images
                                    - <b>kinecti</b> Uses libfreenect to grab Microsoft-Kinect's IR images
                                    - <b>rsb</b> Robotics Service Bus Source
                                    - <b>optris</b> For Optris' IR-Cameras



          @param params comma separated device depend parameter list: e.g.
                                    "v4l=0,file=images//image*.ppm,dc=0" with self-explaining syntax\n
                                    Additionally, each token a=b can be extended by device property that are directly
                                    set after device instantiation. E.g. demo=0\@size=QVGA\@blob-red=128, instantiates
                                    a demo-grabber, where the two additionally given properties (size and blob-red)
                                    are set immediately after grabber instantiation. By these means particularly a
                                    grabber's core::format can be set in the grabber instantiation call. Furthermore, three
                                    special \@-tokens are possible: \@info (e.g. dc=0\@info) lists the 0th dc device's
                                    available properties. \@load=filename loads a given property filename directly.
            \@udist=filename loads a given undistortion parameter filename directly and therefore
                                    makes the grabber grab undistorted images according to the undistortion parameters
                                    and model type (either 3 or 5 parameters) that is found in the given xml-file.
                                    <b>todo fix this sentence according to the fixed application names</b>
                                    Please note, that valid xml-undistortion files can be created using the
                                    undistortion-calibration tools icl-opencvcamcalib-demo,
                                    icl-intrinsic-camera-calibration and icl-intrinsic-calibrator-demo.
                                    On the C++-level, this is only a minor advantage, since all these things can
                                    also be achieved via function calls, however if you use the most recommended way
                                    for ICL-Grabber instantiation using ICL's program-argument evaluation framework,
                                    The GenericGrabber is instantiated using grabber.init(pa("-i")) which then allows
                                    the application user to set grabber parameters via addiation \@-options on the
                                    command line: e.g.: "icl-camviewer -input dc 0\@size=VGA"

                                    Semantics:\n
                                    - v4l=device-name (e.g. "/dev/video0")
                                    - dc=device-index (int) or dc=UniqueID (string)
                                      (the unique ID can be found with 'icl-cam-cfg d -list-devices-only')
                                    - dc800=device-index (int)
                                    - file=pattern (string)
                                    - demo=anything (not regarded)
                                    - create=image name (see also icl::TestImages::create)
                                    - mv=device-name (string)
                                    - sr=device-serial-number (-1 -> menu, 0 -> auto-select)
                                      <b>or</b>
                                      sr=NcC where N is the device numer as above, c is the character 'c' and C is
                                      the channel index to pick (0: core::depth-map, 1: confidence map, 2: intensity image
                                    - video=video-filename (string)
                                    - cvcam=camera index (0=first device,1=2nd device, ...) here, you can also use
                                      opencv's so called 'domain offsets': current values are:
                                      - 100 MIL-drivers (proprietary)
                                      - 200 V4L,V4L2 and VFW,
                                      - 300 Firewire,
                                      - 400 TXYZ (proprietary)
                                      - 500 QuickTime
                                      - 600 Unicap
                                      - 700 Direct Show Video Input
                                      (e.g. device ID 301 selects the 2nd firewire device)
                                    - cvvideo=video-filename (string)
                                    - sm=Shared-memory-segment-id (string)
                                    - myr=deviceIndex (int) (the device index is used to create the /dev/videoX device)
                                    - kinectd=device-index (int)
                                    - kinectc=device-index (int)
                                    - kinecti=device-index (int)
                                    - rsb=[comma-sep. transport-list=spread]:scope)
                                    - optris=camera-serial

          @param notifyErrors if set to false, no exception is thrown if no suitable device was found
      **/
        void init(const std::string &devicePriorityList,
                  const std::string &params,
                  bool notifyErrors = true) throw (utils::ICLException);

        /// this method works just like the other init method
        void init(const utils::ProgArg &pa) throw (utils::ICLException);

        /// resets resource on given devices (e.g. firewire bus)
        static void resetBus(const std::string &deviceList="dc", bool verbose=false);

        /// return the actual grabber type
        std::string getType() const {
          utils::Mutex::Locker __lock(m_mutex);
          return m_poDesc.type;
        }

        /// returns the wrapped grabber itself
        Grabber *getGrabber() const {
          utils::Mutex::Locker __lock(m_mutex);
          return m_poGrabber;
        }

        /// Destructor
        virtual ~GenericGrabber();

        /// grab function calls the Grabber-specific acquireImage-method and applies distortion if necessary
        /** If dst is not NULL, it is exploited and filled with image data **/
        const core::ImgBase *grab(core::ImgBase **dst = 0){
          utils::Mutex::Locker __lock(m_mutex);
          ICLASSERT_RETURN_VAL(!isNull(), 0);

          return m_poGrabber->grab(dst);
        }

        /// returns wheter an underlying grabber could be created
        bool isNull() const { return m_poGrabber == 0; }

        /// simpler interface for isNull() (returns !isNull()
        operator bool() const { return !isNull(); }


        /// internally set a desired format
        void setDesiredFormatInternal(core::format fmt){
          ICLASSERT_RETURN(!isNull());
          utils::Mutex::Locker l(m_mutex);
          m_poGrabber->setDesiredFormatInternal(fmt);
        }

        /// internally set a desired format
        void setDesiredSizeInternal(const utils::Size &size){
          ICLASSERT_RETURN(!isNull());
          utils::Mutex::Locker l(m_mutex);
          m_poGrabber->setDesiredSizeInternal(size);
        }

        /// internally set a desired format
        void setDesiredDepthInternal(core::depth d){
          ICLASSERT_RETURN(!isNull());
          utils::Mutex::Locker l(m_mutex);
          m_poGrabber->setDesiredDepthInternal(d);
        }

        /// returns the desired format
        core::format getDesiredFormatInternal() const{
          ICLASSERT_RETURN_VAL(!isNull(),(core::format)-1);
          utils::Mutex::Locker l(m_mutex);
          return m_poGrabber->getDesiredFormatInternal();
        }

        /// returns the desired format
        core::depth getDesiredDepthInternal() const{
          ICLASSERT_RETURN_VAL(!isNull(),(core::depth)-1);
          utils::Mutex::Locker l(m_mutex);
          return m_poGrabber->getDesiredDepthInternal();
        }

        /// returns the desired format
        utils::Size getDesiredSizeInternal() const{
          ICLASSERT_RETURN_VAL(!isNull(),utils::Size::null);
          utils::Mutex::Locker l(m_mutex);
          return m_poGrabber->getDesiredSizeInternal();
        }

        /// passes registered callback to the internal pointer
        void registerCallback(Grabber::callback cb){
          ICLASSERT_RETURN(!isNull());
          utils::Mutex::Locker l(m_mutex);
          m_poGrabber->registerCallback(cb);
        }

        /// passes registered callback to the internal pointer
        void removeAllCallbacks(){
          ICLASSERT_RETURN(!isNull());
          utils::Mutex::Locker l(m_mutex);
          m_poGrabber->removeAllCallbacks();
        }

        /// returns whether the desired parameter for the given type is used
        /** This method is only available for the type core::depth,icl::utils::Size and core::format*/
        template<class T>
        bool desiredUsed() const{
          ICLASSERT_RETURN_VAL(!isNull(),false);
          utils::Mutex::Locker l(m_mutex);
          return m_poGrabber->desiredUsed<T>();
        }

        /// sets desired parameters (only available for core::depth,utils::Size and core::format)
        template<class T>
        void useDesired(const T &t){
          ICLASSERT_RETURN(!isNull());
          utils::Mutex::Locker l(m_mutex);
          m_poGrabber->useDesired<T>(t);
        }

        /// sets up the grabber to use all given desired parameters
        void useDesired(core::depth d, const utils::Size &size, core::format fmt){
          ICLASSERT_RETURN(!isNull());
          utils::Mutex::Locker l(m_mutex);
          m_poGrabber->useDesired(d, size, fmt);
        }

        /// set the grabber to ignore the desired param of type T
        /** This method is only available for core::depth,utils::Size and core::format */
        template<class T>
        void ignoreDesired() {
          ICLASSERT_RETURN(!isNull());
          utils::Mutex::Locker l(m_mutex);
          m_poGrabber->ignoreDesired<T>();
        }

        /// sets up the grabber to ignore all desired parameters
        void ignoreDesired(){
          ICLASSERT_RETURN(!isNull());
          utils::Mutex::Locker l(m_mutex);
          m_poGrabber->ignoreDesired();
        }

        /// returns the desired value for the given type T
        /** This method is only available for core::depth,utils::Size and core::format */
        template<class T>
        T getDesired() const {
          ICLASSERT_RETURN_VAL(!isNull(), T());
          utils::Mutex::Locker l(m_mutex);
          return m_poGrabber->getDesired<T>();
        }

        /// enables the undistorion
        void enableUndistortion(const std::string &filename){
          ICLASSERT_RETURN(!isNull());
          utils::Mutex::Locker l(m_mutex);
          m_poGrabber->enableUndistortion(filename);
        }

        /// enables the undistortion plugin for the grabber using radial and tangential distortion parameters
        void enableUndistortion(const ImageUndistortion &udist){
          ICLASSERT_RETURN(!isNull());
          utils::Mutex::Locker l(m_mutex);
          m_poGrabber->enableUndistortion(udist);
        }

        /// enables undistortion from given programm argument.
        /** where first argument is the filename of the xml file and second is the size of picture*/
        void enableUndistortion(const utils::ProgArg &pa){
          ICLASSERT_RETURN(!isNull());
          utils::Mutex::Locker l(m_mutex);
          m_poGrabber->enableUndistortion(pa);
        }

        /// enables undistortion for given warp map
        void enableUndistortion(const core::Img32f &warpMap){
          ICLASSERT_RETURN(!isNull());
          utils::Mutex::Locker l(m_mutex);
          m_poGrabber->enableUndistortion(warpMap);
        }

        /// sets how undistortion is interpolated (supported modes are interpolateNN and interpolateLIN)
        /** Please note, that this method has no effect if the undistortion was not enabled before
           using one of the Grabber::enableUndistortion methods. Furthermore, the setting is lost
           if the undistortion is deactivated using Grabber::disableUndistortion */
        void setUndistortionInterpolationMode(core::scalemode mode){
          ICLASSERT_RETURN(!isNull());
          utils::Mutex::Locker l(m_mutex);
          m_poGrabber->setUndistortionInterpolationMode(mode);
        }

        /// disables distortion
        void disableUndistortion(){
          ICLASSERT_RETURN(!isNull());
          utils::Mutex::Locker l(m_mutex);
          m_poGrabber->disableUndistortion();
        }

        /// returns whether distortion is currently enabled
        bool isUndistortionEnabled() const{
          ICLASSERT_RETURN_VAL(!isNull(),false);
          utils::Mutex::Locker l(m_mutex);
          return m_poGrabber->isUndistortionEnabled();
        }

        /// returns the internal warp map or NULL if undistortion is not enabled
        const core::Img32f *getUndistortionWarpMap() const{
          ICLASSERT_RETURN_VAL(!isNull(),0);
          utils::Mutex::Locker l(m_mutex);
          return m_poGrabber->getUndistortionWarpMap();
        }

        /// returns a list of all currently available devices (according to the filter-string)
        /** The filter-string is a comma separated list of single filters like
           <pre> dc=0,unicap </pre>
           If a single token has the core::format deviceType=deviceID, then only not only the
           device type but also a specific ID is used for the filtering operation. If, otherwise,
           a token has the core::format deviceType, then all possible devices for this device type are
           listed.
       */
        static const std::vector<GrabberDeviceDescription> &getDeviceList(const std::string &filter, bool rescan=true);

        /// initializes the grabber from given FoundDevice instance
        /** calls 'init(dev.type,dev.type+"="+dev.id,false)' */
        inline void init(const GrabberDeviceDescription &dev){
          init(dev.type,dev.type+"="+dev.id,false);
        }
    };

  } // namespace io
} 

