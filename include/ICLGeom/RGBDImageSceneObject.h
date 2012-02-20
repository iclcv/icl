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

#include <ICLUtils/Configurable.h>
#include <ICLUtils/Array2D.h>

#include <ICLGeom/Camera.h>
#include <ICLGeom/SceneObject.h>
#include <ICLGeom/RGBDMapping.h>

namespace icl{
  class RGBDImageSceneObject : public SceneObject, public Configurable{
    /// internal data structure
    struct Data;
    
    /// internal data pointer (pimpl)
    Data *m_data;
  
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
    
    /// Destructor
    ~RGBDImageSceneObject();
    
    /// returns the current mapping
    const RGBDMapping &getMapping() const;
    
    /// sets the current mapping
    void setMapping(const RGBDMapping &mapping);
    
    /// returns the current size
    const Size &getSize() const;

    /// returns the camera's viewray directions
    const std::vector<Vec3> &getViewRayDirs() const;
    
    /// depth value correction factors
    const std::vector<float> &getCorrectionFactors() const;
    
    /// returns the last corrected depth image
    /** the result image contains the corrected depth image from the last update call */
    const Img32f &getCorrectedDepthImage() const;

    /// updates the scene object from new kinect depth and optionally also color image
    /** the internal point-cloud image is created using the given depth image.
        @param depthImage kinect depth image, whose values are assumed to be in [mm] units
        @param optionally given RGB image. If this is not null, its values are mapped to the 
               depth-image point cloud using the current mapping
        */
    virtual void update(const Img32f &depthImage, const Img8u *rgbImage=0);

    
    /// maps another given image just like the rgbImage would be mapped
    /** This method uses the last depthImage that was passed to RGBDImageSceneObject::update */
    void mapImage(const ImgBase *src, ImgBase **dst);

    /// returns the objects 3D points as Array2D<Vec>
    /** The returned Array2D<Vec> is just a shallow wrapper around the internal data pointer. */
    inline Array2D<Vec> getPoints();

    
    /// returns the objects 3D points as Array2D<Vec> (const version)
    /** The returned Array2D<Vec> is just a shallow wrapper around the internal data pointer. */
    inline const Array2D<Vec> getPoints() const;
      
    /// returns the objects point colors Array2D<Vec>
    /** The returned Array2D<GeomColor> is just a shallow wrapper around the internal data pointer. 
        The color data is just valid if RGBDImageSceneObject::update was called with a non-null rgbImage. */
    Array2D<GeomColor> getColors();

    /// returns the objects point colors Array2D<Vec> (const version)
    /** The returned Array2D<GeomColor> is just a shallow wrapper around the internal data pointer. 
        The color data is just valid if RGBDImageSceneObject::update was called with a non-null rgbImage. */
    const Array2D<GeomColor> getColors() const;

    virtual void prepareForRendering();
    
    virtual void customRender();
    
  };
}

#endif
