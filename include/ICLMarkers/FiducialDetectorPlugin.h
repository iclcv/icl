/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2012 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : include/ICLMarkers/FiducialDetectorPlugin.h            **
** Module : ICLBlob                                                **
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

#ifndef ICL_FIDUCIAL_DETECTOR_PLUGIN_H
#define ICL_FIDUCIAL_DETECTOR_PLUGIN_H

#include <ICLUtils/Any.h>
#include <ICLUtils/Configurable.h>
#include <ICLUtils/ParamList.h>

#include <ICLCore/Img.h>

#include <ICLGeom/Camera.h>
#include <ICLGeom/CoplanarPointPoseEstimator.h>

#include <ICLMarkers/FiducialImpl.h>

namespace icl{
  
  /// Generic Interface class for FiducialDetector plugins \ingroup PLUGINS
  struct FiducialDetectorPlugin : public Configurable{
    /// Camera structure for 3D feature estimation
    /** The actual camera instance is managed by the parent
        FiducialDetector instance */
    Camera *camera;

    /// for 3D pose estimation
    CoplanarPointPoseEstimator poseEst;
        
    
    /// initializes the plugin with a 0-camera
    FiducialDetectorPlugin();
    
    /// returns how to compute the markers 2D center
    /** <b>note</b>, if this information is already available in the detect method,
        the plugin implementation should instantiate the returned FiducialImpl instances,
        with this this information given. If the fiducial imple was instantiated this way,
        the corresponding getter method must not be reimplemented, because the Fiducial
        wrapper (that wraps the FiducialImpl) will simple return the already available
        result from the corresponding FiducialImpl's info structure
    */
    virtual void getCenter2D(Point32f &dst, FiducialImpl &impl);
    
    /// returns how to compute the markers 2D rotation
    virtual void getRotation2D(float &dst, FiducialImpl &impl);
    
    /// returns how to compute the markers 2D corners (or significant points on its edges
    /** The corners can also be whole line-strips, usually, these are used for visualization
        only */
    virtual void getCorners2D(std::vector<Point32f> &dst, FiducialImpl &impl);
    
    /// returns how to compute a list of image-position/marker-position correspondences
    /** The 2D-keypoints are the most common information that is use to compute a markers
        3D information. Each keypoint defines a 2D marker location in [mm] and a corresponding
        image location */
    virtual void getKeyPoints2D(std::vector<Fiducial::KeyPoint> &dst, FiducialImpl &impl);

    /// computes a markers 3D center
    /** The defalut implementation uses getKeyPoints2D for planar pose estimation.
        If getKeyPoints is not implemented or if less than 4 key points are available,
        the default implementation for getCenter3D will not work
        Note: 3D information are usually only available if a camera was given to the parent
        FiducialDetector class
    */
    virtual void getCenter3D(Vec &dst, FiducialImpl &impl);

    /// computes a markers 3D rotation euler angles
    /** The defalut implementation uses getKeyPoints2D for planar pose estimation.
        If getKeyPoints is not implemented or if less than 4 key points are available,
        the default implementation for getCenter3D will not work
        Note: 3D information are usually only available if a camera was given to the parent
        FiducialDetector class
    */
    virtual void getRotation3D(Vec &dst, FiducialImpl &impl);

    /// computes a markers 3D position and rotation euler angles
    /** The defalut implementation uses getKeyPoints2D for planar pose estimation.
        If getKeyPoints is not implemented or if less than 4 key points are available,
        the default implementation for getCenter3D will not work
        Note: 3D information are usually only available if a camera was given to the parent
        FiducialDetector class
    */
    virtual void getPose3D(Mat &dst, FiducialImpl &impl);
    
    /// Enumeration for differnt source image types
    enum SourceImageType{
      Binary,  //!< detect must be called with an already binarized image (default)
      Gray,    //!< detect must be called with gray scale image
      Color    //!< detect must be called with color image
    };
    
    /// returns Binary (as default)
    virtual SourceImageType getPreProcessing() const { 
      return Binary;
    }

    /// defines which features are supported
    virtual void getFeatures(Fiducial::FeatureSet &dst)=0;
    
    /// defines how to detect markers from a given image
    virtual void detect(std::vector<FiducialImpl*> &dst, const Img8u &image)=0;
    
    /// defines how to load/remove marker definitions
    virtual void addOrRemoveMarkers(bool add, const Any &which, const ParamList &params)=0;

    /// creates a human readable name for the given impl
    /** Per default, this returns a string repesentation of the FiducialImpl's id */
    virtual std::string getName(const FiducialImpl *impl);
    
    /// optionall returns a (comma separated) list of intermediate images
    /** These images can be used for debugging purpose as well as for further processing steps.
        The two intermediate image names 'input' and 'pp' must not be used in the plugin implementation
        because these names are already in use by the parent FiducialDetector instance. */
    virtual std::string getIntermediateImageNames() const{ return ""; }
    
    /// returns the intermediate image, that is associated with the given name
    /** The programmer of a plugin must ensure, that an exception is thrown if the given
        name is not associated with an intermediate image. The parent FiducialDetectors
        intermediate images with names "input" and "pp" must not be checked here. But in
        turn, these two names must not be used for intermediate image names in the
        plugin implementation */
    virtual const ImgBase *getIntermediateImage(const std::string &name) const throw (ICLException){
      throw ICLException("FiducialDetectorPlugin: no intermediate image is associated with givne name '" + name + "'");
      return 0;
    }
    
    /// parses an any instance that is either an int-value, or a range or a list of int-values
    /** The format is "int" | or "{int,int,int}" etc. or "[min,max]" */
    static std::vector<int> parse_list_str(const Any &s);
    
    /// interface for creating an image of a specific marker
    virtual Img8u createMarker(const Any&,const Size &, const ParamList &){
      throw ICLException("FiducialDetectorPlugin::createMarker seems to be not implemented for this marker type");
      return Img8u();
    }

  };
  
}

#endif
