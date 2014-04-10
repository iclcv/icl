/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLGeom/src/ICLGeom/PointCloudObject.h                 **
** Module : ICLGeom                                                **
** Authors: Christof Elbrechter, Patrick Nobou                     **
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
#include <ICLGeom/PointCloudObjectBase.h>

namespace icl{
  namespace geom{
    
    /// Base implementation of the SceneObjectBase interface for compability with common icl::SceneObjects
    /** This class replaces the former implementations
        - PointcloudSceneObject
        - RGBDImageSceneObject 
        
        \section NORMALS Normals
        
        The PointCloudObject can be set up to have also normals in the constructor.
        
        \section TODO
        - add optional color (perhaps not possible)
        
     */
    class ICLGeom_API PointCloudObject : public PointCloudObjectBase{
      protected:
      bool m_organized;       //!< internal 2D organized flag
      utils::Size m_dim2D;    //!< 2D dimension
      bool m_hasNormals;      //!< flag whether normals are given
      bool m_hasColors;       //!< flag whether the point cloud has colors
      
      public:
      
      
      /// creates an empty point cloud with with optionally initialized featuers
      PointCloudObject(bool withNormals=false, bool withColors=false);
      
      /// create an un-organizied point cloud with N points
      PointCloudObject(int numPoints, bool withNormals=false, bool withColors=true);
      
      /// creates a new organized or un-organized SimplePointCloudObject instance
      /** @param width number of points per line (if unordered, number of points)
          @param height number of points per row (if unordered, height is not used)
          @param organized specifies whether there is a 2D data order or not
          @params withNormals if true, also normals will be created for each point
          */
      PointCloudObject(int width, int height, bool organized=true, bool withNormals=false, bool withColors=true);
  
      /// returns which features are supported (only XYZ and RGBA32f)
      virtual bool supports(FeatureType t) const;
      
      /// returns whether the points are 2D-ordered
      virtual bool isOrganized() const;

      /// returns the 2D size of the pointcloud (throws exception if not ordered)
      virtual utils::Size getSize() const throw (utils::ICLException);
      
      /// return the linearily ordered number of point in the point cloud
      virtual int getDim() const;
  
      /// adapts the point cloud size
      /** if the sizes height is smaller than 1, the cloud becomes un-organized*/
      virtual void setSize(const utils::Size &size);
  
      /// returns XYZ data segment
      virtual core::DataSegment<float,3> selectXYZ();

      /// returns XYZH data segment
      virtual core::DataSegment<float,4> selectXYZH();
      
      /// returns the RGBA data segment (4-floats)
      virtual core::DataSegment<float,4> selectRGBA32f();

      /// returns the Normals data segment (4-floats)
      /** Only available if the constructor was called with "withNormals" set to true */
      virtual core::DataSegment<float,4> selectNormal();
      
      /// important, this is again, reimplemented in order to NOT draw the stuff manually here
      virtual void customRender();
  
      /// deep copy function
      virtual PointCloudObject *copy() const {
        return new PointCloudObject(*this);
      }

      /// just a simple wrapper for top-top level classe's addVertex method
      /** This method does not work for organized point cloud objects. The
          organized flag is not checked for performance reason. The behaviour
          of calling push_back on ordered point clouds is undefined */
      void push_back(const Vec &point){
        addVertex(point,GeomColor(0,100,255,255));
        if(m_hasNormals) m_normals.push_back(Vec(0,0,0,1));
      }
      
      /// adds xyz point with given color
      /** @see push_back(const Vec&) */
      void push_back(const Vec &point, const GeomColor &color){
        addVertex(point,color);
        if(m_hasNormals) m_normals.push_back(Vec(0,0,0,1));
      }

      /// adds xyz point with given normal and color
      /** @see push_back(const Vec&) */
      void push_back(const Vec &point, const Vec &normal, const GeomColor &color){
        addVertex(point,color);
        if(m_hasNormals) m_normals.push_back(Vec(0,0,0,1));
      }
      
      private:
      
      /// hidden in this interface to avoid 
      using SceneObject::addVertex;
      using SceneObject::addNormal;
  
    };
  
    
    
  } // namespace geom
}

