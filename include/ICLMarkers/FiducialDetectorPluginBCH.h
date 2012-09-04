/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2012 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : include/ICLMarkers/FiducialDetectorPluginBCH.h         **
** Module : ICLCV                                                **
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

#pragma once

#include <ICLMarkers/FiducialDetectorPluginForQuads.h>

namespace icl{
  namespace markers{
    
    /** \cond */
    class FiducialDetector;
    /** \endcond */
    
    
    /// FiducialDetectorPlugin for ARToolkit+ like markers using BCH coded IDs \ingroup PLUGINS
    /** This is just a plugin class that is used by the FiducialDetector. Please
        refer the icl::FiducialDetector class documentation for more details. */
    class FiducialDetectorPluginBCH : public FiducialDetectorPluginForQuads{
      struct Data;
      Data *data;
  
      /// only the FiducialDetector can instantiate this class
      FiducialDetectorPluginBCH();
      public:
  
      /// This class cannot be used 
      friend class icl::markers::FiducialDetector;
  
      /// Destructor
      ~FiducialDetectorPluginBCH();
      
      /// loads markers ID's
      /** @param def this any instance can be ...
          * of type int (then, only the corresponding marker ID is loaded) 
          * of type utils::Range32s "[a,b]", (then all markers within the range are loaded)
          * of something like {a,b,c,d,...} then all marker IDs in the list are loaded
  
          Please note that other types might be interpreted in the wrong way.
          Mandatory parameter is "size". Please refer to the 
          documentation of icl::FiducialDetector::loadMarkers for more details
      */
      virtual void addOrRemoveMarkers(bool add, const utils::Any &which, const utils::ParamList &params);
  
      /// extracts and stores some properties locally to speed up classifyPatch 
      void prepareForPatchClassification();
  
      /// Identifies the given image patch using bch decoding
      virtual FiducialImpl *classifyPatch(const core::Img8u &image, int *rot, bool returnRejectedQuads, cv::ImageRegion r);
  
      /// describes the marker rectification parameters
      virtual void getQuadRectificationParameters(utils::Size &markerSizeWithBorder,
                                                  utils::Size &markerSizeWithoutBorder);        
  
      /// creates bch marker image
      virtual core::Img8u createMarker(const utils::Any &whichOne,const utils::Size &size, const utils::ParamList &params);
  
    };
  } // namespace markers
}

