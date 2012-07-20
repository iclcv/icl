/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2012 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : include/ICLGeom/PointCloudObject                       **
** Module : ICLGeom                                                **
** Authors: Christof Elbrechter, Patrick Nobou                     **
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

#ifndef ICL_POINT_CLOUD_OBJECT_H
#define ICL_POINT_CLOUD_OBJECT_H

#include <ICLGeom/PointCloudObjectBase.h>

namespace icl{
  
  /// Base implementation of the SceneObjectBase interface for compability with common icl::SceneObjects
  /** This class replaces the former implementations
      - PointcloudSceneObject
      - RGBDImageSceneObject 
      
      \section TODO
      - add optional color (perhaps not possible)
      - add optional normals and may other useful fields
  */
  class PointCloudObject : public PointCloudObjectBase{
    protected:
    bool m_organized; //!< internal 2D organized flag
    Size m_dim2D;     //!< 2D dimension
    
    public:
    
    /// creates a new ordered or unordered SimplePointCloudObject instance
    /** @param width number of points per line (if unordered, number of points)
        @param height number of points per row (if unordered, height is not used)
        @param organized specifies whether there is a 2D data order or not
        */
    PointCloudObject(int width=0, int height=0, bool organized=true);

    /// returns which features are supported (only XYZ and RGBA32f)
    virtual bool supports(FeatureType t);
    
    /// returns whether the points are 2D-ordered
    virtual bool isOrganized() const;

    /// returns the 2D size of the pointcloud (throws exception if not ordered)
    virtual Size getSize() const throw (ICLException);
    
    /// return the linearily ordered number of point in the point cloud
    virtual int getDim() const;

    /// adapts the point cloud size
    /** if the sizes height is smaller than 1, the cloud becomes un-organized*/
    virtual void setSize(const Size &size);

    /// returns XYZ data segment
    virtual DataSegment<float,3> selectXYZ();
    
    /// returns the RGBA data segment (4-floats)
    virtual DataSegment<float,4> selectRGBA32f();

    /// important, this is again, reimplemented in order to NOT draw the stuff manually here
    virtual void customRender();

    /// deep copy function
    virtual PointCloudObject *copy() const {
      return new PointCloudObject(*this);
    }

  };

  
  
}

#endif
