#pragma once

#include <ICLQt/MouseHandler.h>
#include <ICLGeom/Scene.h>
#include <ICLUtils/VisualizationDescription.h>

namespace icl{

  namespace physics{
    
    /** \cond */
    class PhysicsPaper3;
    /** \endcond */
    
    class PhysicsPaper3MouseHandler : public qt::MouseHandler{
      struct Data;
      Data *m_data;
    
      protected:

      void menuCallback(const std::string &entry);

      public:

      PhysicsPaper3MouseHandler(PhysicsPaper3 *model, geom::Scene *scene, int camIndex=0);
    
      virtual void process(const qt::MouseEvent &e);
    
      utils::VisualizationDescription vis() const;
    
      void applyForceToModel(float streangth=1, float radius = 0.1);

    };

  }

}
