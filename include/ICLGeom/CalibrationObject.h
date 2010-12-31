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

  */
  class CalibrationObject : public Uncopyable, public SceneObject, public Configurable{
    class Data; //!< internal data class
    Data *data; //!< internal data storage

    protected:
    /// this method allows to set the calibration objects internal images
    /** The internal images are used for visualization. The currently used
        image is determined by the Configurable interface. Currently 
        image names 'input','gray','threhold' and 'dilated' can be used
        by the Configurable's 'visualized image'-property */
    void setImage(const std::string &name, const ImgBase *image);
    
    public:
    /// The ownership of the calibration grid is NOT passed
    CalibrationObject(CalibrationGrid *grid,
                      const std::string &configurableID);
    
    /// Simple result struct
    struct CalibrationResult{
      const Camera &cam; //!< Calibration Result
      float error;       //!< Calibration Error
      
      /// Shortcut implicit cast to bool to check whether calibration was successful
      operator bool() const { return error >= 0; }
    };

    /// return calibration error (or -1 if object was not found)
    CalibrationResult find(const ImgBase *sourceImage);
    
    /// visualizes current detection result and image
    /** this also sets the given DrawWidget's image */
    void visualize2D(ICLDrawWidget &d, bool unlockWidget=true);
    
    /// This functions gets an arbitrary input image and returns the currently detected image points
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
