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
    
    void addCamera(const Camera &cam);
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
