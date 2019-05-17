/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLIO/src/ICLIO/SwissRangerGrabber.h                   **
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

#include <ICLIO/Grabber.h>
#include <ICLUtils/Exception.h>
#include <ICLUtils/Mutex.h>

namespace icl{
  namespace io{


    /// Grabber-Implementation for the SwissRanger time-of-flight camera using the libMesaSR library \ingroup GRABBER_G
    class SwissRangerGrabber : public Grabber{
      public:
        /// Internally used data-class
        class SwissRanger;

        /// Create interface to device with given serial number:
        /** @param serialNumber if 0 -> automatic select\n
                              if < 0 open selection dialog (windows: gui, linux: shell input)
                              if > 0 specify serial number of device
         @param bufferDepth
         @param pickChannel
      */
        ICLIO_API SwissRangerGrabber(int serialNumber=0,
                               core::depth bufferDepth=core::depth32f,
                               int pickChannel=-1)
        ;

        /// Destructor
        ICLIO_API ~SwissRangerGrabber();

        /// returns a list of all found devices
        ICLIO_API static const std::vector<GrabberDeviceDescription> &getDeviceList(std::string hint, bool rescan);

        /// grab an undistorted image
        ICLIO_API const core::ImgBase *acquireImage();

        /// Internally used utility function, that might be interesting elsewhere
        ICLIO_API static float getMaxRangeMM(const std::string &modulationFreq) ;

        /// adds properties to Configurable
        ICLIO_API void addProperties();

        /// callback for changed configurable properties
        ICLIO_API void processPropertyChange(const utils::Configurable::Property &prop);

      private:
        /// utility function
        float getMaxRangeVal() const;

        /// Internal data
        SwissRanger *m_sr;

        /// Internally used mutex locks grabbing and setting of properties
        utils::Mutex m_mutex;
    };

  } // namespace io
}

