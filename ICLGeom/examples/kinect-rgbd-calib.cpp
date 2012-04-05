/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2012 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLGeom/examples/kinect-rgbd-calib.cpp                 **
** Module : ICLGeom                                                **
** Authors: Andre Ueckermann, Christof Elbrechter                  **
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

#include <ICLCore/Random.h>
#include <ICLUtils/ConsoleProgress.h>
#include <ICLUtils/SimplexOptimizer.h>


PointNormalEstimation *pEst;

ImageUndistortion udistRGB;
ImageUndistortion udistIR;

GUI gui("hsplit");
GenericGrabber grabDepth, grabColor;

SmartPtr<FiducialDetector> fid,fid2;

std::vector<std::vector<float> > correspondences;
std::vector<std::string> names;

//FixedMatrix<float,3,3> H;
Homography2D H;

Img32f matchImage(Size(320,240), formatRGB);

GUI fidDetectorPropertyGUI("hsplit");

float compute_homograpy_error(const Point32f *as, 
                              const Point32f *bs, 
                              int N, const Homography2D &h,
                              int &outlierCounter,
                              int outlierThreshold,
                              std::vector<int> &inliners){
  float err = 0;
  for(int i=0;i<N;++i){
    float e = as[i].distanceTo(h.apply(bs[i]));
    if(e < outlierThreshold){
      err += e;
      inliners.push_back(i);
    }else{
      ++outlierCounter;
    }
  }
  if(outlierCounter == N) return 1E20;
  return err / (N-outlierCounter);
}

Homography2D ransacBasedHomography(const Point32f *as, 
                                   const Point32f *bs, 
                                   int N,
                                   std::vector<int> *inliners=0,
                                   int numSteps=100000,
                                   float outlierThreshold=2){
  if(N < 5) throw ICLException("unabled to apply ransac based homography "
                               "estimation on less than 5 points");
  
  Homography2D hBest;
  float errBest = 0;
  int numOutliersBest = 0;
  std::vector<int> bestInliners;
  
  std::cout << "starting ransac ..." << std::endl;
  URandI ridx(N-1);
  
  progress_init("running ransac optimization");
  
  for(int i=0;i<numSteps;++i){
    if(!(i%100)){
      progress(i,numSteps-1);
    }
    int a = ridx;
    int b = ridx;
    while(b == a) b = ridx;
    int c = ridx;
    while(c==a || c==b) c = ridx;
    int d = ridx;
    while(d==a || d==b || d==c) d = ridx;

    
    const Point32f pa[4] = { as[a], as[b], as[c], as[d] };
    const Point32f pb[4] = { bs[a], bs[b], bs[c], bs[d] };
    
    Homography2D h(pa,pb,4);
    int numOutliers = 0;
    std::vector<int> inlinerIndices;
    float err = compute_homograpy_error(as,bs,N,h, numOutliers, outlierThreshold, inlinerIndices);

    if(!i 
       || (numOutliers < numOutliersBest) 
       || (numOutliers==numOutliersBest && err<errBest)) {
      hBest = h;
      errBest = err;
      numOutliersBest = numOutliers;
      bestInliners = inlinerIndices;
    }    
  }
  progress_finish();
  std::cout << "\n#outliers:" << numOutliersBest
            << "\n error:" << errBest
            << "\n";

  if(inliners) *inliners = bestInliners;
  return hBest;
}


struct Err{
  const std::vector<int> &inliners;
  const Point32f *as;
  const Point32f *bs;
  float f(const std::vector<float> &params) const{
    Homography2D h;
    std::copy(params.begin(), params.end(), h.begin());
    h(2,2) = 1;
    //    SHOW(h);
    float err = 0;
    for(unsigned int i=0;i<inliners.size();++i){
      err += as[inliners[i]].distanceTo(h.apply(bs[inliners[i]]));
    }
    return err / inliners.size();
  }
};

//void step_callback(const SimplexOptimizer<float,std::vector<float> >::Result &res){
//  std::cout << "  simplex error: " << res.fx << std::endl;
//}

Homography2D simplexBasedHomography(const Point32f *as, 
                                    const Point32f *bs, 
                                    int N){
  typedef SimplexOptimizer<float,std::vector<float> > Simplex;
  std::vector<int> inliners;
  Homography2D h = ransacBasedHomography(as,bs,N,&inliners,10000);
  Err err = {inliners, as, bs};

  Simplex simplex(function(err,&Err::f), 8, 1E6, 1.E-5, 1.E-15);
  std::cout << "running simplex optimization " << std::endl;
  Simplex::Result res = simplex.optimize(std::vector<float>(h.begin(), h.begin()+8));
  std::cout << "simplex error: " << res.fx << std::endl;
  std::copy(res.x.begin(), res.x.end(),h.begin());
  return h;  
}

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
      
  gui << (GUI("vsplit")
          << "draw[@handle=color@minsize=16x12@label=color image]"
          << "draw[@handle=ir@minsize=16x12@label=IR image]"
         )
      << (GUI("vsplit")
          << "draw[@handle=match@minsize=16x12@label=result visualization]"
          << "draw[@handle=depth@minsize=16x12@label=depth image]"
         )
      << ( GUI("vbox[@minsize=12x2]")
           << "togglebutton(pause, run)[@out=paused]"
           << ("combo(R/G mapped_RGB/depth,|mapped_RGB - IR|,!R/B mapped_RGB/IR)"
               "[@handle=resultVis@label=result visualization method@maxsize=99x3]")
           << "combo(least squares,ransac, ransac+simplex)[@handle=optMethod@label=optimization method]"
           << "checkbox(use corners,unchecked)[@out=useCorners]"
           << "button(add points)[@handle=addPoints]"
           << "button(calculate homography)[@handle=calcHomo]"
           << "button(save homography)[@handle=saveHomo]"
           << "button(clear points and reset)[@handle=clearPoints]"
           //  << (GUI("hbox")
               << "button(more ...)[@handle=more]" 
           //    << "camcfg"
           //    )
           )
      << "!show";
  
  fidDetectorPropertyGUI  << (GUI("vbox[@maxsize=16x100]") 
                              << ("combo(" + fid->getIntermediateImageNames() + ")"
                                  "[@maxsize=100x2@handle=vis@label=color image]")
                              << "prop(fid)")
                          << (GUI("vbox[@maxsize=16x100]") 
                              << ("combo(" + fid2->getIntermediateImageNames() + ")"
                                  "[@maxsize=100x2@handle=vis2@label=infrared image]")
                              << "prop(fid2)")
                          << "!create";
  gui["more"].registerCallback(function(&fidDetectorPropertyGUI,&GUI::switchVisibility));
        
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
  
  gui_DrawHandle(depth);
  depth->setRangeMode(ICLWidget::rmAuto);
  gui_DrawHandle(match);
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

void visualizeMatches(DrawHandle &draw, const std::vector<Fiducial> &fids, 
                      FiducialDetector *fd, const std::string &imageName, 
                      bool showCorrespondances){
  draw = fd->getIntermediateImage(imageName);
  draw->lock();
  draw->reset();
  draw->linewidth(1);
  draw->symsize(20);
  for(unsigned int i=0;i<fids.size();++i){
    draw->color(0,100,255,200);
    draw->linestrip(fids[i].getCorners2D());
    draw->color(255,0,0,255);
    draw->sym(fids[i].getCenter2D(),'x');
  }
  draw->color(0,255,0,255);
  draw->fill(0,0,0,0);
  
  if(showCorrespondances){
    for(unsigned int i=0;i<correspondences.size(); ++i){
      const std::vector<float> &v = correspondences[i];
      draw->sym(Point32f(v[0],v[1]), 'o');
    }
  }
  
  draw->unlock();
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
  grabColor.setProperty("format","IR Image (8Bit)");
  if(pa("-do-not-shift-the-IR-image")){
    grabColor.setProperty("shift-IR-image", "accurate");
  }else{
    grabColor.setProperty("shift-IR-image", "off");
  }
  
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
        
  const std::vector<Fiducial> &fids = fid->detect(&C);
  const std::vector<Fiducial> &fids2 = fid2->detect(&IRmed);
  
  static DrawHandle color = gui["color"];
  static DrawHandle ir = gui["ir"];

  visualizeMatches(color,fids, fid.get(), fidDetectorPropertyGUI["vis"], true);
  visualizeMatches(ir,fids2, fid2.get(), fidDetectorPropertyGUI["vis2"], false);

  const Rect r = C.getImageRect();
  Channel32f c = C[0];

  switch(gui["resultVis"].as<int>()){
    case 0:{
      matchImage.setChannels(2);
      std::copy(D.begin(0),D.end(0),matchImage.begin(1));
      Channel32f miC0 = matchImage[0];
      for(int y=0; y<size.height; y++){
        for(int x=0; x<size.width; x++){
          Point p = H.apply_int(Point32f(x,y));
          miC0(x,y) = r.contains(p.x,p.y) ? c(p.x,p.y) : 0.0f;
        }
      }
      break;
    }
    case 1:{
      matchImage.setChannels(1);
      Channel32f miC0 = matchImage[0];
      Channel32f irC0 = IR[0];
      for(int y=0; y<size.height; y++){
        for(int x=0; x<size.width; x++){
          Point p = H.apply_int(Point32f(x,y));
          miC0(x,y) = r.contains(p.x,p.y) ? ::abs(irC0(x,y) - c(p.y,p.y)) : 0.0f;
        }
      }
      break;
    }
    case 2:{
      matchImage.setChannels(2);
      std::copy(IR.begin(0),IR.end(0),matchImage.begin(1));
      Channel32f miC0 = matchImage[0];
      for(int y=0; y<size.height; y++){
        for(int x=0; x<size.width; x++){
          Point p = H.apply_int(Point32f(x,y));
          miC0(x,y) = r.contains(p.x,p.y) ? c(p.x,p.y) : 0.0f;
        }
      }
      break;
    }
  }
      
  matchImage.normalizeAllChannels(Range32f(0,255));

  gui["match"] = matchImage;
    
  gui["depth"].update();
  gui["color"].update();
  gui["ir"].update();
  gui["match"].update();
        
  while(gui["paused"]){}
   
  if(clearPoints.wasTriggered()){
    correspondences.clear();
    names.clear();
    (FixedMatrix<float,3,3>&)H=FixedMatrix<float,3,3>::id();
  }
   
  if(addPoints.wasTriggered()){
    bool useCorners = gui["useCorners"];
    
    std::cout<<"COLOR POINTS: "<<std::endl;
	  for(unsigned int i=0; i<fids.size(); i++){  		    
      std::cout<<fids[i].getName()<<", "<<fids[i].getCenter2D().x<<"x"<<fids[i].getCenter2D().y<<std::endl;
    }
    std::cout<<"IR POINTS: "<<std::endl;
    for(unsigned int i=0; i<fids2.size(); i++){ 		    
      std::cout<<fids2[i].getName()<<", "<<fids2[i].getCenter2D().x<<"x"<<fids2[i].getCenter2D().y<<std::endl;
    }
        	    
    for(unsigned int a=0;a<fids.size(); a++){
      const Fiducial &fa = fids[a];
      for(unsigned int b=0; b<fids2.size(); b++){
        const Fiducial &fb = fids2[b];
        if(fa.getName()==fb.getName()){
          const float d[4] = {fa.getCenter2D().x, fa.getCenter2D().y,
                              fb.getCenter2D().x, fb.getCenter2D().y };
          correspondences.push_back(std::vector<float>(d,d+4));
          names.push_back(fids[a].getName());
          if(useCorners){
            const std::vector<Fiducial::KeyPoint> &ka = fa.getKeyPoints2D();
            const std::vector<Fiducial::KeyPoint> &kb = fb.getKeyPoints2D();
            for(unsigned int i=0;i<ka.size();++i){
              const float d[4] = { ka[i].imagePos.x, ka[i].imagePos.y,
                                   kb[i].imagePos.x, kb[i].imagePos.y };
              correspondences.push_back(std::vector<float>(d,d+4));
              names.push_back(fids[a].getName() + " key-point: " +str(i));
            }
          }
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
    const int Ncorr = correspondences.size();
    if(Ncorr>3){
      std::vector<Point32f> A(Ncorr),B(Ncorr);
      for(int i=0;i<Ncorr; ++i){
        A[i].x=correspondences[i][0];
        A[i].y=correspondences[i][1];
        B[i].x=correspondences[i][2];
        B[i].y=correspondences[i][3];
        std::cout<<A[i]<<" , "<<B[i]<<std::endl;
      }
      
      switch(gui["optMethod"].as<int>()){
        case 0:
          H = Homography2D(A.data(),B.data(),Ncorr);
          break;
        case 1:
          H = ransacBasedHomography(A.data(), B.data(), Ncorr);
          break;
        case 2:
          H = simplexBasedHomography(A.data(), B.data(), Ncorr);
          break;          
      }

      std::cout << H << std::endl;
      
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
      f["config.homogeneity"] = (FixedMatrix<float,3,3>&)H; // !homography ?!?!
      f.save(*pa("-o"));
      std::cout << "saved homography to file " << *pa("-o") << std::endl;
    }catch(ICLException &ex){
      ERROR_LOG(ex.what());
    }
  }
}

int main(int n, char **ppc){
  return ICLApp(n,ppc,
                "-size|-s(Size=VGA) "
                "-output|-o(output-xml-file-name=homogeneity.xml) "
                "-rgb-udist(fn1) "
                "-ir-udist(fn2) "
                "-marker-type|-m(type=bch,whichToLoad=[0-1000],size=50x50) "
                "-do-not-shift-the-IR-image",
                init,run).exec();
}
