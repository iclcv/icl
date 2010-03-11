/*************************************************************************** 
**                                                                        **
** Copyright (C) 2006-2010 neuroinformatics group (vision)                **
**                         University of Bielefeld                        **
**                         nivision@techfak.uni-bielefeld.de              **
**                                                                        **
** This file is part of the ICLBlob module of ICL                         **
**                                                                        **
** Commercial License                                                     **
** Commercial usage of ICL is possible and must be negotiated with us.    **
** See our website www.iclcv.org for more details                         **
**                                                                        **
** GNU General Public License Usage                                       **
** Alternatively, this file may be used under the terms of the GNU        **
** General Public License version 3.0 as published by the Free Software   **
** Foundation and appearing in the file LICENSE.GPL included in the       **
** packaging of this file.  Please review the following information to    **
** ensure the GNU General Public License version 3.0 requirements will be **
** met: http://www.gnu.org/copyleft/gpl.html.                             **
**                                                                        **
***************************************************************************/ 

#include <ICLQuick/Common.h>
#include <ICLBlob/RegionDetector.h>
#include <ICLCC/Color.h>
#include <ICLQuick/QuickRegions.h>

// global data (GUI and reference color)
GUI gui("draw[@handle=draw@minsize=16x12]");
std::vector<double> refcol(3);
GenericGrabber grabber;
// reference color callback (ref. color is
// updated by mouse-click/drag)
void click_color(const MouseEvent &evt){
  if(evt.isLeft() && evt.getColor().size() == 3){
    refcol = evt.getColor();
  }
}

// initialization (create gui and install callback)
void init(){
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
  
  // create a region detector
  static RegionDetector rd(100,1<<20,255,255);
  const std::vector<icl::Region> &rs = rd.detect(&bi);

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
