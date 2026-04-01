// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include <ICLUtils/CompatMacros.h>
#include <ICLGeom/SceneObject.h>
#include <ICLGeom/Scene.h>

namespace icl{
  namespace geom{

    /// The scene light object looks like a light bulb and represents an OpenGL light
    /** This class is used by the Scene class to visualize lights in the 3D scene */
    class ICLGeom_API SceneLightObject : public SceneObject{
      /// thread of the bulb is implemented as a child object
      class ThreadPart;

      /// parent scene
      //Scene *m_scene;

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
