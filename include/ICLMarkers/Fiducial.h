/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2012 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : include/ICLMarkers/Fiducial.h                          **
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

#ifndef ICL_FIDUCIAL_H
#define ICL_FIDUCIAL_H

#include <bitset>

#include <ICLUtils/Point32f.h>
#include <ICLGeom/GeomDefs.h>
#include <ICLCV/ImageRegion.h>

namespace icl{
  namespace markers{
    
    /** \cond */
    struct FiducialImpl;
    struct FiducialDetectorPlugin;
    /** \endcond */
    
    
    /// Main class for detected image markers/fiducials
    /** Internally, each fiducial is just a shallow wrapper
        around a FiducialImpl instance, which is managed
        by it's parent FiducialDetectorPlugin instance.
        Therefore, Fiducial instances can simply be copied
        as fast as a pointer-copy.
    */
    class Fiducial{
      /// hidden implementation (always managed by the parent MarkerDetector)
      FiducialImpl *impl;
      
      public:
      
      
      /// Currently supported feature types
      /** Features can be binary-ored in order to create FeatureSet instances */
      enum Feature{
        Center2D,     //!< center in image coordinates
        Rotation2D,   //!< rotation in the image plain
        Corners2D,    //!< list of corners
        KeyPoints2D,  //!< list of 2D points with corresponding marker coordinates
        ImageRegion,  //!< associated image region
        Center3D,     //!< 3D center information
        Rotation3D,   //!< 3D orientation information
        Pose3D,       //!< 3D orientation information
        FeatureCount  //!< number of features
      };
      
      /// FeatureSet class
      /** given a SeatureSet instances s, Feature x is supported if
          s[x] is true */
      typedef std::bitset<(int)FeatureCount> FeatureSet;
  
      /// a full feature set 
      static const FeatureSet AllFeatures;
      
      /// key point structure
      /** A key point is a certain known point on the marker 
          for each keypoint, an image position as well as
          position in (real world) marker coordinates. Furthermore
          KeyPoints have an ID, which can be used e.g. for
          multi-view marker pose detection.
      */
      struct KeyPoint{
        /// Default constructor (does nothing)
        inline KeyPoint(){}
        
        /// Special constructor with given params
        inline KeyPoint(const Point32f &imagePos, const Point32f &markerPos, int ID):
        imagePos(imagePos),markerPos(markerPos),ID(ID){}
  
        Point32f imagePos;  //!< key point in image space
        Point32f markerPos; //!< key point in marker space
        int ID;             //!< key point ID
      };
      
      /// private Constructor
      inline Fiducial(FiducialImpl *impl=0):impl(impl){}
  
      /// the marker ID
      int getID() const;
      
      /// returns a string representation of the marker name
      std::string getName() const;
      
      /// returns wheather the Marker provides given information
      bool supports(Feature f) const;
      
      /// returns the marker'S center in the image
      const Point32f &getCenter2D() const;
      
      /// retrurs the marker's rotation in the image plain
      const float &getRotation2D() const;
      
      /// returns the markers corners
      const std::vector<Point32f> &getCorners2D() const;
  
      /// returns key points in the marker
      const std::vector<KeyPoint> &getKeyPoints2D() const;
  
      /// returns the associated image region
      const icl::ImageRegion getImageRegion() const;
  
      /// returns the markers
      const Vec &getCenter3D() const;
      
      /// returns the markers
      const Vec &getRotation3D() const;
      
      /// returns the markers
      const Mat &getPose3D() const;    
  
      /// returns whether is marker has been initialized
      inline operator bool() const { return impl; }
  
      /// returns whether is marker has not been initialized
      inline bool operator!() const { return isNull(); }
  
      /// returns whether is marker has not been initialized
      inline bool isNull() const { return !impl; }
  
      /// returns whether the fiducials have the same impl
      inline bool operator==(const Fiducial &a) const { return impl == a.impl; }
  
      /// returns whether the fiducials have not the same impl
      inline bool operator!=(const Fiducial &a) const{ return impl != a.impl; }
      
      /// returns the marker's internal implementation structure
      inline FiducialImpl *getImpl() { return impl; }
  
      /// returns the marker's internal implementation structure (const)
      inline const FiducialImpl *getImpl() const { return impl; }
      
      /// returns the parent fiducial detector
      FiducialDetectorPlugin *getDetector();
  
      /// returns the parent fiducial detector (const)
      const FiducialDetectorPlugin *getDetector() const;
    };
    
    
    /// or operator for convenient creation of Fiducial::FeatureSet instances
    inline Fiducial::FeatureSet operator|(Fiducial::FeatureSet s, Fiducial::Feature f){
      s.set(f); return s;
    }
  
    /// or operator for convenient creation of Fiducial::FeatureSet instances
    inline Fiducial::FeatureSet operator|(Fiducial::Feature f, Fiducial::FeatureSet s){
      return s|f;
    }
  
    /// or operator for convenient creation of Fiducial::FeatureSet instances
    inline Fiducial::FeatureSet operator|(Fiducial::Feature f1, Fiducial::Feature f2){
      Fiducial::FeatureSet s; s.reset(); s.set(f1); s.set(f2); return s;
    }
  } // namespace markers
}

#endif
