/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2012 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : include/ICLAlgorithms/TemplateTracker.h                **
** Module : ICLAlgorithms                                          **
** Authors: Eckard Riedenklau, Christof Elbrechter                 **
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

#ifndef ICL_TEMPLATE_TRACKER_H
#define ICL_TEMPLATE_TRACKER_H

#include <ICLUtils/Configurable.h>
#include <ICLUtils/Uncopyable.h>
#include <ICLCore/Img.h>
namespace icl{


  /// Utility class vor viewbased template tracking
  /** TODO: add more documentation! */
  class TemplateTracker : public Configurable, public Uncopyable{
    struct Data; //!< internal data storage
    Data *data;  //!< internal data pointer
    
    public:
    /// Result class that describes a tracking result
    struct Result{
      /// Constructor with given parameters
      inline Result(const Point32f &pos=Point32f(-1,-1), 
                    float angle=0, float proximityValue=0,
                    const Img8u *matchedTemplateImage=0):
        pos(pos),angle(angle),proximityValue(proximityValue),
        matchedTemplateImage(matchedTemplateImage){}
      Point32f pos; //!< image position
      float angle;  //!< pattern orientation
      float proximityValue; //!< match quality
      /// internally assotiate (and rotatated) tempalte
      const Img8u *matchedTemplateImage;  
    };

    /// Constructor with given parameters
    /** TODO: describe parameters and methology */
    TemplateTracker(const Img8u *templateImage=0,
                    float rotationStepSizeDegree=1.0,
                    int positionTrackingRangePix=100, 
                    float rotationTrackingRangeDegree=45,
                    int coarseSteps=10,int fineSteps=1,
                    const Result &initialResult=Result());

    /// Desctructor
    ~TemplateTracker();
    
    
    /// utility method that shows the template rotation lookup table
    void showRotationLUT() const;
    
    /// sets a new set or rotated template images
    void setRotationLUT(const std::vector<SmartPtr<Img8u> > &lut);
    
    /// sets a new template image, that is internally rotated
    /** internally 360/rotationStepSizeDegree images are sampled using
        image rotation. Please note that at some loations the image
        edges are cut. Therefore, the template should only be located
        within the inner center circle of the template's image rectangle */
    void setTemplateImage(const Img8u &templateImage, 
                          float rotationStepSizeDegree=1.0);
    
    /// actual track method
    Result track(const Img8u &image, const Result *initialResult=0,
                 std::vector<Result> *allResults=0);

  };

}

#endif
