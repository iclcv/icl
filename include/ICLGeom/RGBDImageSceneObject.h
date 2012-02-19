/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2010 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : include/ICLGeom/RGBDImageSceneObject.h                 **
** Module : ICLGeom                                                **
** Authors: Christof Elbrechter, Andre Ückermann                   **
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

#ifndef ICL_RGBD_IMAGE_SCENE_OBJECT_H
#define ICL_RGBD_IMAGE_SCENE_OBJECT_H

#include <ICLGeom/Camera.h>
#include <ICLGeom/SceneObject.h>
#include <ICLGeom/RGBDMapping.h>
#include <ICLUtils/Configurable.h>


namespace icl{
  
  
  class RGBDImageSceneObject : public SceneObject, public Configurable{
    protected:
    /// corresponging depth image size
    Size m_size;
    
    /// each view-ray directions and depth normalization factors
    /** v = [ dir-x, dir-y, dir-z, norm-factor ] */
    std::vector<Vec> m_viewRaysAndNorms;
    
    /// Camera center
    Vec m_viewRayOffset;
    
    /// mapping, that maps RGB to depth values
    RGBDMapping m_mapping;
    
    /// computes the normalization factor for two given viewrays
    /** if the depth values, a camera provide are given wrt to the distance to the 
        camera's sensor plane, using view-rays demands to correct these values
        because, the view-ray lengths need the distances to the sensor center.
        
        In 3D space, the nomalization factor is exactly the cosine between the
        center view-ray direction an the actual view ray direction        
    */
    float getDepthNorm(const Vec &dir, const Vec &centerDir);

    void init(const Size &size,
              const RGBDMapping &mapping,
              const Camera &cam);
    public:
    
    /// returns a default camera model for VGA and QVGA kinect cameras
    /** If the default camera model does not work well for your application, you
        can calibrate your kinect device using the icl-cam-calib-2 tool */
    static Camera get_default_kinect_camera(const Size &size);
    
    /// returns a default rgb to depth mapping for VGA and QVGA kinect cameras
    /** if the default mapping does not work for your kinect device, you can 
        create a new mapping using the icl-kinect-rgbd-calib tool */
    static RGBDMapping get_default_kinect_rgbd_mapping(const Size &size);

    /// creates an RGBDImageSceneObject with given size which uses the default mapping and camera
    RGBDImageSceneObject(const Size &size = Size::VGA);
    
    /// creates and RGBDImageSceneObject with given mapping and camera
    RGBDImageSceneObject(const Size &size, const RGBDMapping &mapping,const Camera &cam);
    
    /// returns the current mapping
    const RGBDMapping &getMapping() const;
    
    /// sets the current mapping
    void setMapping(const RGBDMapping &mapping);
    
    /// returns the current size
    const Size &getSize() const;

    /// returns all view-rays-directions and norms
    /** Each Vec in the return value is layouted [x,y,z,n], where (x,y,z) is the viewray
        and n is the depthvalue normalization factor. The whole data vector is orded row-major */
    const std::vector<Vec> &getViewRaysAndNorms() const;

    /// updates the scene object from new kinect depth and optionally also color image
    /** the internal point-cloud image is created using the given depth image.
        @param depthImage kinect depth image, whose values are assumed to be in [mm] units
        @param optionally given RGB image. If this is not null, its values are mapped to the 
               depth-image point cloud using the current mapping
        */
    virtual void update(const Img32f &depthImage, const Img8u *rgbImage=0);
  };
}

#endif
