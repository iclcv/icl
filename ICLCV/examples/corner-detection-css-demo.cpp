/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2012 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLCV/examples/corner-detection-css-demo.cpp         **
** Module : ICLCV                                                **
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

// Copyright 2009 Erik Weitnauer
/// Visulalization of the CSS corner detection algorithm.

#include <ICLCV/Common.h>
#include <ICLCore/Color.h>
#include <ICLCV/RegionDetector.h>
#include <ICLCore/Line.h>
#include <ICLCV/CornerDetectorCSS.h>

HBox gui;

Mutex mutex;
Color refColor = Color(255,255,255);
CornerDetectorCSS css;
GenericGrabber *grabber = 0;

void mouse(const MouseEvent &event){
  if(event.isPressEvent()){
    if(event.getColor().size() == 3) {
      Mutex::Locker l(mutex);
      for(int i=0;i<3;++i) refColor[i] = event.getColor()[i];
      std::cout << "new Ref-Color:"  << refColor.transp() << std::endl;
    }
  }
}

void init(){
  if(pa("-r")){
    GenericGrabber::resetBus();
  }
  
  css.setConfigurableID("css");
  css.setPropertyValue("debug-mode","on");

  gui << ( VBox()
           << ( HBox() 
                << CamCfg("")
                <<  Combo("color image,binary image").handle("vis")
              )
           << FSlider(0,1,0.03).out("t").label("threshold")
         )
      << ( VSplit()
           << ( HBox()
                << Draw().handle("img_in").minSize(16,12)
                << Draw().handle("img1").minSize(16,12)
                << Draw().handle("img2").minSize(16,12)
                )
           << Draw().handle("img3").minSize(16,12)
          )
      << Prop("css").label("CSS Params").minSize(14,12)
      << Show();

  gui["img_in"].install(mouse);

  
  // grabber
  grabber = new GenericGrabber(pa("-i"));
  grabber->useDesired<Size>(pa("-size"));
  grabber->useDesired(depth8u);
}

template<class T>
    void thresh(const Img<T> &input, Img8u &result, float t,const Color &ref){
  Mutex::Locker l(mutex);
  result.setChannels(1);
  result.setSize(input.getSize());
  const Channel<T> cs[3] = {input[0], input[1], input[2]};
  Channel8u dst = result[0];
  t *= 3*255*255;
  for(int i=0;i<cs[0].getDim();++i){
    int d = 0;
    for(int c=0;c<3;++c){
      d += pow( double(cs[c][i] - ref[c]) ,2.0); 	
    } 
    dst[i] = 255 * (d < t);
  }
} 

template <class T>
inline std::string to_string (const T& t)
{
  std::stringstream ss;
  ss.precision(3);
  ss << t;
  return ss.str();
}



void drawInput(ICLDrawWidget *w, const CornerDetectorCSS::DebugInformation &css_inf) {
	w->color(255,0,0,255); w->fill(255,0,0,255);
  w->linestrip(css_inf.boundary,true);
  for (unsigned int i=0; i<css_inf.corners.size(); i++) {
    w->color(255,50,50,255); w->fill(255,50,50,255);
    w->ellipse(css_inf.corners[i].x-2, css_inf.corners[i].y-2,4,4);
  }
}

void drawStep1(ICLDrawWidget *w, const CornerDetectorCSS::DebugInformation &css_inf) {
  w->color(255,0,0);
  w->linestrip(css_inf.boundary,true);
	// draw smoothed boundary.
	float x,y,lx=-1,ly=-1;
	int n = css_inf.smoothed_boundary.size();
	for (int i=0; i<n; i++) {
		if (i<css_inf.offset || i>=n-css_inf.offset) continue;
		else w->color(0,0,0);
		x = css_inf.smoothed_boundary[i].x; y = css_inf.smoothed_boundary[i].y;
	  if (lx != -1) w->line(lx,ly,x,y);
	  lx = x; ly = y;
	}
	w->color(0,0,0);
	w->ellipse(css_inf.smoothed_boundary[css_inf.offset].x-1, css_inf.smoothed_boundary[css_inf.offset].y-1,2,2);
}

void drawStep3(ICLDrawWidget *w, const CornerDetectorCSS::DebugInformation &css_inf) {
	w->rel(); // positions between [0,1]
	// draw kurvature
	int n = css_inf.kurvature.size();
	float lx=0, ly=0, x, y;
	float s=0.25; // scaling
	
	for (int i=0; i<n; i++) {
		if (i<css_inf.offset || i>=n-css_inf.offset) w->color(180,180,180);
		else w->color(0, 0, 0);
		x = float(i)/n;
		y = css_inf.kurvature[i]*s;
		w->line(lx,1-ly,x,1-y);
		lx = x; ly = y;
	}
	// draw extrema
	w->nofill();
	for (unsigned int i=0; i<css_inf.extrema.size(); i++) {
		if (i % 2 == 0) w->color(0,255,0); else w->color(0,0,255);
		w->ellipse(float(css_inf.extrema[i])/n-0.005,1-s*css_inf.kurvature[css_inf.extrema[i]]-0.005,0.01,0.01);
	}
	// draw extrema without minima and rond corners
	w->color(200,50,50);
	for (unsigned int i=0; i<css_inf.maxima_without_round_corners.size(); i++) {
		w->ellipse(float(css_inf.maxima_without_round_corners[i])/n-0.005,1-s*css_inf.kurvature[css_inf.maxima_without_round_corners[i]]-0.005,0.01,0.01);
	}
	
	int count = 0;
	// draw extrema without false corners
	for (unsigned int i=0; i<css_inf.maxima_without_false_corners.size(); i++) {
		float x = float(css_inf.maxima_without_false_corners[i])/n;
		float y = s*css_inf.kurvature[css_inf.maxima_without_false_corners[i]];
		if (css_inf.maxima_without_false_corners[i]>=css_inf.offset &&
		    css_inf.maxima_without_false_corners[i]<n-css_inf.offset) {
			w->color(0,0,0);
			w->text(to_string(count++),x,1-y,-1,-1,10);
		} else {
			w->color(180,180,180);
			w->text("x",x,1-y);
		}
	}
	w->abs();
}

void drawStep2(ICLDrawWidget *w, const CornerDetectorCSS::DebugInformation &css_inf) {
	w->color(0,0,0);
  w->linestrip(css_inf.corners,true);
  for (unsigned int i=0; i<css_inf.angles.size(); i++) {
  	w->text("("+to_string(i)+")"+to_string(css_inf.angles[i]), css_inf.corners[i].x, css_inf.corners[i].y,-1,-1,8);
  }
}

void run(){
	// draw handles
  static DrawHandle &h = gui.get<DrawHandle>("img_in");
  static DrawHandle &h1 = gui.get<DrawHandle>("img1");
  static DrawHandle &h2 = gui.get<DrawHandle>("img2");
  static DrawHandle &h3 = gui.get<DrawHandle>("img3");
	// images: get camera input and apply threshold
  const Img8u &image = *grabber->grab()->asImg<icl8u>();
  static Img8u threshedImage;
  static Img8u bgImage1(image.getSize(), 1); bgImage1.clear(0,255);
  static Img8u bgImage2(image.getSize(), 1); bgImage2.clear(0,255);
  static Img8u bgImage3(image.getSize(), 1); bgImage3.clear(0,255);
  thresh(image,threshedImage,gui.get<float>("t"),refColor);
	
	// draw background images
  std::string vis = gui["vis"];
  h = (vis == "color image") ? &image : &threshedImage;
	h1 = &bgImage1; h2 = &bgImage2; h3 = &bgImage3;

	// lock the DrawWidgets before drawing
  ICLDrawWidget *w = *h,  *w1 = *h1, *w2 = *h2, *w3 = *h3;

	// detect regions
  static RegionDetector d(100,200000,255,255);
  const std::vector<ImageRegion> &rs = d.detect(&threshedImage);

  // iterate over all regions and draw information onto the DrawWidgets
  for(unsigned int i=0;i<rs.size();++i) {
    const vector<Point> &boundary = rs[i].getBoundary();
    css.detectCorners(boundary);
    const CornerDetectorCSS::DebugInformation &css_inf = css.getDebugInformation();
//    cout << "contour points: " << boundary.size() << endl;
//    cout << "extrema: " << css_inf.extrema.size() << endl;
//    cout << "maxima: " << css_inf.maxima.size() << endl;
//    cout << "maxima_without_round_corners: " << css_inf.maxima_without_round_corners.size() << endl;
//    cout << "maxima_without_false_corners: " << css_inf.maxima_without_false_corners.size() << endl;
//    cout << "corners: " << css_inf.corners.size() << endl;
    drawInput(w, css_inf);
    drawStep1(w1, css_inf);
    drawStep2(w2, css_inf);
    drawStep3(w3, css_inf);
    
    Point32f cog = rs[i].getCOG();
    w->color(255,0,0,255); w->fill(255,0,0,255);
    w->ellipse(cog.x-1, cog.y-1,2,2);  
  }
	
	// update the draw widgets
  h.render(); 
  h1.render();
  h2.render();
  h3.render();
  Thread::msleep(100);
}


int main(int n, char **ppc){
  paex
  ("-i","defines input device and parameters")
  ("-s","defines image size to use")
  ("-r","if given, the dc-bus is resetted automatically before use");
  return ICLApp(n,ppc,"[m]-input|-i(device,device-params) -size|-s(Size=VGA) -reset|-r",init,run).exec();
}
