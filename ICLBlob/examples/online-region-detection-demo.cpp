/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2010 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLBlob/examples/online-region-detection-demo.cpp      **
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

#include <ICLQuick/Common.h>
#include <ICLBlob/RegionDetector.h>
#include <ICLCC/Color.h>
#include <ICLQuick/QuickRegions.h>

// global data (GUI and reference color)
GUI gui("hsplit");
std::vector<double> refcol(3);
GenericGrabber grabber;
RegionDetector rd(100,1<<20,255,255);

// reference color callback (ref. color is
// updated by mouse-click/drag)
void click_color(const MouseEvent &evt){
  if(evt.isLeft() && evt.getColor().size() == 3){
    refcol = evt.getColor();
  }
}

// initialization (create gui and install callback)
void init(){
  rd.setConfigurableID("rd");
  gui << "draw[@handle=draw@minsize=16x12]"
      << "prop(rd)";
  gui.show();
  gui["draw"].install(new MouseHandler(click_color));
  grabber.init(FROM_PROGARG("-input"));
  grabber.setIgnoreDesiredParams(true);
}


// working loop
void run(){
  
  Img32f im = cvt(grabber.grab());
  Img32f cm = colormap(im,refcol[0],refcol[1],refcol[2]);
  Img32f bi = thresh(cm,240);
  Img8u im8u = cvt8u(bi);
  
  // create a region detector

  const std::vector<ImageRegion> &rs = rd.detect(&im8u);

  gui_DrawHandle(draw);

  // visualization
  draw = im;
  draw->lock();
  draw->reset();
  draw->color(0,100,255);
  for(unsigned int i=0;i<rs.size();++i){
    // obtain region information (boundary pixels here)
    draw->linestrip(rs[i].getBoundary());
  }
  draw->unlock();
  draw.update();
}

// default main function
int main(int n, char **ppc){
  return ICLApp(n,ppc,"-input(2)",init,run).exec();
}
