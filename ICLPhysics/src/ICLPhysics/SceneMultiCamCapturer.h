#pragma once

#include <ICLGeom/Scene.h>
#include <ICLIO/GenericImageOutput.h>

namespace icl{

  namespace physics{
    
    class SceneMultiCamCapturer {
      std::vector<int> camIndices;
      geom::Scene *scene;
      std::vector<utils::SmartPtr<io::GenericImageOutput> > outputs;
      
      public:
      /// Dummy constructor
      SceneMultiCamCapturer();
      
      /// Simple constructor, that adds 3 default cameras to the scene, and creates SharedMemory image outputs sm 1,2,3 
      SceneMultiCamCapturer(geom::Scene &scene, const std::vector<geom::Camera> &cams=std::vector<geom::Camera>());
      
      SceneMultiCamCapturer(geom::Scene &scene, int num, int* camIndices, 
                            const std::string &progArgName="-o") throw (utils::ICLException);
      
      void init(geom::Scene &scene, int num, int* camIndices, 
                const std::string &progArgName="-o") throw (utils::ICLException);
      
      void capture();
    };
  }
}
