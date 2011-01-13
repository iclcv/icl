/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2010 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : include/ICLGeom/Scene.h                                **
** Module : ICLGeom                                                **
** Authors: Christof Elbrechter, Erik Weitnauer, Daniel Dornbusch  **
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

#ifndef ICL_SCENE_H
#define ICL_SCENE_H

#include <ICLGeom/SceneObject.h>
#include <ICLGeom/Camera.h>
#include <ICLCore/Img.h>

#ifdef HAVE_QT
#include <ICLQt/MouseHandler.h>
#ifdef HAVE_OPENGL
#include <ICLQt/DrawWidget3D.h>
#endif
#include <ICLGeom/SceneMouseHandler.h>
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
#ifdef HAVE_OPENGL
    struct GLCallback;
#endif
    Scene();
    ~Scene();
    Scene(const Scene &scene);
    Scene &operator=(const Scene &scene);

    /// Adds a new Camera to the scene
    /** @param camera which is copied into the scene
        @param visSize this parameter determines the size of the
               3D visualized cameras in the scene. If visSize is
               1.0, The camera coordinate system is visualized with size
               50x50x50.
               Actually, the visSize you need depends on the unit
               you use for your scene in your application.
               The default visSize is quite appropriate if you work
               with mm. If you e.g. use cm instead, visSize should
               be set to 0.1.
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
    void setMouseHandler(SceneMouseHandler* sceneMouseHandler, int camIndex=0, bool deleteExistingMouseHandler=true);
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
    void renderSceneObjectRecursive(SceneObject *o);
#endif
#endif
    float getMaxSceneDim() const;
    void extendMaxSceneDimRecursive(float &minX, float &maxX, 
                               float &minY, float &maxY, 
                               float &minZ, float &maxZ,
                               SceneObject *o) const;

    void render(RenderPlugin &p, int camIndex);
    std::vector<Camera> m_cameras;
    std::vector<SceneObject*> m_objects;
    std::vector<SceneObject*> m_cameraObjects;
    std::vector<std::vector<std::vector<Vec> > >m_projections;//[cam][obj][vertex]

#ifdef HAVE_QT
#ifdef HAVE_OPENGL
    std::vector<SceneMouseHandler*> m_mouseHandlers;
	std::vector<GLCallback*> m_glCallbacks;
#endif
#endif

    

    bool m_lightSimulationEnabled;
    bool m_drawCamerasEnabled;
  };
}

#endif
