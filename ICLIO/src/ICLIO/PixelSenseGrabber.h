/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLIO/src/ICLIO/PixelSenseGrabber.h                    **
** Module : ICLIO                                                  **
** Authors: Eckard Riedenklau, Christof Elbrechter                 **
**          (based on code by Florian Echtler)                     **
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

    /// PixelSense Grabber class
    /** This grabber can be used to interface with devices implementing the
        Microsoft PixelSense technology, such as the Samsung SUR40 device.

        The grabbed images will contain meta data which describes the
        natively detected blobs. To extract the meta-data as a vector of Blobs,
        the static PixelSenseGrabber::extractBlobMetaData method can be used
        */
    class PixelSenseGrabber : public Grabber{
      struct Data;   //!< internal data structure
      Data *m_data;  //!< internal data pointer

      public:

      /// Blob structure
      /** Please note: positions and sizes are given in screen coordinates.
          corresponding image coords can be obtained by deviding by two */
      struct Blob{
        icl16u id;           //!< blob ID
        icl8u  action;       //!< 0x2 enter/exit, 0x3 update?
        icl8u  __unknown;    //!<  always 0x01 or 0x02 (no idea what this is?)
        icl16u bbx;          //!< upper left x of bounding box (in screen coords)
        icl16u bby;          //!< upper left y of bounding box (in screen coords)
        icl16u bbwidth;      //!< bounding box width (in screen coords)
        icl16u bbheight;     //!< bounding box height (in screen coords)
        icl16u posx;         //!< finger tip x-pos (in screen coords)
        icl16u posy;         //!< finger tip y-pos (in screen coords)
        icl16u cx;           //!< x-center (in screen coords);
        icl16u cy;           //!< y-center (in screen coords);
        icl16u axisx;        //!< x-axis (related to first principal axis)
        icl16u axisy;        //!< y-axis (related to 2nd principal axis)
        icl32f angle;        //!< angle of first principal axis
        icl32u area;         //!< size in pixels (correlated to pressure?)
        icl8u  padding[32];  //!< padding bytes (unused)
      };

      /// default grab function
      ICLIO_API virtual const core::ImgBase* acquireImage();

      /// Create a PixelSenseGrabber with given max. fps count
      ICLIO_API PixelSenseGrabber(float maxFPS = 30);

      /// destructor
      ICLIO_API ~PixelSenseGrabber();

      /// this utility method can be used to extract the meta-data of a grabbed image
      ICLIO_API static std::vector<Blob> extractBlobMetaData(const core::ImgBase *image);
    };

    /// overloaded ostream operator for the PixelSenseGrabber::Blob type
    /** concatenates relevant information using a comma delimiter */
    ICLIO_API std::ostream &operator<<(std::ostream &str, const PixelSenseGrabber::Blob &b);

    /// overloaded istream operator for the PixelSenseGrabber::Blob type
    /** relevant information is assumed to be comma delimited */
    ICLIO_API std::istream &operator>>(std::istream &str, PixelSenseGrabber::Blob &b);

  } // namespace io
}

