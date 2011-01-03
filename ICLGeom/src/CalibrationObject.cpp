/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2010 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLGeom/src/CalibrationObject.cpp                      **
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

#include <ICLGeom/CalibrationObject.h>

#include <ICLUtils/DynMatrix.h>
#include <ICLUtils/Point32f.h>
#include <ICLUtils/ConfigFile.h>
#include <ICLGeom/Camera.h>

#include <ICLCore/Img.h>
#include <ICLCC/CCFunctions.h>
#include <ICLQt/DrawWidget.h>

#include <ICLFilter/LocalThresholdOp.h>
#include <ICLFilter/MorphologicalOp.h>
#include <ICLBlob/RegionDetector.h>

#include <ICLCC/CCFunctions.h>
#include <ICLCore/Mathematics.h>



namespace icl{
  typedef FixedColVector<float,3> Vec3;
  
  struct CalibrationObject::Data{
    ImgBase *gray8u;
    LocalThresholdOp lt;
    RegionDetector rd;
    MorphologicalOp mo;
    CalibrationGrid *grid;
    Camera cam;
    
    std::map<IntermediateImageType,const ImgBase*> images;
    
    Data():gray8u(0),lt(35,-10,0),rd(90,1800,0,0),mo(MorphologicalOp::dilate3x3){
      mo.setClipToROI(false);
      lt.setClipToROI(false);
    }
    ~Data(){
      ICL_DELETE(gray8u);
    }
  };

  CalibrationObject::CalibrationObject(CalibrationGrid *grid,
                                       const std::string &configurableID):data(new Data){
    
    setConfigurableID(configurableID);
    addChildConfigurable(&data->rd,"region detector");
    addChildConfigurable(&data->lt,"local threshold");

    addProperty("morph thresholded image","menu","off,on","on");
    addProperty("min formfactor","range","[0.5,8]","2.5"); 
    //addProperty("visualized image","menu","input,gray,threshold,dilated","input"); 
    
    data->grid = grid;
    grid->initializeSceneObject(*this);
    
    /// remove properties, that are not appropriate here
    deactivateProperty("^local threshold.UnaryOp");
    deactivateProperty("^local threshold.gamma slope");
    deactivateProperty("^region detector.create region graph");
    deactivateProperty("^region detector\\..* value");
    deactivateProperty("^region detector\\.CSS\\..*");
  }  
  

  const ImgBase *CalibrationObject::getIntermediateImage(IntermediateImageType t) const{
    std::map<IntermediateImageType,const ImgBase*>::const_iterator it = data->images.find(t);
    if(it != data->images.end()) return it->second;
    else return 0;
  }

  /// returns the intermediate image processing result of null if not available
  void CalibrationObject::setIntermediateImage(IntermediateImageType t, const ImgBase *image){
    data->images[t] = image;
  }


  const ImgBase *CalibrationObject::storeInputImage(const ImgBase *inputImage){
    return (data->images[InputImage] = inputImage);
  }
  const ImgBase *CalibrationObject::computeAndStoreGrayImage(const ImgBase *colorImage){
    if(colorImage->getDepth() != depth8u || colorImage->getChannels() != 1){
      ensureCompatible(&data->gray8u,depth8u,colorImage->getSize(),formatGray);
      icl::cc(colorImage,data->gray8u);
      return data->images[GrayImage] = data->gray8u;
    }else{
      return (data->images[GrayImage] = colorImage);
    }
  }
  const ImgBase *CalibrationObject::computeAndStoreThresholdImage(const ImgBase *grayImage){
    return (data->images[ThresholdImage] = data->lt.apply(grayImage));
  }
  const ImgBase *CalibrationObject::computeAndStoreDilatedImage(const ImgBase *thresholdedImage){
    if(getPropertyValue("morph thresholded image") == "on"){
      return (data->images[DilatedImage] = data->mo.apply(thresholdedImage) );
    }else{
      return (data->images[DilatedImage] = thresholdedImage);
    }
  }


  const ImgBase * CalibrationObject::findPoints(const ImgBase *sourceImage,
                                                std::vector<Point32f> &cogs,
                                                std::vector<Rect> &bbs){
    if(!sourceImage) throw ICLException("CalibrationObject::find: source image is null");
    if(!sourceImage->getChannels()) throw ICLException("CalibrationObject::find: source image has zero channels");
    if(!sourceImage->getDim()) throw ICLException("CalibrationObject::find: source image has null dimension");

    const ImgBase *im = storeInputImage(sourceImage);
    im = computeAndStoreGrayImage(im);
    im = computeAndStoreThresholdImage(im);
    im = computeAndStoreDilatedImage(im);
    
    const std::vector<ImageRegion> &rsd = data->rd.detect(im);
    const float minFF = parse<float>(getPropertyValue("min formfactor"));
    for(unsigned int i=0;i<rsd.size();++i){
      if(rsd[i].getFormFactor() <= minFF){
        cogs.push_back(rsd[i].getCOG());
        bbs.push_back(rsd[i].getBoundingBox());
      }
    }
    
    return im;
  }

  
  CalibrationObject::CalibrationResult CalibrationObject::find(const ImgBase *sourceImage){
    std::vector<Point32f> cogs;
    std::vector<Rect> bbs;
    
    const ImgBase *image = findPoints(sourceImage,cogs,bbs);
    
    data->grid->update(cogs,bbs,image ? image->asImg<icl8u>() : 0);
    float err = data->grid->applyCalib(image->getSize(),data->cam);
    CalibrationResult result = { data->cam, err };
    return result;
  }

  void CalibrationObject::visualizeGrid2D(ICLDrawWidget &d){
    data->grid->visualize2D(d);
  }

  
  CalibrationGrid *CalibrationObject::getCalibrationGrid(){
    return data->grid;
  }
  
  /// provides access to the currently used calibration grid (const)
  const CalibrationGrid *CalibrationObject::getCalibrationGrid() const{
    return data->grid;
  }

}
  




