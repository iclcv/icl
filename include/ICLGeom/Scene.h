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

  /// Scene Implementation that is used to handle a list of objects and cameras
  /** The scene combines visual objects (icl::SceneObject) which define nodes of a <b>scene graph</b>
      and cameras. You can use ICL's camera calibration tool icl-cam-calib to calibrate
      you camera in your scene. Later, you can define a scene (including SceneObject-instances) and
      use the calibrated camera to draw the Scene as an image overlay.
      
      The following example demonstrates how to use the Scene class. The example can also be
      found at ICLGeom/examples/geom-demo-simple.cpp. A more complex demo that also uses the Scene's 
      scene graph can be found at ICLGeom/examples/scene-graph-demo.cpp

      \code
#include <ICLQuick/Common.h>
#include <ICLGeom/Geom.h>
#include <ICLUtils/FPSLimiter.h>

// global data
GUI gui;
Scene scene;

void init(){
  // create graphical user interface
  gui << "draw3D[@minsize=16x12@handle=draw@label=scene view]"
      << "fslider(0.5,20,3)[@out=f@handle=focal"
         "@label=focal length@maxsize=100x3]";
  gui.show();
  
  // defines the scene's viewport
  (**gui.getValue<DrawHandle3D>("draw")).setDefaultImageSize(Size(640,480));

  // create camera and add to scene instance
  Camera cam(Vec(0,0,-10), // position
             Vec(0,0,1),   // view-direction
             Vec(1,0,0));   // up-direction
  scene.addCamera(cam);

  
  if(pa("-o")){ // either load an .obj file
    scene.addObject(new SceneObject(*pa("-o")));
  }else{ // or create a simple cube
    float data[] = {0,0,0,7,3,2};
    scene.addObject(new SceneObject("cuboid",data));
  }

  // link the scene's first camera with mouse gestures in the draw-GUI-component
  gui["draw"].install(scene.getMouseHandler(0));
}


void run(){
  DrawHandle3D draw = gui["draw"]; // get the draw-GUI compoment
  scene.getCamera(0).setFocalLength(gui["f"]); // update the camera's focal length

  // now simply copy and past this block ...
  draw->lock();    // lock the internal drawing queue
  draw->reset3D(); // remove former drawing commands
  draw->callback(scene.getGLCallback(0)); // render the whole scene
  draw->unlock();  // unlock the internal drawin queue
  draw.update();   // post an update-event (don't use draw->update() !!)

  /// limit drawing speed to 25 fps
  static FPSLimiter limiter(25);
  limiter.wait();
}


int main(int n, char**ppc){
  /// create a whole application 
  return ICLApplication(n,ppc,"-obj|-o(.obj-filename)",init,run).exec();
}
      \endcode

  */
  class Scene : public Lockable{
    public:

#ifdef HAVE_OPENGL
    struct GLCallback;
#endif
    
    /// Base constructor (creates an empty scene)
    Scene();
    
    /// Destructor
    ~Scene();
    
    /// Explicitly implemented deep copy (performs complete deep copy of all cameras and objects)
    Scene(const Scene &scene);
    
    /// Assignment operator (complete deep copy)
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
    
    /// removed the camera at given index
    void removeCamera(int index);
    
    /// returns a reference to the i-th camera
    /** This can also be used to set the camera or to set's some of it's properties:
        \code
        // set a totally new camera
        scene.getCamera(0) = Camera(...);
        
        // update the focal length only
        scene.getCamera(0).setFocalLength(77);
        \endcode
    */
    Camera &getCamera(int camIndex = 0);
    
    /// returns the i-th scene camera
    const Camera &getCamera(int camIndex =0) const;
    
    /// returns the count of contained cameras
    inline int getCameraCount() const { return (int)m_cameras.size(); }

    /// adds a new top-level object to the Scene instance 
    /** By default, the object's memory is managed externally. If you want
        to pass the ownership to the Scene instance, you have to set
        passOwnerShip to true */
    void addObject(SceneObject *object, bool passOwnerShip=false);
    
    /// removed object at given index
    /** The object is deleted if it's ownwership was passed */
    void removeObject(int idx);

    /// removed object at given indices
    /** The object's are deleted if their ownwership was passed */
    void removeObjects(int startIndex, int endIndex=-1);
    
    /// returns the number of top-level objects
    int getObjectCount() const { return m_objects.size(); }

    /// deletes and removes all objects, handlers and callbacks
    /** If camerasToo is set to true, also all cameras are removed */
    void clear(bool camerasToo=false);

#ifdef HAVE_QT
#ifdef HAVE_OPENGL
    
    /// returns a mouse handler that adapts the scene's camera using mouse-interaction
    MouseHandler *getMouseHandler(int camIndex=0);
    
    /// registeres a custom SceneMouseHandler for given camera index
    void setMouseHandler(SceneMouseHandler* sceneMouseHandler, int camIndex=0);
    
    /// returns a callback that is used to render the scene into a GL-context
    /** please see ICLQt::ICLDrawWidget3D::callback */
    ICLDrawWidget3D::GLCallback *getGLCallback(int camIndex);
#endif
#endif

    /// sets wheter cameras are also visualized in scenes.
    /** This means, that you will be able to see e.g. camera 1 in the view of camera 0 */
    void setDrawCamerasEnabled(bool enabled);
    
    /// returns wether cameras are visualized
    bool getDrawCamerasEnabled() const;
    
    /// sets wheter a coordinate frame is automatically inserted into the scene
    void setDrawCoordinateFrameEnabled(bool enabled, float axisLength=100, float axisThickness=5);
    
    /// returns wheter a coordinate frame is automatically shown in the scene
    /** Optionally, destination pointers can be given to query the current coordinate frames parameters */
    bool getDrawCoordinateFrameEnabled(float *axisLength=0,float *axisThickness=0) const;
    
    private:
#ifdef HAVE_QT
#ifdef HAVE_OPENGL
    /// internally used rendering method
    void render(int camIndex, ICLDrawWidget3D *widget);

    /// internally used rendering method for recursive rendering of the scene graph
    void renderSceneObjectRecursive(SceneObject *o);
#endif
#endif
    /// internally used utility method that computes the extend of the Scene content
    /** The extend is used when scene mouse handlers are created. Here, it will e.g. 
        compute a usefull step when moving forward or strafing. */
    float getMaxSceneDim() const;
    
    /// recursive utility method 
    void extendMaxSceneDimRecursive(float &minX, float &maxX, 
                               float &minY, float &maxY, 
                               float &minZ, float &maxZ,
                               SceneObject *o) const;

    /// internal list of cameras
    std::vector<Camera> m_cameras;

    /// internal list of top-level objects
    std::vector<SmartPtr<SceneObject> > m_objects;

    /// internal list of top-level camera objects used for camera visualization
    std::vector<SmartPtr<SceneObject> > m_cameraObjects;

#ifdef HAVE_QT
#ifdef HAVE_OPENGL
    
    /// internally used list of mouse handlers
    std::vector<SmartPtr<SceneMouseHandler> > m_mouseHandlers;

    /// internally used list of callbacks
    std::vector<SmartPtr<GLCallback> > m_glCallbacks;
#endif
#endif
    
    /// internally used flag
    bool m_drawCamerasEnabled;
    
    /// internally used flag
    bool m_drawCoordinateFrameEnabled;

    /// internally used scene object
    SmartPtr<SceneObject> m_coordinateFrameObject;
  };
}

#endif
