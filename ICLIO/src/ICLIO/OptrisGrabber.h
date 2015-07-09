/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLIO/src/ICLIO/OptrisGrabber.h                        **
** Module : ICLIO                                                  **
** Authors: Christof Elbrechter                                    **
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
#include <ICLIO/Grabber.h>

namespace icl{
  namespace io{
    
    /// Grabber class that grabs images using the libImager library from Optris
    /** Optris provides IR-Cameras, such as the TIM 160 which yields IR-temperature
        images of 160x120 resolution at 120 Hz.
    */
    class ICLIO_API OptrisGrabber : public Grabber {
        /// Internal Data storage class
        struct Data;

        /// Hidden Data container
        Data *m_data;

        /// provide protected access for the data class
        friend class Data;
      public:

        /// Creates a new OptrisGrabber instance (please use the GenericGrabber instead)
        OptrisGrabber(const std::string &serialPattern, bool testOnly=false) throw(utils::ICLException);

        /// Destructor
        ~OptrisGrabber();

        /// returns a list of all available devices
        static const std::vector<GrabberDeviceDescription> &getDeviceList(std::string hint, bool rescan);

        /// grabbing function
        /** \copydoc icl::io::Grabber::grab(core::ImgBase**)  **/
        virtual const core::ImgBase* acquireImage();

        /// callback for changed configurable properties
        void processPropertyChange(const utils::Configurable::Property &prop);
    };
    
  } // namespace io
}

