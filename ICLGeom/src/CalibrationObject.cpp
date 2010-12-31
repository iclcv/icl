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
    
    std::map<std::string,const ImgBase*> images;
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
    addProperty("visualized image","menu","input,gray,threshold,dilated","input"); 
    
    data->grid = grid;
    grid->initializeSceneObject(*this);
    
    /// remove properties, that are not appropriate here
    deactivateProperty("^local threshold.UnaryOp");
    deactivateProperty("^local threshold.gamma slope");
    deactivateProperty("^region detector.create region graph");
    deactivateProperty("^region detector\\..* value");
    deactivateProperty("^region detector\\.CSS\\..*");
  }  
  
  void CalibrationObject::setImage(const std::string &name, const ImgBase *image){
    data->images[name] = image;
  }

  const ImgBase * CalibrationObject::findPoints(const ImgBase *sourceImage,
                                                std::vector<Point32f> &cogs,
                                                std::vector<Rect> &bbs){
    data->images["input"] = sourceImage;
    const ImgBase *image = sourceImage;
    if(!image) throw ICLException("CalibrationObject::find: source image is null");
    if(!image->getChannels()) throw ICLException("CalibrationObject::find: source image has zero channels");
    if(!image->getDim()) throw ICLException("CalibrationObject::find: source image has null dimension");
    
    if(image->getDepth() != depth8u || image->getChannels() != 1){
      ensureCompatible(&data->gray8u,depth8u,image->getSize(),formatGray);
      icl::cc(image,data->gray8u);
      image = data->gray8u;
    }

    data->images["gray"] = image;
    
    image = data->lt.apply(image);
    data->images["threshold"] = image;

    if(getPropertyValue("morph thresholded image") == "on"){
      image = data->mo.apply(image);
      data->images["dilated"] = image;
    }else{
      data->images["dilated"] = 0;
    }

    
    const std::vector<ImageRegion> &rsd = data->rd.detect(image);
    const float minFF = parse<float>(getPropertyValue("min formfactor"));
    for(unsigned int i=0;i<rsd.size();++i){
      if(rsd[i].getFormFactor() <= minFF){
        cogs.push_back(rsd[i].getCOG());
        bbs.push_back(rsd[i].getBoundingBox());
      }
    }
    
    return image;
  }
  
  CalibrationObject::CalibrationResult CalibrationObject::find(const ImgBase *sourceImage){
    std::vector<Point32f> cogs;
    std::vector<Rect> bbs;
    
    const ImgBase *image = findPoints(sourceImage,cogs,bbs);
    
    data->grid->update(cogs,bbs,*image->asImg<icl8u>());
    float err = data->grid->applyCalib(image->getSize(),data->cam);
    CalibrationResult result = { data->cam, err };
    return result;
  }

  void CalibrationObject::visualize2D(ICLDrawWidget &d, bool unlockWidget){
    std::string visImage = getPropertyValue("visualized image");
    d.setImage(data->images[visImage]);
    d.lock();
    d.reset();
    data->grid->visualize2D(d);
    if(unlockWidget) d.unlock();
  }

  
  CalibrationGrid *CalibrationObject::getCalibrationGrid(){
    return data->grid;
  }
  
  /// provides access to the currently used calibration grid (const)
  const CalibrationGrid *CalibrationObject::getCalibrationGrid() const{
    return data->grid;
  }

}
  




