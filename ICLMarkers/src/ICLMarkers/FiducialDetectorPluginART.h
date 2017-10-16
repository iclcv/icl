/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLMarkers/src/ICLMarkers/FiducialDetectorPluginART.h  **
** Module : ICLMarkers                                             **
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
#include <ICLMarkers/FiducialDetectorPluginForQuads.h>

namespace icl{
  namespace markers{

    /** \cond */
    class FiducialDetector;
    /** \endcond */


    /// FiducialDetectorPlugin for ARToolkit like markers using binary image patches as marker IDs \ingroup PLUGINS
    /** This is just a plugin class that is used by the FiducialDetector. Please
        refer the icl::markers::FiducialDetector class documentation for more details. */
    class ICLMarkers_API FiducialDetectorPluginART : public FiducialDetectorPluginForQuads{
      struct Data;
      Data *data;

      /// only the FiducialDetector can instantiate this class
      FiducialDetectorPluginART();
      public:

      /// This class cannot be used
      friend class icl::markers::FiducialDetector;

      /// Destructor
      ~FiducialDetectorPluginART();

      /// loads markers ID's
      /** @param add
          @param which this instance of Type icl::Any can be any image filename or filename pattern
                       <b>Please note:</b> internally, all loaded patterns are stored
                       by a unique ID. The unique ID is computed from the image filename by
                       removing the file postfix (e.g. .png) and the files folder prefix (e.g. ./patterns/)
                       In other words, only the image files base-name is used as key. Therefore,
                       the pattern files have to be named differently, even if they have different
                       post-fixes and/or different folders.\n
                       All image files are loaded/removed. The images are internally converted
                       to grayscale. Every time the 'match algorithm' property is changed, all loaed
                       patters are processed in order to optimize matching speed.\n
                       If markers are removed, again, the filepattern is used to 'glob' all markers
                       that have to be removed. The special Token '*' is used to remove <b>all</b>
                       loaded markers.\n
                       The parameter list params must contain the real markers 'size' in mm
         @param params
      */
      virtual void addOrRemoveMarkers(bool add, const utils::Any &which, const utils::ParamList &params);

      /// Identifies the given image patch using bch decoding
      virtual FiducialImpl *classifyPatch(const core::Img8u &image, int *rot, bool returnRejectedQuads,cv::ImageRegion r);

      /// this method is reimplemented here; it returns the impl's file-basename
      std::string getName(const FiducialImpl *impl);

      /// describes the special marker image rectificatio parameters
      virtual void getQuadRectificationParameters(utils::Size &markerSizeWithBorder,
                                                  utils::Size &markerSizeWithoutBorder);

      /// creates marker image from given parameters (see FiducialDetector for more details)
      virtual core::Img8u createMarker(const utils::Any &whichOne,const utils::Size &size, const utils::ParamList &params);
    };
  } // namespace markers
}

