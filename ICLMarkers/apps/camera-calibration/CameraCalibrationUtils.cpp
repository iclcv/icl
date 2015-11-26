/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLMarkers/apps/camera-calibration/CameraCalibration   **
**          Utils.cpp                                              **
** Module : ICLMarkers                                             **
** Authors: Christof Elbrechter                                    **
**                                                                 **
**                                                                 **
** GNU LESSER GENERAL PUBLIC LICENSE                               **
** This file may be used under the terms of the GNU Lesser General **
** Public License version 3.0 as published by the                  **
**                                                                 **
** Free Software Foundation and appearing in the file LICENSE.LGPL **
** included in the packaging of this file.  Please review the      **
** following information to ensure the license requirements will   **
** be met: http://www.gnu.org/licenses/lgpl-3.0.txt                **
**                                                                 **
** The development of this software was supported by the           **
** Excellence Cluster EXC 277 Cognitive Interaction Technology.    **
** The Excellence Cluster EXC 277 is a grant of the Deutsche       **
** Forschungsgemeinschaft (DFG) in the context of the German       **
** Excellence Initiative.                                          **
**                                                                 **
********************************************************************/

#include "CameraCalibrationUtils.h"
#include <ICLGeom/Scene.h>

#include <ICLUtils/ProgArg.h>
#include <ICLUtils/ConfigFile.h>
#include <ICLMath/LevenbergMarquardtFitter.h>
#include <ICLQt/Common.h>
#include <fstream>

#include <QtWidgets/QMessageBox>
#include <ICLMarkers/FiducialDetectorPlugin.h>
#include <ICLGeom/GridSceneObject.h>


namespace icl{
  namespace markers{

    static inline Vec set_3_to_1(Vec a){
      a[2] += 1;
      a[3] = 1;
      return a;
    }

    const std::string &CameraCalibrationUtils::create_sample_calibration_file_content(){
      static const std::string sample= ("<config>\n"
                                        "  <!-- A grid is a regular 1D or 2D grid of markers in 3D space.\n"
                                        "       Each grid is defined by the (x,y,z)-offset of the upper left marker,\n"
                                        "       and by two direction vectors 'x-direction' and 'y-direction'. \n"
                                        "       A grid marker at grid position (i,j) is located at \n"
                                        "       offset + x-direction*i + y-direction*j\n"
                                        "       The grid markers are assumed to be in the defined marker-range;\n"
                                        "       the markers are distributed in row-major order. Possible marker\n"
                                        "       types are 'bch' and 'amoeba'. If the marker type is bch, not only\n"
                                        "       the marker centers, but also the markers corners are used as\n"
                                        "       reference points. -->\n"
                                        "  <section id=\"grid-0\">\n"
                                        "      <data id=\"dim\" type=\"string\">(NumXCells)x(NumYCells)</data>\n"
                                        "      <data id=\"offset\" type=\"string\">x,y,z</data>\n"
                                        "      <data id=\"x-direction\" type=\"string\">dx1,dy1,dz1</data>\n"
                                        "      <data id=\"y-direction\" type=\"string\">dx2,dy2,dz2</data>\n"
                                        "      <data id=\"marker-type\" type=\"string\">amoeba|bch</data>\n"
                                        "      <data id=\"marker-ids\" type=\"string\">[minID,maxID]</data>\n"
                                        "   </section>\n"
                                        "    <!-- more grids with raising indices (also index 0 is optional)-->\n"
                                        "\n"
                                        "  <!-- For simplicity, 1 by 1 grids (i.e. single markers) can be defined\n"
                                        "       in a simpler way. Here, less information needs to be given, however\n"
                                        "       it is important to mention, that the marker corners can only be used\n"
                                        "       for grids. -->\n"
                                        "   <section id=\"marker-0\">\n"
                                        "      <data id=\"marker-type\" type=\"string\">amoeba|bch</data>\n"
                                        "      <data id=\"offset\" type=\"string\">x,y,z</data>\n"
                                        "      <data id=\"marker-id\" type=\"int\">id</data>\n"
                                        "   </section>\n"
                                        "   <!-- more markers with raising indices (also index 0 is optional)-->\n"
                                        "\n"
                                        "  <!-- If the calibration object can be placed in the scene in different distinct\n"
                                        "       ways (i.e. with differnt relative transformations), several world-tranforms can\n"
                                        "       can be defined. All defined world transforms can be chosen interactively at runtime.\n"
                                        "       By default, the first world transform is used. If no world transform is given, a dummy\n"
                                        "       transform called 'identity' is added automatically -->\n"
                                        "   <section id=\"world-transform-0\" type=\"string\">    <!-- also optional -->\n"
                                        "      <data id=\"name\" type=\"string\">example with z-offset of 100mm</data>\n"
                                        "      <data id=\"transform\" type=\"string\">\n"
                                        "          1 0 0 0\n"
                                        "          0 1 0 0\n"
                                        "          0 0 1 100\n"
                                        "          0 0 0 1\n"
                                        "      </data>\n"
                                        "   </section>\n"
                                        "   <!-- more possible world transforms (if none is given, an identity transform is added automatically -->\n"
                                        "   <data id=\"obj-file\" type=\"string\">\n"
                                        "      <!-- optional .obj file content that describes the visual shape of the calibration object -->\n"
                                        "   </data>\n"
                                        "</config>\n");
      return sample;
    }

    FiducialDetector *CameraCalibrationUtils::create_new_fd(MarkerType t,
                                                            std::vector<std::string> &configurables,
                                                            std::string &iin,
                                                            const utils::Size *size){
      static const std::string ts[2] = {"bch","amoeba"};
      FiducialDetector *fd = new FiducialDetector(ts[t]);
      fd->setConfigurableID(ts[t]);
      configurables.push_back(ts[t]);
      iin = fd->getIntermediateImageNames();
      fd->setPropertyValue("css.angle-threshold",180);
      fd->setPropertyValue("css.curvature-cutoff",66);
      fd->setPropertyValue("css.rc-coefficient",1);
      fd->setPropertyValue("thresh.global threshold",-10);
      fd->setPropertyValue("thresh.mask size",30);

      if(size && size->getDim() <= utils::Size::QVGA.getDim()){
        fd->setPropertyValue("pp.filter","none");
      }
      return fd;
    }

    std::string CameraCalibrationUtils::get_save_filename(const std::string &progArgName){
      if(pa(progArgName)){
        return *pa(progArgName);
      }else{ 
        try{
          return saveFileDialog("*.xml","save current camera","./");
        }catch(...){}
      }
      return "";

    }

    void CameraCalibrationUtils::save_cam_filename(geom::Camera cam,
                                                   const std::string &outputSizeProgArg,
                                                   const std::string &filename){
      if(filename.length()){
        std::ofstream s(filename.c_str());
        if(pa(outputSizeProgArg)){
          Camera::RenderParams &r = cam.getRenderParams();
          Size os = parse<Size>(*pa(outputSizeProgArg));
          float fx = float(os.width) / float(r.chipSize.width);
          float fy = float(os.height) / float(r.chipSize.height);
          r.chipSize = os;
          cam.setSamplingResolutionX(cam.getSamplingResolutionX()*fx);
          cam.setSamplingResolutionY(cam.getSamplingResolutionY()*fy);
          cam.setPrincipalPointOffset(cam.getPrincipalPointOffsetX()*fx,
                                      cam.getPrincipalPointOffsetY()*fy);
          r.viewport = Rect(Point::null,os);
        }
        s << cam;
      }
    }
    
    void CameraCalibrationUtils::save_cam_pa(const geom::Camera &cam, 
                                             const std::string &outputSizeProgArg,
                                             const std::string &outputFileNameProgArg){
      std::string filename = get_save_filename(outputFileNameProgArg);
      save_cam_filename(cam,outputSizeProgArg,filename);
    }

    
    CameraCalibrationUtils::BestOfNSaver::BestOfNSaver(utils::Function<int> nFramesSource):
      inited(false),nFramesSource(nFramesSource){}
    
    bool CameraCalibrationUtils::BestOfNSaver::event ( QEvent * event ){
      ICLASSERT_RETURN_VAL(event,false);
      if(event->type() == QEvent::User){
        QMessageBox::information(0,"saved",("camera file has been saved to\n"+lastFileName+"\n"+"error was:"+str(lastBestError)).c_str());
        return true;
      }else{
        return QObject::event(event);
      }
    } 
    
    void CameraCalibrationUtils::BestOfNSaver::init(){
      num_end = nFramesSource();
      Mutex::Locker l(this);
      if(inited) return;
      filename = CameraCalibrationUtils::get_save_filename("-o");
      if(filename != ""){
        cams.resize(0); cams.reserve(num_end);
        errors.resize(0); errors.reserve(num_end);
        n = 0;
        inited = true;
      }
      runningBestError = 99999;
    }
    
    void CameraCalibrationUtils::BestOfNSaver::stop(){
      Mutex::Locker l(this);
      if(inited){
        n = num_end;
      }
    }
    
    std::pair<int,float> CameraCalibrationUtils::BestOfNSaver::next_hook(const Camera &cam, float error){
      Mutex::Locker l(this);
      if(!inited) return std::pair<int,float>(0,0);


      if(error > 0){
        cams.push_back(cam);
        errors.push_back(error);
      }
      ++n;
      
      //gui["save_remaining_frames"] = num_end - n;
      if(error > 0 && (error < runningBestError)){
        runningBestError = error;
      }
      //gui["save_best_error"] = runningBestError;

      if(n >= num_end){
        if(!cams.size()){
          QMessageBox::critical(0,"error","unable to save the best of N calibration results:\n"
                                "all N calibration runs failed!");
        }
        else{
          int best = (int)(std::min_element(errors.begin(),errors.end())-errors.begin());
          int worst = (int)(std::max_element(errors.begin(),errors.end())-errors.begin());
          
          std::cout << "best error:" << errors[best] << std::endl;
          std::cout << "worst error:" << errors[worst] << std::endl;
            
          CameraCalibrationUtils::save_cam_filename(cams[best],"-os",filename);
          lastBestError = errors[best];
          lastFileName = filename;
          QApplication::postEvent(this,new QEvent(QEvent::User),Qt::HighEventPriority);
        }
        inited = false;
      }
      return std::pair<int,float>(num_end -n, runningBestError);
            
    }

    CameraCalibrationUtils::CalibFile CameraCalibrationUtils::parse_calib_file(const std::string &fn, int c,
                                                                               CameraCalibrationUtils::CalibFileData &d){
      CalibFile cf;
      cf.filename = fn;
      cf.obj = 0;
      ConfigFile cfg(cf.filename);
    
      std::cout << "* parsing given configuration file '" << cf.filename << "'" << std::endl;
    


      std::ostringstream transformNameList;    
      for(int t=0;true;++t){
        try{
          const std::string name = cfg["config.world-transform-" + str(t) + ".name"].as<std::string>();
          transformNameList << (t?",":"") << name;
          const Mat transform = parse<Mat>(cfg["config.world-transform-"+str(t)+".transform"]);
          cf.transforms.push_back(NamedTransform(name,transform));
        }catch(...){
          break;
        }
      }

      const bool transformGiven = cf.transforms.size();
      if(!transformGiven){
        cf.transforms.push_back(NamedTransform("identity",Mat::id()));
      }


      File cff(cf.filename);
      std::string tt = "full path: " + cf.filename;
      int initIdx = pa("-it");
      d.objGUI << ( HBox().label(cff.getBaseName()+cff.getSuffix()).minSize(1,3).maxSize(100,3)
                    << CheckBox("enable",true).out("enable-obj-"+str(c)).tooltip(tt)
                    << Combo(transformNameList.str() + (transformGiven?"":",id"),initIdx).handle("transform-obj-"+str(c)).tooltip(tt)
                    );

#ifdef WIN32
      std::string tmpFilename("tmp-obj-file.obj");
#else
      std::string tmpFilename("/tmp/tmp-obj-file.obj");
#endif

      try{
        std::string s;
        try{
          const std::string s2 = cfg["config.obj-file"].as<std::string>();
          s = s2;
        }catch(...){
          throw 1;
        }
        {
          std::ofstream obj(tmpFilename.c_str());
          obj << s << std::endl;
        }

        SceneObject *o = new SceneObject(tmpFilename.c_str());
        o->setColor(Primitive::quad,GeomColor(0,100,255,100));
        o->setColor(Primitive::line,GeomColor(255,0,0,255));
        o->setVisible(Primitive::line,true);
        o->setLineWidth(2);
        o->setTransformation(cf.transforms[0].transform);
        o->setVisible(false);
        cf.obj = o;
      }catch(ICLException &e){
        SHOW(e.what());
      }catch(int){}
    
      int systemResult = system(string(ICL_SYSTEMCALL_RM).append(tmpFilename.c_str()).c_str());
      (void)systemResult;
    
      enum Mode{
        ExtractGrids,
        ExtractSingleMarkers,
        ExtractionDone
      } mode = ExtractGrids;

      for(int i=0;mode != ExtractionDone ;++i){
      
        cfg.setPrefix(str("config.") + ((mode == ExtractGrids) ? "grid-" : "marker-")+str(i)+".");  
        if(!cfg.contains("offset")) {
          mode = (Mode)(mode+1);
          i = -1;
          continue;
        }

        Vec3 o,dx,dy,dx1,dy1;
        Size s(1,1);
        Size32f ms;
        std::vector<int> markerIDs;
        MarkerType t;
        bool haveCorners;
      
        t = (MarkerType)(cfg["marker-type"].as<std::string>() == "amoeba");
        o = parse<Vec3>(cfg["offset"]);
        if(mode == ExtractGrids){
          s = parse<Size>(cfg["dim"]);
          dx = parse<Vec3>(cfg["x-direction"]);
          dy = parse<Vec3>(cfg["y-direction"]);
          dx1 = dx.normalized();
          dy1 = dy.normalized();

          markerIDs = FiducialDetectorPlugin::parse_list_str(cfg["marker-ids"].as<std::string>());

          ICLASSERT_THROW((int)markerIDs.size() == s.getDim(), 
                          ICLException("error loading configuration file at given grid " + str(i)
                                       + ": given size " +str(s) + " is not compatible to "
                                       + "given marker ID range "  + 
                                       cfg["marker-ids"].as<std::string>()));
        }else{
          markerIDs.push_back(cfg["marker-id"].as<int>());
        }
        
        if(!d.fds[t]){
          static SmartPtr<Size> s = pa("-s") ? SmartPtr<Size>(new Size(pa("-s").as<Size>())) : SmartPtr<Size>();
          d.fds[t] = create_new_fd(t,d.configurables,d.iin,s.get());
          d.lastFD = d.fds[t].get();
        }
        try{ 
          ms = parse<Size32f>(cfg["marker-size"]); 
        } catch(...){}

        if(markerIDs.size() > 1){
          d.fds[t]->loadMarkers("{"+cat(markerIDs,",")+"}",t==AMOEBA ? ParamList() : ParamList("size",ms));
        }else{
          d.fds[t]->loadMarkers(str(markerIDs[0]),t==AMOEBA ? ParamList() : ParamList("size",ms));
        }

        if(mode == ExtractGrids){
          MarkerGrid g = { o, dx, dy, s, ms, markerIDs, t };
          cf.grids.push_back(g);
                    
          std::cout << "** registering grid with " << (t?"amoeba":"bch") << " marker ids: {" 
                    << markerIDs.front() << ", ..., " << markerIDs.back() << "}"  << std::endl;
        }else{
          std::cout << "** registering single " << (t?"amoeba":"bch") << " marker with id " << markerIDs[0] << std::endl; 
        }

        
        int idIdx = 0;
        std::vector<PossibleMarker> &lut = d.possible[t];
        //  std::vector<Vec> vertices;
        
        haveCorners = (mode==ExtractGrids) && (ms != Size32f::null) && (t==BCH);

        for(int y=0;y<s.height;++y){
          for(int x=0;x<s.width;++x, ++idIdx){
            int id = markerIDs[idIdx];
            if(id == -1) continue;
            Vec3 v = o+dx*x +dy*y;
            if(lut[id].loaded) throw ICLException("error loading configuration file at given grid " + str(i)
                                                  +" : the marker ID " + str(id) + " was already used before");
            if(haveCorners){
              Vec3 ul = v + dx1*(ms.width/2) - dy1*(ms.height/2);
              Vec3 ur = v + dx1*(ms.width/2) + dy1*(ms.height/2);
              Vec3 ll = v - dx1*(ms.width/2) + dy1*(ms.height/2);
              Vec3 lr = v - dx1*(ms.width/2) - dy1*(ms.height/2);
              
              lut[id] = PossibleMarker(c,
                                       v.resize<1,4>(1),
                                       ul.resize<1,4>(1),
                                       ur.resize<1,4>(1),
                                       ll.resize<1,4>(1),
                                       lr.resize<1,4>(1));
            }else{
              lut[id] = PossibleMarker(c,Vec(v[0],v[1],v[2],1));
            }
            lut[id].gridIdx = mode == ExtractGrids ? i : -1;
            //            vertices.push_back(cf.transforms[0].transform*Vec(v[0],v[1],v[2],1));
          }
        }
      }
      return cf;
    }

    void CameraCalibrationUtils::change_plane(const std::string &handle, GUI &planeOptionGUI, Scene &scene,
                                              CameraCalibrationUtils::CalibFileData &calibFileData){
      if(handle == "planeDim"){
        if(planeOptionGUI["planeDim"].as<std::string>() == "none"){
          planeOptionGUI["planeOffset"].disable();
          planeOptionGUI["planeRadius"].disable();
          planeOptionGUI["planeTicDist"].disable();
          planeOptionGUI["planeColor"].disable();
          planeOptionGUI["planeStatus"] = str("removed");
          scene.removeObject(calibFileData.planeObj);
          calibFileData.planeObj = 0;
          //      havePlane = false;
          return;
        }else{
          //havePlane = true;
          planeOptionGUI["planeOffset"].enable();
          planeOptionGUI["planeRadius"].enable();
          planeOptionGUI["planeTicDist"].enable();
          planeOptionGUI["planeColor"].enable();
        }
      }
      if(calibFileData.planeObj){
        scene.removeObject(calibFileData.planeObj);
        ICL_DELETE(calibFileData.planeObj);
      }
  
      const std::string t = planeOptionGUI["planeDim"].as<std::string>();
      const float offset = planeOptionGUI["planeOffset"];
      const float radius = parse<float>(planeOptionGUI["planeRadius"]);
      const float ticDist = planeOptionGUI["planeTicDist"];
      const Color4D c = planeOptionGUI["planeColor"];

      int n = (2*radius) / ticDist;
      if(n * n > 1000000){
        planeOptionGUI["planeStatus"] = str("too many nodes");
        return;
      }else{
        planeOptionGUI["planeStatus"] = str("ok (") + str(n*n) + " nodes)";
      }
      Vec o(offset*(t=="x"),offset*(t=="y"),offset*(t=="z"),1);
      Vec dx,dy;
      if(t == "x"){
        dx = Vec(0,ticDist,0,1);
        dy = Vec(0,0,ticDist,1);
      }else if(t == "y"){
        dx = Vec(ticDist,0,0,1);
        dy = Vec(0,0,ticDist,1);
      }else{
        dx = Vec(ticDist,0,0,1);
        dy = Vec(0,ticDist,0,1);
      }
      int n2 = n/2;

      calibFileData.planeObj = new GridSceneObject(n,n,o -dx*(n2) - dy*(n2) ,dx,dy,true,false);
      calibFileData.planeObj->setColor(Primitive::line,GeomColor(c[0],c[1],c[2],c[3]));
      calibFileData.planeObj->setVisible(Primitive::vertex,false);

      calibFileData.planeObj->addVertex(set_3_to_1(o-dx*n2));
      calibFileData.planeObj->addVertex(set_3_to_1(o+dx*n2));

      calibFileData.planeObj->addVertex(set_3_to_1(o-dy*n2));
      calibFileData.planeObj->addVertex(set_3_to_1(o+dy*n2));
  
      calibFileData.planeObj->addLine(n*n,n*n+1,GeomColor(255,0,0,255));
      calibFileData.planeObj->addLine(n*n+2,n*n+3,GeomColor(0,255,0,255));


      scene.addObject(calibFileData.planeObj);
    }
    void CameraCalibrationUtils::visualize_found_markers(DrawHandle3D &draw,
                                                         const std::vector<FoundMarker> &markers,
                                                         const std::vector<bool> &enabled,
                                                         bool deactivatedCenters, bool useCorners){
      draw->symsize(7);
      for(unsigned int i=0;i<markers.size();++i){
        const FoundMarker &m = markers[i];
        draw->linewidth(2);
        
        const int idx = markers[i].cfgFileIndex;
          if(enabled[idx]){
            draw->color(0,100,255,255);
            draw->fill(0,100,255,100);
          }else{
            draw->color(200,200,200,255);
            draw->fill(100,100,100,100);
            
          }
          if(m.hasCorners){
            draw->polygon(std::vector<Point32f>(m.imageCornerPositions,m.imageCornerPositions+4));
          }
          if(enabled[idx]){
            draw->color(255,0,0,255);
          }else{
            draw->color(200,200,200,255);
          }
          
          draw->linewidth(1);
          draw->sym(m.imagePos,'x');
          if(useCorners && m.hasCorners){
            if(deactivatedCenters) draw->color(255,0,0,100);
            draw->sym(m.imageCornerPositions[0],'x');
            draw->sym(m.imageCornerPositions[1],'x');
            draw->sym(m.imageCornerPositions[2],'x');
            draw->sym(m.imageCornerPositions[3],'x');
          }
          if(enabled[idx]){
            draw->color(0,255,0,255);
          }else{
            draw->color(200,200,200,255);
          }
          draw->text(str(m.id),m.imagePos.x, m.imagePos.y+12, -10);
        }
      }
    std::vector<CameraCalibrationUtils::FoundMarker> CameraCalibrationUtils::detect_markers(const core::ImgBase *image,
                                                                                            CalibFileData &calibFileData){
      std::vector<FoundMarker> markers;
      for(int x=0;x<2;++x){
        if(!calibFileData.fds[x]) continue;
        const std::vector<Fiducial> &fids = calibFileData.fds[x]->detect(image);
        for(unsigned int i=0;i<fids.size();++i){
          
          const PossibleMarker &p = calibFileData.possible[x][fids[i].getID()];
          if(p.loaded){
            if(p.hasCorners && fids[i].getKeyPoints2D().size() == 4){
              const std::vector<Fiducial::KeyPoint> &kps = fids[i].getKeyPoints2D();
              Point32f imagePositions[4] = { 
                kps[0].imagePos, 
                kps[1].imagePos, 
                kps[2].imagePos,
                kps[3].imagePos 
              };
              
              markers.push_back(FoundMarker(fids[i].getID(), &p,(MarkerType)x,fids[i],fids[i].getCenter2D(),p.center,
                                            imagePositions,p.corners,p.cfgFileIndex));
            }else{
              markers.push_back(FoundMarker(fids[i].getID(), &p,(MarkerType)x,fids[i],fids[i].getCenter2D(),p.center,p.cfgFileIndex));
            }
          }
        }
      }
      return markers;
    }

    void CameraCalibrationUtils::visualize_plane(DrawHandle3D &draw,
                                                 const std::string &planeDim,
                                                 float planeOffset,
                                                 const utils::Point32f &currentMousePos,
                                                 geom::Scene &scene){
      draw->linewidth(1);
      const Point32f p = currentMousePos;
      const std::string t = planeDim;
      const float o = planeOffset;
      const float x=t=="x",y=t=="y",z=t=="z";
      PlaneEquation pe(Vec(o*x,o*y,o*z,1),Vec(x,y,z,1));
      
      const Vec w = scene.getCamera(0).getViewRay(p).getIntersection(pe);
      const Vec wx = x ? Vec(w[0],o,w[2],1) : Vec(o,w[1],w[2],1);
      const Vec wy = y ? Vec(w[0],w[1],o,1) : Vec(w[0],o,w[2],1);
      
      draw->color(0,100,255,255);
      draw->line(p,scene.getCamera(0).project(wx));
      draw->line(p,scene.getCamera(0).project(wy));
      
      const std::string tx = str("(")+str(w[0])+","+str(w[1])+","+str(w[2])+")";
      draw->color(0,0,0,255);
      draw->text(tx,p.x+1,p.y-11,8);
      draw->text(tx,p.x,p.y-12,8);
      draw->color(255,255,255,255);
      draw->text(tx,p.x-1,p.y-13,8);
    }
    const core::ImgBase *CameraCalibrationUtils::preprocess(const core::ImgBase *image){
      if(pa("-normalize-input-image")){
        static ImgBase *normalized = 0;
        image->deepCopy(&normalized);
        normalized->normalizeAllChannels(Range64f(0,255));
        
        static Img8u normalized8u;
        normalized->convert(&normalized8u);
        
        image = &normalized8u;    
      }
      
      if(pa("-crop-and-rescale")){
        static Rect *r = 0;
        static ImgBase *croppedAndRescaled = 0;
        if(!r){
          int bx = pa("-crop-and-rescale",0);
          int by = pa("-crop-and-rescale",1);
          int tw = pa("-crop-and-rescale",2);
          int th = pa("-crop-and-rescale",3);
          
          r = new Rect(bx,by,image->getWidth()-2*bx, image->getHeight()-2*by);
          
          ICLASSERT_THROW(r->width <= image->getWidth(),ICLException("clipping rect width is larger then image width"));
          ICLASSERT_THROW(r->height <= image->getHeight(),ICLException("clipping rect height is larger then image height"));
          ICLASSERT_THROW(r->x>= 0,ICLException("clipping x-offset < 0"));
          ICLASSERT_THROW(r->y>= 0,ICLException("clipping y-offset < 0"));
          ICLASSERT_THROW(r->right() < image->getWidth(),ICLException("clipping rect's right edge is outside the image rect"));
          ICLASSERT_THROW(r->bottom() < image->getHeight(),ICLException("clipping rect's right edge is outside the image rect"));
          
          croppedAndRescaled = imgNew(image->getDepth(),Size(tw,th),image->getChannels(),image->getFormat()); 
        }
        const ImgBase *tmp = image->shallowCopy(*r);
        
        tmp->scaledCopyROI(&croppedAndRescaled, interpolateLIN);
        delete tmp;
        image = croppedAndRescaled; 
      }
      return image;
    }

    /* move to Camera.cpp
    namespace{
      struct LMAOptUtil{
        typedef math::LevenbergMarquardtFitter<double> LMA;
        typedef LMA::Vector vec;
        typedef LMA::Params params;
        typedef LMA::Matrix mat;
        
        Camera cam;
        Mat P,T;
        LMAOptUtil(const Camera &cam):cam(cam){
          P = cam.getProjectionMatrix();
          T = cam.getCSTransformationMatrix();
        }

        static Mat m(const params &p){
          return create_hom_4x4<float>(p[0],p[1],p[2],p[3]*1000,p[4]*1000,p[5]*1000);
        }
        
        Camera fixCam(const params &par){
          Mat3 R = T.part<0,0,3,3>();
          Vec3 pOrig =  cam.getPosition().resize<1,3>(); //T.part<3,0,1,3>();
          Mat d = m(par);
          Mat3 dR = d.part<0,0,3,3>();
          Vec3 dt = d.part<3,0,1,3>();

          Mat3 dRR = dR * R;
 
          Camera c = cam;
          c.setUp(Vec(dRR(0,1), dRR(1,1), dRR(2,1), 1));
          c.setNorm(Vec(dRR(0,2), dRR(1,2), dRR(2,2), 1));
          c.setPosition( (pOrig - dRR.transp() * dt).resize<1,4>(1));

          return c;
        }
        Mat getFixedCSMatrix(const params &p){
          return  m(p) * T;
        }
        
        vec f(const params &p, const vec &x) const{
          Mat Q = P * m(p) * T;
          Vec y = Q * Vec(x[0],x[1],x[2],1);
          vec r(2);
          r[0] = y[0]/y[3];
          r[1] = y[1]/y[3];
          return r;
        }
        
        static Camera optimize(const Camera &init, 
                               const std::vector<Vec> &Xws, 
                               const std::vector<Point32f> &xis){
        
          LMAOptUtil u(init);
          LMA lma(function(u,&LMAOptUtil::f), 2, std::vector<LMA::Jacobian>(),
                  0.1, 1000);
          //lma.setDebugCallback();

          int n = (int)Xws.size();
          mat xs(3,n), ys(2,n);
          for(int i=0;i<n;++i){
            xs(0,i) = Xws[i].x;
            xs(1,i) = Xws[i].y;
            xs(2,i) = Xws[i].z;

            ys(0,i) = xis[i].x;
            ys(1,i) = xis[i].y;
          }
                    
          LMA::Result r = lma.fit(xs, ys, params(6,0.0));
          std::cout << "LMA Optimization Results: " << std::endl << r << std::endl;
          
          return u.fixCam(r.params);
        }
      };
    }
    
    Camera CameraCalibrationUtils::optimize_extrinsic_lma(const Camera &init, const std::vector<Vec> &Xws, 
                                                          const std::vector<Point32f> &xis){
      return LMAOptUtil::optimize(init, Xws, xis);
    }
    */
    CameraCalibrationUtils::CalibrationResult
    CameraCalibrationUtils::perform_calibration(const std::vector<FoundMarker> &markers,
                                                const std::vector<bool> &enabledCfgFiles,
                                                const std::vector<geom::Mat> &Ts,
                                                const geom::Mat &Trel, const utils::Size &imageSize,
                                                bool &deactivatedCenters, bool useCorners,
                                                bool normalizeError, BestOfNSaver *saver,
                                                bool &haveAnyCalibration, geom::Scene &scene,
                                                const geom::Camera *givenIntrinsicParams,
                                                bool performLMAbasedOptimiziation){
      CalibrationResult res;
      res.error = 0;
      std::vector<Vec> xws,xwsCentersOnly;
      std::vector<Point32f> xis,xisCentersOnly;
      
      for(unsigned int i=0;i<markers.size();++i){
        const int idx = markers[i].cfgFileIndex;
        if(!enabledCfgFiles[idx]) continue;
        Mat T = Trel * Ts[idx];
        xws.push_back(T *markers[i].worldPos);
        xis.push_back(markers[i].imagePos);
        
        xwsCentersOnly.push_back(xws.back());
        xisCentersOnly.push_back(xis.back());
        
        if(useCorners && markers[i].hasCorners){
          for(int j=0;j<4;++j){
            xws.push_back(T * markers[i].worldCornerPositions[j]);
            xis.push_back(markers[i].imageCornerPositions[j]);
          }
        }
      }
      
      std::vector<Vec> *W[2] = { &xws, &xwsCentersOnly };
      std::vector<Point32f> *I[2] = { &xis, &xisCentersOnly };
      int idx = 0;
      deactivatedCenters = false;
      while(true){
        try{
          scene.lock();
          Camera cam = scene.getCamera(0);
          {
            Mutex::Locker lock(saver);
            if(givenIntrinsicParams){
              cam = Camera::calibrate_extrinsic(*W[idx], *I[idx], *givenIntrinsicParams, 
                                                Camera::RenderParams(),  performLMAbasedOptimiziation);
            }else{
              cam = Camera::calibrate_pinv(*W[idx], *I[idx], 1, performLMAbasedOptimiziation);
            }
            cam.getRenderParams().viewport = Rect(Point::null,imageSize);
            cam.getRenderParams().chipSize = imageSize;
            
            //if(performLMAbasedOptimiziation){
            //  cam = optimize_extrinsic_lma(cam, *W[idx], *I[idx]);
            //}
          }
          scene.getCamera(0) = cam;
          scene.unlock();

          
          float error = 0;
          
          for(unsigned int i=0;i<W[idx]->size();++i){
            error += I[idx]->operator[](i).distanceTo(cam.project(W[idx]->operator[](i)));
          }
          if(normalizeError){
            error /= sqr(W[idx]->size());
            error *= 100;
          }else{
            error /= W[idx]->size();
          }
          
          //gui["error"] = error;
          res.error = error;
          //gui["status"] = str("ok") + ((idx&&useCorners) ? "(used centers only)" : "");
          res.status =  str("ok") + ((idx&&useCorners) ? "(used centers only)" : "");
          if(idx) deactivatedCenters = true;
          
          std::pair<int,float> r = saver->next_hook(cam,error);
          res.saveRemainingFrames = r.first; //gui["save_remaining_frames"] = r.first;
          res.saveBestError = r.second; //gui["save_best_error"] = r.second;
          
          if(error > 0 && !haveAnyCalibration){
            haveAnyCalibration = true;
            for(int i=0;i<scene.getObjectCount();++i){
              scene.getObject(i)->setVisible(true);
            }
          }
          break;
        }catch(ICLException &e){
          if(idx == 0){
            ++idx;
          }else{
            res.status = str(e.what());
            //gui["status"] = str(e.what());
            std::pair<int,float> r = saver->next_hook(Camera(),-1);
            res.saveRemainingFrames = r.first;//        gui["save_remaining_frames"] = r.first;
            res.saveBestError = r.second; //gui["save_best_error"] = r.second;
            break;
          }
        }
      }
      return res;
    }

    int CameraCalibrationUtils::MarkerGrid::getCellIdx(int id) const{
      std::vector<int>::const_iterator b = markerIDs.begin();
      std::vector<int>::const_iterator e = markerIDs.end();
      std::vector<int>::const_iterator it = std::find(b,e,id);
      if(it == e) return -1;
      return (int)(it - b);
    }

    utils::Point CameraCalibrationUtils::MarkerGrid::getCellFromCellIdx(int cellIdx) const{
      if(cellIdx == -1){
        return Point(-1,-1);
      }else{
        return utils::Point(cellIdx%dim.width,cellIdx/dim.width);
      }
    }

    utils::Point CameraCalibrationUtils::MarkerGrid::getCell(int id) const{
      return getCellFromCellIdx(getCellIdx(id));
    }


    
    CameraCalibrationUtils::DetectedGrid::DetectedGrid():realGrid(0){}

    CameraCalibrationUtils::DetectedGrid::operator bool() const {
      return !!realGrid;
    }
    
    void CameraCalibrationUtils::DetectedGrid::setup(const CameraCalibrationUtils::MarkerGrid *realGrid){
      this->realGrid = realGrid;
      foundMarkers.resize(realGrid->getDim(),(const CameraCalibrationUtils::FoundMarker*)0);
    }
    int CameraCalibrationUtils::DetectedGrid::numFound() const{
      int n = 0;
      for(size_t i=0;i<foundMarkers.size();++i){
        if(foundMarkers[i]) ++n;
      }
      return n;
    }

    static float len_3(const math::Vec3 &v){
      return ::sqrt(v[0]*v[0] + v[1]*v[1] + v[2]*v[2]);
    }
    
    static Point32f get_marker_center(int cellIdx, const CameraCalibrationUtils::MarkerGrid &g){
      Point cell = g.getCellFromCellIdx(cellIdx);
      float x = cell.x * len_3(g.dx) + g.markerSize.width/2;
      float y = cell.y * len_3(g.dy) + g.markerSize.height/2;
      return Point32f(x,y);
    }

    static Point32f get_marker_corner(const Point32f &markerCenter, const CameraCalibrationUtils::MarkerGrid &g, int cornerIdx){
      Point32f delta(g.markerSize.width/2,g.markerSize.height/2);
      switch(cornerIdx){
        case 0: delta.y *= -1; break;
        case 1: break;
        case 2: delta.x *= -1; break;
        default: delta.x *= -1; delta.y *=-1 ; break;
      };
      return markerCenter + delta;
    }

    Mat CameraCalibrationUtils::DetectedGrid::estimatePose(const Camera &cam) const{
      std::vector<Point32f> modelCoords;
      std::vector<Point32f> imageCoords;
      for(size_t i=0;i<foundMarkers.size();++i){
        const FoundMarker *f = foundMarkers[i];
        if(!f) continue;
        imageCoords.push_back(f->imagePos); // center
        Point32f center = get_marker_center(i, *realGrid);
        modelCoords.push_back(center);
        if(f->hasCorners){
          for(int j=0;j<4;++j){
            imageCoords.push_back(f->imageCornerPositions[j]);
            modelCoords.push_back(get_marker_corner(center, *realGrid, j));
          }
        }
      }
      CoplanarPointPoseEstimator cppe(CoplanarPointPoseEstimator::worldFrame,
                                      CoplanarPointPoseEstimator::SamplingFine);
      Mat pose = cppe.getPose(modelCoords.size(), modelCoords.data(),
                              imageCoords.data(), cam);
      
      return pose;
    }
    
    void CameraCalibrationUtils::DetectedGrid::getGridCornersAndTexture(const Camera &cam,
                                                                        std::vector<Point> &points,
                                                                        std::vector<Line32f> &lines,
                                                                        Size32f &size,
                                                                        const Rect &bounds) const{
      Point32f centerLR = get_marker_center(realGrid->getDim()-1,*realGrid);
      Point32f c = get_marker_corner(centerLR, *realGrid, 1);

      bool pointsInBounds = true;
      if(numFound()){
        Mat T = estimatePose(cam);
   
        const Vec cs[4] = {
          T * Vec(0,0,0,1),
          T * Vec(c.x,0,0,1),
          T * Vec(c.x,c.y,0,1),
          T * Vec(0, c.y, 0, 1)
        };
        
        std::vector<Point32f> ps = cam.project(std::vector<Vec>(cs,cs+4));
        
        points = std::vector<Point>(ps.begin(), ps.end());
        for(int i=0;i<4;++i){
          if(!bounds.contains(ps[i].x,ps[i].y)){
            pointsInBounds = false;
          }
        }
      }
      if(!numFound() || !pointsInBounds){
        Rect r = Rect(Point::null,cam.getResolution()).enlarged(-10);
        Point ps[4] = { r.ul(), r.ur(), r.lr(), r.ll() };
        points.assign(ps,ps+4);                        
      }
      
      size = Size32f(c.x,c.y);
      
      lines.clear();
      
      int cellIdx = 0;
      for(int y=0;y<realGrid->dim.height;++y){
        for(int x=0;x<realGrid->dim.width;++x,++cellIdx){
          Point32f c = get_marker_center(cellIdx,*realGrid);
          Point32f cs[5];
          for(int i=0;i<4;++i){
            cs[i] = get_marker_corner(c, *realGrid, i);
          }
          cs[4] = cs[0];
          for(int i=0;i<4;++i){
            lines.push_back(Line32f(cs[i],cs[i+1]));
          }
          // a cross?
          lines.push_back(Line32f(cs[0], cs[2]));
          lines.push_back(Line32f(cs[1], cs[3]));
        }
      }
    }
  }
}
