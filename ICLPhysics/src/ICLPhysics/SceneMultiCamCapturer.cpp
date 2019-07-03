/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2014 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLPhysics/src/ICLPhysics/SceneMultiCamCapturer.cpp    **
** Module : ICLPhysics                                             **
** Author : Christof Elbrechter, Matthias Esau                     **
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
#include <ICLPhysics/SceneMultiCamCapturer.h>

namespace icl{

  using namespace utils;
  using namespace io;
  using namespace core;
  using namespace geom;

  namespace physics{
    SceneMultiCamCapturer::SceneMultiCamCapturer(){}

    SceneMultiCamCapturer::SceneMultiCamCapturer(Scene &scene, const std::vector<Camera> &cams){
      this->scene = &scene;
      int num = scene.getCameraCount();

      if(!cams.size()){
        outputs.resize(3);

        Camera cams2[3] = {
          Camera(Vec(1533.31,-1791.21,1956.45,1),
                 Vec(-0.509914,0.571226,-0.643186,1),
                 Vec(0.355427,-0.601997,-0.715033,1)),
          Camera(Vec(-1520.31,-2429.55,2732.13,1),
                 Vec(0.324928,0.596491,-0.733908,1),
                 Vec(-0.120457,-0.800318,-0.587351,1)),
          Camera(Vec(21.2392,2860.37,2069.35,1),
                 Vec(0.0741057,-0.742273,-0.665988,1),
                 Vec(-0.134311,0.777196,-0.614758,1))
        };


        for(int i=0;i<3;++i){
          camIndices.push_back(num+i);
          this->scene->addCamera(cams2[i]);
          outputs[i] = new GenericImageOutput("sm","sm="+str(i));
        }
      }else{
        outputs.resize(cams.size());

        for(size_t i=0;i<cams.size();++i){
          camIndices.push_back(num+i);
          this->scene->addCamera(cams[i]);
          outputs[i] = new GenericImageOutput("sm","sm="+str(i));
        }

      }
    }

    SceneMultiCamCapturer::SceneMultiCamCapturer(Scene &scene, int num, int* camIndices,
                                                 const std::string &progArgName){
      init(scene,num, camIndices,progArgName);
    }

    void SceneMultiCamCapturer::init(Scene &scene, int num, int* camIndices,
                                     const std::string &progArgName){
      this->scene = &scene;
      this->camIndices.assign(camIndices,camIndices+num);

      std::string outputType = pa(progArgName,0);
      std::vector<std::string> defs = tok(*pa(progArgName,1),",");
      ICLASSERT_THROW((int)defs.size() == num, ICLException("SceneMultiCamCapturer::init: "
                                                            "number of camera indices is "
                                                            "different from the number of "
                                                            "given output definitions "));

      outputs.resize(num);
      for(int i=0;i<num;++i){
        outputs[i] = new GenericImageOutput(outputType,outputType+"="+defs[i]);
      }
    }

    void SceneMultiCamCapturer::capture(){
      for(size_t i=0;i<camIndices.size();++i){
        Img8u image = scene->render(camIndices[i]);
        outputs[i]->send(&image);
      }
    }

  }
}
