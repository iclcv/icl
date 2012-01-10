/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2011 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLGeom/examples/kinect-rgbd-calib.cpp                 **
** Module : ICLGeom                                                **
** Authors: Andre Ueckermann                                       **
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
#include <ICLUtils/ConfigFile.h>
#include <ICLMarkers/FiducialDetector.h>
#include <ICLGeom/PointNormalEstimation.h>
#include <ICLUtils/Homography2D.h>

PointNormalEstimation *pEst;

ImageUndistortion udistRGB;
ImageUndistortion udistIR;

GUI gui("hsplit");
GenericGrabber grabDepth, grabColor;

FiducialDetector *fid = 0;
FiducialDetector *fid2 = 0;

std::vector<std::vector<float> > correspondences;
std::vector<std::string> names;
FixedMatrix<float,3,3> H;

Img8u matchImage(Size(320,240), formatRGB);

void init(){
  Size size = pa("-size");
  pEst=new PointNormalEstimation(size);
  matchImage.setSize(size);
  grabDepth.init("kinectd","kinectd=0");
  grabColor.init("kinectc","kinectc=0");
  grabDepth.useDesired(depth32f, size, formatMatrix);
  grabColor.useDesired(depth32f, size, formatMatrix);
  
	fid = new FiducialDetector(pa("-m").as<std::string>(), 
                         pa("-m",1).as<std::string>(), 
                         ParamList("size",(*pa("-m",2)) ) );
                         
  fid2 = new FiducialDetector(pa("-m").as<std::string>(), 
                         pa("-m",1).as<std::string>(), 
                         ParamList("size",(*pa("-m",2)) ) );

  fid->setConfigurableID("fid");
  fid2->setConfigurableID("fid2");
    
    
  GUI controls("vbox[@minsize=12x2]");
  controls << "togglebutton(pause, run)[@out=paused]"
           << "button(add points)[@handle=addPoints]"
           << "button(calculate homography)[@handle=calcHomo]"
           << "button(save homography)[@handle=saveHomo]"
           << "button(clear points and reset)[@handle=clearPoints]";
      
  gui << "draw3D[@handle=depth@minsize=16x12]"
      << "draw3D[@handle=color@minsize=16x12]"
      << "draw3D[@handle=ir@minsize=16x12]"
      << "draw3D[@handle=match@minsize=16x12]"
      << controls
      << (GUI("vbox[@maxsize=16x100]") 
      << ("combo(" + fid->getIntermediateImageNames() + ")"
          "[@maxsize=100x2@handle=vis@label=color image]")
      << "prop(fid)")
      << (GUI("vbox[@maxsize=16x100]") 
      << ("combo(" + fid2->getIntermediateImageNames() + ")"
          "[@maxsize=100x2@handle=vis2@label=infrared image]")
      << "prop(fid2)")
      << "!show";
        
  try{
    fid->setPropertyValue("thresh.algorithm","tiled linear");
    
    fid2->setPropertyValue("thresh.algorithm","region mean");
    fid2->setPropertyValue("matching algorithm","gray ncc");
    fid2->setPropertyValue("thresh.global threshold","-0.7");
    fid2->setPropertyValue("thresh.mask size","19");
    fid2->setPropertyValue("quads.minimum region size","360");

  }catch(ICLException &e){
    WARNING_LOG("exception caught while setting initial parameters: " << e.what());
  }
  
	gui_DrawHandle3D(depth);
	depth->setRangeMode(ICLWidget::rmAuto);
	gui_DrawHandle3D(match);
	match->setRangeMode(ICLWidget::rmAuto);
		
  if(pa("-rgb-udist")){
    string fn1 = pa("-rgb-udist");
    udistRGB=ImageUndistortion(fn1);
    grabColor.enableUndistortion(udistRGB);
    std::cout<<"RGB-UNDISTORTION: --- "<<fn1<<" --- "<<std::endl<<udistRGB<<std::endl;
  }
  if(pa("-ir-udist")){
    string fn2 = pa("-ir-udist");
    udistIR=ImageUndistortion(fn2);
    grabDepth.enableUndistortion(udistIR);
    std::cout<<"IR-UNDISTORTION: --- "<<fn2<<" --- "<<std::endl<<udistIR<<std::endl;
  }
	H.at(0,0)=1;
	H.at(1,1)=1;
	H.at(2,2)=1;	
}

void run(){
  gui_ButtonHandle(addPoints);
  gui_ButtonHandle(clearPoints);
  gui_ButtonHandle(calcHomo);
  gui_ButtonHandle(saveHomo);
    
  Size size = pa("-size");
  static Img32f C,IR,D;
  Img32f IRmed;

  grabDepth.grab(bpp(D));
  grabDepth.grab();
  grabColor.disableUndistortion();
  grabColor.setProperty("format","Color Image (24Bit RGB)");
  if(pa("-rgb-udist")){
    grabColor.enableUndistortion(udistRGB);
  }
  grabColor.grab();
  grabColor.grab(bpp(C));
  grabColor.disableUndistortion();
  grabColor.setProperty("format","IR Image (10Bit)");
  if(pa("-ir-udist")){
    grabColor.enableUndistortion(udistIR);
  }
  grabColor.grab();
  grabColor.grab(bpp(IR));
       
  if(size==Size::QVGA){
    pEst->setMedianFilterSize(3);
    pEst->setDepthImage(IR);
    pEst->medianFilter();
    IRmed=pEst->getFilteredImage();
  }else if(size==Size::VGA){
    pEst->setMedianFilterSize(5);
    pEst->setDepthImage(IR);
    pEst->medianFilter();
    IRmed=pEst->getFilteredImage(); 
  }else{
    IRmed=IR;
  } 

  gui["ir"] = IRmed; 
  gui["depth"] = D;
  gui["color"] = C;
  gui["match"] = matchImage;
        
  const std::vector<Fiducial> &fids = fid->detect(&C);
  gui_DrawHandle3D(color);
  color = fid->getIntermediateImage(gui["vis"]);

  color->lock();
  color->reset();
  color->reset3D();
  color->linewidth(2);
  for(unsigned int i=0;i<fids.size();++i){
    color->color(255,0,0,255);
    color->linestrip(fids[i].getCorners2D());
    color->color(0,100,255,255);
    color->text(fids[i].getName(),fids[i].getCenter2D().x, fids[i].getCenter2D().y,9);
    color->color(0,255,0,255);
    float a = fids[i].getRotation2D();
    color->line(fids[i].getCenter2D(), fids[i].getCenter2D() + Point32f( cos(a), sin(a))*100 );
  }
  color->unlock();
 
  const std::vector<Fiducial> &fids2 = fid2->detect(&IRmed);
  gui_DrawHandle3D(ir);
  ir = fid2->getIntermediateImage(gui["vis2"]);

  ir->lock();
  ir->reset();
  ir->reset3D();
  ir->linewidth(2);
  for(unsigned int i=0;i<fids2.size();++i){
    ir->color(255,0,0,255);
    ir->linestrip(fids2[i].getCorners2D());
    ir->color(0,100,255,255);
    ir->text(fids2[i].getName(),fids2[i].getCenter2D().x, fids2[i].getCenter2D().y,9);
    ir->color(0,255,0,255);
    float a = fids2[i].getRotation2D();
    ir->line(fids2[i].getCenter2D(), fids2[i].getCenter2D() + Point32f( cos(a), sin(a))*100 );
  }
  ir->unlock();

  for(int y=0; y<size.height; y++){
    for(int x=0; x<size.width; x++){
      float az = H(0,2)*x + H(1,2) * y + H(2,2);
      int ax = round(( H(0,0)*x + H(1,0) * y + H(2,0) ) / az);
      int ay = round(( H(0,1)*x + H(1,1) * y + H(2,1) ) / az);
      matchImage(x,y,1)=D(x,y,0);
      if(ax<0 || ax>=size.width || ay<0 || ay>=size.height){
        matchImage(x,y,0)=0;
      }else{
        matchImage(x,y,0)=C(ax,ay,0);
      }
    }
  }
  gui["match"] = matchImage;
    
  gui["depth"].update();
  gui["color"].update();
  gui["ir"].update();
  gui["match"].update();
        
  while(gui["paused"]){}
   
  if(clearPoints.wasTriggered()){
    correspondences.clear();
    names.clear();
    H=H.id();
  }
   
  if(addPoints.wasTriggered()){
    std::cout<<"COLOR POINTS: "<<std::endl;
	  for(unsigned int i=0; i<fids.size(); i++){  		    
      std::cout<<fids[i].getName()<<", "<<fids[i].getCenter2D().x<<"x"<<fids[i].getCenter2D().y<<std::endl;
    }
    std::cout<<"IR POINTS: "<<std::endl;
    for(unsigned int i=0; i<fids2.size(); i++){ 		    
      std::cout<<fids2[i].getName()<<", "<<fids2[i].getCenter2D().x<<"x"<<fids2[i].getCenter2D().y<<std::endl;
    }
        	    
    for(unsigned int a=0;a<fids.size(); a++){
      for(unsigned int b=0; b<fids2.size(); b++){
        if(fids[a].getName()==fids2[b].getName()){
          std::vector<float> data;
          data.push_back(fids[a].getCenter2D().x);
          data.push_back(fids[a].getCenter2D().y);
          data.push_back(fids2[b].getCenter2D().x);
          data.push_back(fids2[b].getCenter2D().y);
          correspondences.push_back(data);
              
          /*/////////////////////////////////////////////////////  
          for(int i=0; i<fids[a].getCorners2D().size(); i++){
            data.clear();
            data.push_back(fids[a].getCorners2D().at(i).x);
            data.push_back(fids[a].getCorners2D().at(i).y);
            data.push_back(fids2[b].getCorners2D().at(i).x);
            data.push_back(fids2[b].getCorners2D().at(i).y);
            correspondences.push_back(data);
          }
          *//////////////////////////////////////////////////////
        	
          names.push_back(fids[a].getName());
          break;
        }
      }
    }
    std::cout<<correspondences.size()<<"CORRESPONDENCES: "<<std::endl;
    for(unsigned int c=0;c<correspondences.size(); c++){
      std::cout<<"Color: "<<correspondences[c][0]<<"x"<<correspondences[c][1]<<" , IR: "<<correspondences[c][2]<<"x"<<correspondences[c][3]<<std::endl;
    }
  }
   
  if(calcHomo.wasTriggered()){
    if(correspondences.size()>3){
      Point32f* A = new Point32f[correspondences.size()];
      Point32f* B = new Point32f[correspondences.size()];
      for (unsigned int i=0;i<correspondences.size(); i++){
        A[i].x=correspondences[i][0];
        A[i].y=correspondences[i][1];
        B[i].x=correspondences[i][2];
        B[i].y=correspondences[i][3];
        std::cout<<A[i]<<" , "<<B[i]<<std::endl;
      }
      Homography2D *hom = new Homography2D(A,B,correspondences.size());//, alg);
      H=*hom;
      std::cout<<H<<std::endl;
      
      float errorsum=0;
      for(unsigned int i=0; i<correspondences.size(); i++){
        float xi = correspondences[i][0];
        float yi = correspondences[i][1];
        
        float x = correspondences[i][2];
        float y = correspondences[i][3];
        float az = H(0,2)*x + H(1,2) * y + H(2,2);
        int ax = round(( H(0,0)*x + H(1,0) * y + H(2,0) ) / az);
        int ay = round(( H(0,1)*x + H(1,1) * y + H(2,1) ) / az);
        float dist=sqrt(sqr(xi-ax)+sqr(yi-ay));
        errorsum+=dist;
        std::cout<<"Point "<<i+1<<"("<<names[i]<<") Error (Distance): "<<dist<<std::endl;//
      }
      std::cout<<"Full Error: "<<errorsum<<std::endl;
    }else{
      std::cout<<"Too few points for calculation, please add points."<<std::endl;
    }
  }
      	
  if(saveHomo.wasTriggered()){
    try{
      ConfigFile f;
		  f.setPrefix("config.");
      f["homogeneity"] = H;
      std::string filename = pa("-output");
		  f.save(filename);
	  }catch(...){}
  }
}

int main(int n, char **ppc){
  return ICLApp(n,ppc,"-size|-s(Size=VGA) -output|-o(output-xml-file-name=homogeneity.xml) -rgb-udist(fn1) -ir-udist(fn2) -marker-type|-m(type=art,whichToLoad=art/*.png,size=50x50)",init,run).exec();
}
