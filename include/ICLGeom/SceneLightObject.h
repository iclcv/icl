/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2012 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : include/ICLGeom/SceneLightObject.h                     **
** Module : ICLGeom                                                **
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

#pragma once

#include <ICLGeom/SceneObject.h>
#include <ICLGeom/Scene.h>

namespace icl{
  namespace geom{
    
    /// The scene light object looks like a light bulb and represents an OpenGL light
    /** This class is used by the Scene class to visualize lights in the 3D scene */
    class SceneLightObject : public SceneObject{
      /// thread of the bulb is implemented as a child object
      class ThreadPart; 

      /// parent scene
      Scene *m_scene;
      
      /// corresponding light ID
      int m_lightID;
      
      /// internal flag
      bool m_hasText;
      
      public:
      /// constructor
      SceneLightObject(Scene *scene, int lightID);

      /// custom rendering stuff
      virtual void prepareForRendering();
    };
    
  }
}    
      
