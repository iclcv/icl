/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2014 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLPhysics/src/ICLPhysics/PhysicsPaper3MouseHandler.h  **
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
      
      /// if enabled, links are always added twice (with few pixels distance)
      void setAddLinksTwice(bool enabled);

    };

  }

}
