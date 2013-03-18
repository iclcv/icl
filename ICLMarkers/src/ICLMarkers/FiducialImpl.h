/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLMarkers/src/ICLMarkers/FiducialImpl.h               **
** Module : ICLMarkers                                             **
** Authors: Christof Elbrechter                                    **
**                                                                 **
**                                                                 **
** GNU LESSER GENERAL PUBLIC LICENSE                               **
** This file may be used under the terms of the GNU Lesser General **
** Public License version 3.0 as published by the                  **
**                                                                 **
** Free Software Foundation and appearing in the file LICENSE.GPL  **
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

#include <bitset>
#include <ICLUtils/Size32f.h>
#include <ICLMarkers/Fiducial.h>


namespace icl{
  namespace markers{
    
    /// Hidden implemetation for fiduical classes
    /** The FiducialImpl class works as data storage for the public
        Fiducial class. It also holds a back-reference to it's parent
        FiducialDetectorPlugin that is used to compute features
        on demand if not alread available
        
        \section DEV Developers
        The FiduicalImpl should not be visible for the FiduicalDetector user
        who accesses Fiducial information via the public Fiduical class.
        The Fiduical class implementation contains smart methods for
        querying Fiducial-Features. If a feature has been computed once,
        further calls will just return the automatically buffered result.
        
        \section PP Precomputed vs. Deferred Feature Extraction
        The feature Extraction is implemented in the FiducialDetectorPlugin
        class that defines 
        * how Fiducials are detected
        * what features are supported
        * how certain features are computed.
        
        The FiducialDetectorPlugin <b>can</b> initialize the FiduicalImpl instances
        with precomputed values. Most of the time, at least some of the 
        Fiducial feature values are already estimated within the detection step.
        If e.g. a region based fiducial detector needs to estimate a regions 2D-center
        for the detection itself, it can initialize the FiducialInstances with
        these values. Of course, it has to set up the 'computed' mask in the
        FiduicalImpl instances according to the set of precomputed features.\n
        Other features, that are supported, but perhaps more difficult to compute
        can in contrast be supported in a deferred manner. The FiducialDetectorPlugin
        will the leave the 'computed' mask empty for this feature and provide
        an appropriate getter-method implementation. This is in particular useful
        to speed up the detection step itself since usually most complex features
        dont have to be computed for each detected Fiducial.
    */
    struct FiducialImpl{
      /// parent Fiducial Detector instance
      FiducialDetectorPlugin *parent;
      
      /// list of generally available features
      Fiducial::FeatureSet supported;
      
      /// list of already computed features
      Fiducial::FeatureSet computed;
  
      /// Fiduical ID
      /*  The Fiduical ID can be used externally to distinguish between differnent
          Fiducial instances */
      int id;
      
      /// Internally used index
      /** The index can be estimate a FiducialImpl instances index from
          the getFeature-methods in the FiducialDetectorPlugin class */
      int index;
  
      /// real marker size in millimeters
      /** This information is neccessary for creation of feature points and
          for most 3D pose detection stuff */
      utils::Size32f realSizeMM;
  
      /// most plugins can provide an image region
      cv::ImageRegion imageRegion;
      
      /// set of 2D features
      /* this pointer and its members <b>can</b> be instantiated by 
          parent FiduicalDetectorImpl instance */
      struct Info2D{
        utils::Point32f infoCenter;
        float infoRotation;
        std::vector<utils::Point32f> infoCorners;
        std::vector<Fiducial::KeyPoint> infoKeyPoints;
      } *info2D;
  
      /// set of 3D features
      /* this pointer and its members <b>can</b> be instantiated by 
          parent FiduicalDetectorImpl instance */
      struct Info3D{
        geom::Vec infoCenter;
        geom::Vec infoRotation;
        geom::Mat infoPose;
      } *info3D;
  
      /// base constructor initializing index with -1 and all other stuff with 0
      inline FiducialImpl():supported(0),computed(0),id(-1),index(-1),info2D(0),info3D(0){}
  
      /// 2nd constructor with given parameters
      inline FiducialImpl(FiducialDetectorPlugin *parent,
                          Fiducial::FeatureSet supported,
                          Fiducial::FeatureSet computed,
                          int id, int index,
                          const utils::Size32f &realSizeMM):
        parent(parent),supported(supported),
        computed(computed),id(id),index(index),
        realSizeMM(realSizeMM),info2D(0),info3D(0){}
      
      /// explicitly implemented deep copy constructor
      FiducialImpl(const FiducialImpl &o);
  
      /// explicitly implemented assignment operator (deep copy)
      FiducialImpl &operator=(const FiducialImpl &o);
      
      /// Destructor (also deletes info2D and info3D if not null)
      inline ~FiducialImpl(){
        if(info2D) delete info2D;
        if(info3D) delete info3D;
      }
      
      /// ensures that the info2D pointer is initialized
      inline Info2D *ensure2D(){
        if(!info2D) info2D = new Info2D;
        return info2D;
      }
  
      /// ensures that the info3D pointer is initialized
      inline Info3D *ensure3D(){
        if(!info3D) info3D = new Info3D;
        return info3D;
      }
    
    };
    
    
  } // namespace markers
}


