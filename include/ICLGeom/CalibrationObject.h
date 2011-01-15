/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2010 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : include/ICLGeom/CalibrationObject.h                    **
** Module : ICLGeom                                                **
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

#ifndef ICL_CALIBRATION_OBJECT_H
#define ICL_CALIBRATION_OBJECT_H

#include <ICLUtils/Uncopyable.h>
#include <ICLCore/ImgBase.h>
#include <ICLUtils/Configurable.h>
#include <ICLGeom/CalibrationGrid.h>

namespace icl{
  
  /// Utility class for Camera calibration 
  /** \section GEN General information 
      TODO !!

      \section IIL Internal Image List
      Internally, the calibration object uses an image list that stores intermediate image
      processing results. Currently the following images are available and accessible using
      the getImage(const std::string&) method:
      - input (last given input image)
      - gray (gray converted image, or same as input if input was already in format gray and depth8u)
      - threshold (locally thresholded image)
      - dilated (result of dilation applied on the threshold image, or same as threshold if dilation 
        is deactivated using the Configurable interface)
  */
  class CalibrationObject : public Uncopyable, public SceneObject, public Configurable{
    public:

    /// enumeration of intermediate image processing result images (for debuggin and visualization only)
    enum IntermediateImageType{
      InputImage,     //!< last given input image (maybe color)
      GrayImage,      //!< gray image (if input image was gray, then this is identical to InputImage)
      ThresholdImage, //!< result of local threshold operation
      DilatedImage    //!< result of image dilation (if dilation is disabled, this is identical to ThresholdImage)
    };
    private:
    
    class Data; //!< internal data class
    Data *data; //!< internal data storage

    protected:
    /// utility method that is used internally to store a given input image in the internal image list
    /** The given input image is stored at key 'input' */
    const ImgBase *storeInputImage(const ImgBase *inputImage);

    /// converts the given 'color'-input image into gray format if neccessary
    /** the result image is automatically stored in the internal image list
        at key 'gray' */
    const ImgBase *computeAndStoreGrayImage(const ImgBase *colorImage);
    
    /// applies a local threshold on the given 'gray'-image into color'-input image into gray format if neccessary
    /** the result image is automatically stored in the internal image list
        at key 'threshold' */
    const ImgBase *computeAndStoreThresholdImage(const ImgBase *grayImage);
    
    /// applies a dilation filter on the given thresholded image
    /** If the dilation-mode is deactivated, this returns the thresholded image pointer
        directly. The result of this method (either dilation result of thresholdedImage)
        is automatically stored in the internal image list at key 'dilated' */
    const ImgBase *computeAndStoreDilatedImage(const ImgBase *thresholdedImage);
    
    /// sets an intermediate iamge
    void setIntermediateImage(IntermediateImageType t, const ImgBase *image);

    public:
    
    /// returns the intermediate image processing result of null if not available
    const ImgBase *getIntermediateImage(IntermediateImageType t) const;
    
    
    /// creates an empty calibration object 
    CalibrationObject();
    
    /// Destructor
    ~CalibrationObject();
    
    /// The ownership of the calibration grid is NOT passed
    CalibrationObject(CalibrationGrid *grid,
                      const std::string &configurableID);
    
    /// for deferred initialization (can only be called once
    void init(CalibrationGrid *grid,
              const std::string &configurableID) throw (ICLException);
    
    /// returns whether the CalibrationObject is already inited
    bool isNull() const;
    
    /// Simple result struct
    struct CalibrationResult{
      const Camera &cam; //!< Calibration Result
      float error;       //!< Calibration Error
      
      /// Shortcut implicit cast to bool to check whether calibration was successful
      operator bool() const { return error >= 0; }
    };

    /// return calibration error (or -1 if object was not found)
    CalibrationResult find(const ImgBase *sourceImage);
    
    /// visualizes current detection result
    /** Note: the widge must be prepared before (image must be set, widget must
        be locked and reset) */
    void visualizeGrid2D(ICLDrawWidget &d);
    
    /// This functions gets an arbitrary input image and returns the currently detected image points
    /** <b>Note:</b> If you overwrite this method, you should set intermediate images
        that do no longer exist to null or to the input image 
        \code
        setIntermediateImage(InputImage,0);
        setIntermediateImage(GrayImage,0);
        setIntermediateImage(ThresholdImage,0);
        setIntermediateImage(DilatedImage,0);
        \endcode
        or
        \code
        setIntermediateImage(InputImage,sourceImage);
        setIntermediateImage(GrayImage,sourceImage);
        setIntermediateImage(ThresholdImage,sourceImage);
        setIntermediateImage(DilatedImage,sourceImage);
        \endcode
    */
    virtual const ImgBase *findPoints(const ImgBase *sourceImage,
                                      std::vector<Point32f> &cogs,
                                      std::vector<Rect> &bbs);

    /// provides access to the currently used calibration grid
    CalibrationGrid *getCalibrationGrid();
    
    /// provides access to the currently used calibration grid (const)
    const CalibrationGrid *getCalibrationGrid() const;
  };
}


#endif
