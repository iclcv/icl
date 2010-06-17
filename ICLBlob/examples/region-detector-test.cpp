/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2010 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLBlob/examples/region-detector-test.cpp              **
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

#include <ICLQuick/Quick.h>
#include <ICLQuick/QuickRegions.h>
#include <ICLBlob/RegionDetector.h>


int main(){

  ImgQ a = create("parrot");

  a = scale(a,0.5);

  a = gray(a);

  a = thresh(a,128);

  a = filter(a,"median");

  a.setROI(Rect(20,20,100,300));
  vector<vector<Point> > px = pixels(a,0,2<<20,255,255);
  vector<RegionPCAInfo> pcas = pca(a,0,2<<20,255,255);
  
  ImgQ b = a*0;
  a = a|b|b;
  for(unsigned int i=0;i<px.size();++i){
    color(0,255,0);
    pix(a,px[i]);
    
    color(0,0,255);
    draw(a,pcas[i]);
  }
  show(a);
  
  /** TEST 2
  a.setROI(Rect(20,20,100,300));

  ERROR_LOG("centers");
  vector<Point> cs = centers(a);
  ERROR_LOG("bbs");
  vector<Rect> bs = boundingboxes(a);
  ERROR_LOG("pcas");
  vector<RegionPCAInfo> pcas = pca(a);
  ERROR_LOG("boundaries");

  
  

  ImgQ xxx = rgb(a);
  xxx = xxx*0.2;
  ImgQ xxx2 = copy(xxx);
  color(255,0,0);
  pix(xxx,118-a.getROI().x,57-a.getROI().y);
  show((xxx,zeros(1,1,1),xxx2));
  

  vector<vector<Point> > bounds = boundaries(a);

  a.setFullROI();
 
  a = rgb(a);
  color(255,0,0); pix(a,cs);
  for(unsigned int i=0;i<bs.size();++i){
    color(255,255,0); rect(a,bs[i]);
  }
  color(0,0,255); draw(a,pcas);

  for(unsigned int i=0;i<bounds.size();++i){
    color(0,255,255); pix(a,bounds[i]);
  }
  show(a);
  */
  
  

  

  return 0;
}
