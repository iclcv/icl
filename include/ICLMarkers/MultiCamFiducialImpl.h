/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2012 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : include/ICLMarkers/MultiCamFiducialImpl.h              **
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

#include <ICLMarkers/Fiducial.h>
#include <ICLGeom/Camera.h>

namespace icl{
  namespace markers{
  
    /// Internal Implementation class for the MutiCamFiducial
    /** @section _SB_ Smart Buffering
        The 3D pose estimation results are buffered internally so that
        mutiple calls to MutiCamFiducial::getPose3D do not entail
        doubled pose estimation.      
    */
    struct MultiCamFiducialImpl{
      int id;                              //!< associated fiducial ID
      int numFound;                        //!< number of view, this Fiducial was found in
      std::vector<Fiducial> fids;          //!< all 2D fiducials 
      std::vector<Camera*> cams;           //!< all cameras
      FixedColVector<float,3> center;      //!< smart buffer for the center
      FixedColVector<float,3> orientation; //!< smart buffer for the orientation 
      Mat pose;                            //!< smart buffer for the pose
      bool haveCenter;      //!< has the center already been estimated
      bool haveOrientation; //!< has the orientation already been estimated
      bool havePose;        //!< has the pose already been estimated
      
      /// null/empty constructor
      MultiCamFiducialImpl();
      
      /// default constructor with given ID
      MultiCamFiducialImpl(int id,
                           const std::vector<Fiducial> &fids,
                           const std::vector<Camera*> cams);
      
  
      /// (re-) initialization
      void init(int id);
      
      /// estimate and return the 3D center
      const FixedColVector<float,3> &estimateCenter3D();
  
      /// estimate and return the 3D pose
      const Mat &estimatePose3D();
      
      /// estimate and return the 3D orientation
      const FixedColVector<float,3> &estimateOrientation3D();
    };
  
  } // namespace markers
}
