/*************************************************************************** 
**                                                                        **
** Copyright (C) 2006-2010 neuroinformatics group (vision)                **
**                         University of Bielefeld                        **
**                         nivision@techfak.uni-bielefeld.de              **
**                                                                        **
** This file is part of the ICLGeom module of ICL                         **
**                                                                        **
** Commercial License                                                     **
** Commercial usage of ICL is possible and must be negotiated with us.    **
** See our website www.iclcv.org for more details                         **
**                                                                        **
** GNU General Public License Usage                                       **
** Alternatively, this file may be used under the terms of the GNU        **
** General Public License version 3.0 as published by the Free Software   **
** Foundation and appearing in the file LICENSE.GPL included in the       **
** packaging of this file.  Please review the following information to    **
** ensure the GNU General Public License version 3.0 requirements will be **
** met: http://www.gnu.org/copyleft/gpl.html.                             **
**                                                                        **
***************************************************************************/ 

#ifndef ICL_SCENE_H
#define ICL_SCENE_H

#include <ICLGeom/SceneObject.h>
#include <ICLGeom/Camera.h>
#include <ICLCore/Img.h>

#ifdef HAVE_QT
#include <ICLQt/MouseHandler.h>
#include <ICLQt/DrawWidget3D.h>
#endif

#include <ICLUtils/Lockable.h>

namespace icl{
  
  /** \cond */
  class ICLDrawWidget;
  /** \endcond */

  
  /// *NEW* Scene Implementation 
  class Scene : public Lockable{
    public:
    struct RenderPlugin;
#ifdef HAVE_QT
#ifdef HAVE_OPENGL
    struct SceneMouseHandler;
#endif
#endif
    struct GLCallback;
    
    Scene();
    ~Scene();
    Scene(const Scene &scene);
    Scene &operator=(const Scene &scene);
    
    /// Adds a new Camera to the scene
    /** @param camera which is copied into the scene 
        @param visSize this parameter determines the size of the 
               3D visualized cameras in the scene. If visSize is
               1.0, cameras are visualized with size w=6, h=5 and l=10.
               Actually, the visSize you need depends on the unit
               you use for your scene in your application.
               The default visSize is quite appropriate if you work
               with cm. If you e.g. use mm instead, visSize should 
               be set to 10.
        */
    void addCamera(const Camera &cam, float visSize=1.0);
    void removeCamera(int index);
    Camera &getCamera(int camIndex = 0);
    const Camera &getCamera(int camIndex =0) const;
    inline int getCameraCount() const { return (int)m_cameras.size(); }

    void render(Img32f &image, int camIndex = 0);
    void render(ICLDrawWidget &widget, int camIndex = 0);


    
    void addObject(SceneObject *object);
    void removeObject(int idx, bool deleteObject = true);
    void removeObjects(int startIndex, int endIndex=-1, bool deleteObjects=true);
    int getObjectCount() const { return m_objects.size(); }
    
    void clear(bool camerasToo=false);
    
#ifdef HAVE_QT
#ifdef HAVE_OPENGL
    MouseHandler *getMouseHandler(int camIndex=0);
    ICLDrawWidget3D::GLCallback *getGLCallback(int camIndex);
#endif
#endif

    void setLightSimulationEnabled(bool enabled);
    bool getLightSimulationEnabled() const;
    void setDrawCamerasEnabled(bool enabled);
    bool getDrawCamerasEnabled() const;
    
    private:
    /// renders the scene into current OpenGL context
#ifdef HAVE_QT
#ifdef HAVE_OPENGL
    void render(int camIndex, ICLDrawWidget3D *widget);
#endif
#endif
    float getMaxSceneDim() const;

    void render(RenderPlugin &p, int camIndex);
    std::vector<Camera> m_cameras;
    std::vector<SceneObject*> m_objects;
    std::vector<SceneObject*> m_cameraObjects;
    std::vector<std::vector<std::vector<Vec> > >m_projections;//[cam][obj][vertex]
    
#ifdef HAVE_QT
#ifdef HAVE_OPENGL
    std::vector<SceneMouseHandler*> m_mouseHandlers;
#endif
#endif

    std::vector<GLCallback*> m_glCallbacks;
    
    bool m_lightSimulationEnabled;
    bool m_drawCamerasEnabled;
  };
}

#endif
