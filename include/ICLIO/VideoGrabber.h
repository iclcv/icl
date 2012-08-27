/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2012 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : include/ICLIO/VideoGrabber.h                           **
** Module : ICLIO                                                  **
** Authors: Christof Elbrechter                                    **
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

#ifndef ICL_VIDEO_GRABBER_H
#define ICL_VIDEO_GRABBER_H

#include <ICLIO/Grabber.h>
#include <ICLUtils/Exception.h>

namespace icl{
  namespace io{
    
    /// Xine-based Video Grabber (grabs most common image formats) \ingroup MOVIE_FILE_G
    /** The VideoGrabber implementation differs somehow from other Grabber implementations, as 
        video playback is heavily constrained by the current video framerate. The
        VideoGrabber's property interface provides functions to set up the grabbers video playback
        speed.
        Internally video frames are obtained successively (using xine's 'experimental feature: 
        framegrab_video_port').
        
        Currently, drop-frames are not allowed/possible, so if the grabber is too slow to reach the
        desired Video-framerate, playback speed is decreased implicitly 
        
        \section KI Known Issues
        The video grabber does not really work very stable. 
        * In particular when linked against
          the default xine build shipped with ubuntu, you will most likely get some seg-faults due
          to a known xine bug.
        * Video streams that have a stream-size whose width is not a multiple of 8, will not be
          displayed correctly due to some undocumented data allignment issues.
        * Some formats just cause seg-faults within 3rd party libs.
        * seeking using setProperty("stream-pos") does not work robustly, however 
          it does work in the icl-video-player application
        
        \section ALT Alternative
        Please note, that the OpenCVVideoGrabber might be an alternative for you
     */
    class VideoGrabber : public Grabber{
      public:
      
      /// Create video grabber with given video-file name
      VideoGrabber(const std::string &fileName) throw (FileNotFoundException,InvalidFileException);
      
      /// Destructor
      ~VideoGrabber();
  
      /// grab function
      virtual const ImgBase *acquireImage();
      
      /// direct access to pause video playback (grab will block then)
      void pause();
  
      /// direct access to unpause video playback
      void unpause();
      
      /// direct access to restart video playback from the first frame
      void restart();
  
      virtual void setProperty(const std::string &property, const std::string &value);
      
      /// returns a list of properties, that can be set using setProperty
      /** currently:
          - speed-mode with values 
            - auto : which adapts grabbing speed to the video stream frame-rate
            - manual: which lets the user adapt the speed from 1/10 to 10x of the
                      normal speed
            - unlimited: which makes the grabber just grab the next frame without
                         regarding any framerate constraints
          - speed which is only used if speed-mode has current value 'manual'. Then
            the speed is a value from 0 to 100, where values below 50 slow down the
            video-playback (mapped with a linear function from 0: 1/10 speed to 50: 
            normal speed) and values above 50 increase the video playback (100 means
            10x speed here)
          - stream-pos : gets an internal 16bit position value (from 0 to 65535) and 
            allows the user to have random access (seeking) to single frames within the
            video stream
          - stream-length is just an 'info' typed variable which will be shown in
            an appropriate GUI 
          - volume sets up audio volume to a value from (0: mute to 100:max volume)
      */
      virtual std::vector<std::string> getPropertyList();
  
      /// get type of property 
      virtual std::string getType(const std::string &name);
      
      /// get information of a properties valid values
      virtual std::string getInfo(const std::string &name);
  
  
      /// returns the current value of a property or a parameter
      virtual std::string getValue(const std::string &name);
      
      /// here only the 'stream-pos' is volatile
      virtual int isVolatile(const std::string &propertyName);
      
      
      protected:
      /** \cond */
      struct Data;
      struct XineHandle;
      struct Params;
      /** \endcond */
      
      XineHandle *m_xine; //!< internal data belonging to the xine library
      Data *m_data;       //!< internal data
      Params *m_params;   //!< property values 
    };
  
  } // namespace io
}

#endif
