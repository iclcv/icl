/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLIO/src/ICLIO/XiGrabber.h                            **
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

    /// Grabber class that grabs images using the XiAPI (extension of the M3API)
    /** The XiGrabber can be used e.g. for cameras from Ximea. Use device type 'xi'
        with the generic grabber for this.
    */
    class ICLIO_API XiGrabber : public Grabber {
        /// Internal Data storage class
        struct Data;

        /// Hidden Data container
        Data *m_data;

        /// internal initialization function
        void init(int deviceID) throw (utils::ICLException);

        /// provide protected access for the data class
        friend class Data;
      public:

        /// Creates a new XiGrabber instance (please use the GenericGrabber instead)
        XiGrabber(int deviceID) throw(utils::ICLException);

        /// Destructor
        ~XiGrabber();

        /// returns a list of all connected devices
        static const std::vector<GrabberDeviceDescription> &getDeviceList(std::string hint, bool rescan);

        /// grabbing function
        /** \copydoc icl::io::Grabber::grab(core::ImgBase**)  **/
        virtual const core::ImgBase* acquireImage();

        /// callback for changed configurable properties
        void processPropertyChange(const utils::Configurable::Property &prop);
    };

  } // namespace io
}

