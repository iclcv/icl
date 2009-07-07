#ifndef ICL_SCENE_2_H
#define ICL_SCENE_2_H

#include <iclObject2.h>
#include <iclCamera.h>
#include <iclImg.h>

#ifdef HAVE_QT
#include <iclMouseHandler.h>
#include <iclDrawWidget3D.h>
#endif

#include <iclLockable.h>

namespace icl{
  
  /** \cond */
  class ICLDrawWidget;
  /** \endcond */

  
  /// *NEW* Scene Implementation 
  class Scene2 : public Lockable{
    public:
    struct RenderPlugin;
#ifdef HAVE_QT
    struct SceneMouseHandler;
#endif
    struct GLCallback;
    
    Scene2();
    ~Scene2();
    Scene2(const Scene2 &scene);
    Scene2 &operator=(const Scene2 &scene);
    
    void addCamera(const Camera &cam);
    void removeCamera(int index);
    Camera &getCamera(int camIndex = 0);
    const Camera &getCamera(int camIndex =0) const;
    inline int getCameraCount() const { return (int)m_cameras.size(); }

    void render(Img32f &image, int camIndex = 0);
    void render(ICLDrawWidget &widget, int camIndex = 0);

    /// renders the scene into current OpenGL context
    void render(int camIndex=0);

    
    void addObject(Object2 *object);
    void removeObject(int idx, bool deleteObject = true);
    void removeObjects(int startIndex, int endIndex=-1, bool deleteObjects=true);
    int getObjectCount() const { return m_objects.size(); }
    
    void clear(bool camerasToo=false);
    
#ifdef HAVE_QT
    MouseHandler *getMouseHandler(int camIndex=0);
    ICLDrawWidget3D::GLCallback *getGLCallback(int camIndex);
#endif

    void setLightSimulationEnabled(bool enabled);
    bool getLightSimulationEnabled() const;
    void setDrawCamerasEnabled(bool enabled);
    bool getDrawCamerasEnabled() const;
    
    private:

    float getMaxSceneDim() const;
    void render(RenderPlugin &p, int camIndex);
    
    std::vector<Camera> m_cameras;
    std::vector<Object2*> m_objects;
    std::vector<Object2*> m_cameraObjects;
    std::vector<std::vector<std::vector<Vec> > >m_projections;//[cam][obj][vertex]
    
#ifdef HAVE_QT
    std::vector<SceneMouseHandler*> m_mouseHandlers;
#endif

    std::vector<GLCallback*> m_glCallbacks;
    
    bool m_lightSimulationEnabled;
    bool m_drawCamerasEnabled;
  };
}

#endif
