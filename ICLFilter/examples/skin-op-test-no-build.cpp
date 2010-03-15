/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2010 Neuroinformatics, CITEC                 **
**                         University of Bielefeld                 **
**                Contact: nivision@techfak.uni-bielefeld.de       **
**                Website: www.iclcv.org                           **
**                                                                 **
** File   : ICLFilter/examples/skin-op-test-no-build.cpp           **
** Module : ICLFilter                                              **
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
*********************************************************************/

#include <iclSkinOp.h>
#include <iclImg.h>
#include <iclFileGrabber.h>
#include <iclFileWriter.h>

using namespace icl;

int main(){
  // Variable initilization
  const ImgBase* refImg, *img;
  ImgBase *skinImg = imgNew();
  FileGrabber r("demoImages/skinRefImg.ppm");
  FileGrabber r2("demoImages/skinImg.ppm");
  FileWriter w("skinMask.pgm");
  std::vector<float> vecparameter;
  SkinOp skinFilter, skinFilter2;
  
  // --------------------------------------------------------------
  // ---- Train skin color filter 
  // --------------------------------------------------------------
  // Load skin reference image
  refImg = r.grab();
  
  // Apply skin parameter
  skinFilter.train(refImg);
  
  // --------------------------------------------------------------
  // ---- Detect skin color 
  // --------------------------------------------------------------
  // Load skin test image
  img = r2.grab();
  
  // Set parabola parameter
  vecparameter.push_back(101);
  vecparameter.push_back(65);
  vecparameter.push_back(0.1);
  vecparameter.push_back(103);
  vecparameter.push_back(88);
  vecparameter.push_back(-0.09);
  skinFilter2.setParameter(vecparameter);
  
  // Detect skin color 
  skinFilter2.apply(img, &skinImg);
  
  //Write skin mask
  w.write(skinImg);
  
  // Destroy objects and return
  return 0;
}
