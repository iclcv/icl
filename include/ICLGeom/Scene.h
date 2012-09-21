/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2012 CITEC, University of Bielefeld          **
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

#pragma once

#ifndef HAVE_OPENGL
#warning "this header must not be included if HAVE_OPENGL is not defined"
#else

#include <ICLGeom/SceneObject.h>
#include <ICLGeom/Camera.h>
#include <ICLGeom/SceneLight.h>
#include <ICLCore/Img.h>

#ifdef HAVE_QT
#include <ICLQt/MouseHandler.h>
#include <ICLQt/DrawWidget3D.h>
#include <ICLGeom/SceneMouseHandler.h>
#endif

#include <ICLUtils/Lockable.h>
#include <ICLUtils/SmartArray.h>


namespace icl{
    /** \cond */
  namespace qt {class ICLDrawWidget;}
    /** \endcond */

  namespace geom{
 
    /// Scene Implementation that is used to handle a list of objects and cameras
    /** The scene combines visual objects (icl::SceneObject) which define nodes of a <b>scene graph</b>
        and cameras. You can use ICL's camera calibration tool icl-cam-calib (and icl-cam-calib-2) to calibrate
        cameras in your scene. Later, you can define a scene (including SceneObject-instances) and
        use the calibrated camera to draw the Scene as an image overlay.
        
        The following example demonstrates how to use the Scene class. The example can also be
        found at ICLGeom/examples/geom-demo-simple.cpp. A more complex demo that also uses the Scene's 
        scene graph can be found at ICLGeom/examples/scene-graph-demo.cpp
  
        \code
  #include <ICLQt/Common.h>
  #include <ICLGeom/Geom.h>
  #include <ICLUtils/FPSLimiter.h>

  // global data
  icl::qt::GUI gui;
  Scene scene;

  void init(){
    // create graphical user interface with a scene's viewport set of size 640x480
    gui << Draw3D(utils::Size(640,480)).minSize(16,12).handle("draw").label("scene view")
        << FSlider(0.5,20,3).out("f").handle("focal").label("focal length").maxSize(100,3);
    gui.show();

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
    draw->link(scene.getGLCallback(0)); // render the whole scene
    draw.render();   // post an update-event (don't use draw->render() !!)

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
    class Scene : public utils::Lockable{
      public:

      /// make SceneObject friend of Scene
      friend class SceneObject;
    
      struct GLCallback;
    
      /// Base constructor (creates an empty scene)
      Scene();
    
      /// Destructor
      ~Scene();
    
      /// Explicitly implemented deep copy (performs complete deep copy of all cameras and objects)
      Scene(const Scene &scene);
    
      /// Assignment operator (complete deep copy)
      Scene &operator=(const Scene &scene);

      /// Adds a new Camera to the scene
      /** @param cam which is copied into the scene
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

      /// returns a vector with pointers to all cameras in given range
      /** Please note, that the results become invalid if addCamera() or
          removeCamera() was called. If 'num' selects more cameras than
          possible, is it just increased appropriately automatically. */
      std::vector<Camera*> getAllCameras(int firstIndex=0, int num=-1);
    
      /// adds a new top-level object to the Scene instance 
      /** By default, the object's memory is managed externally. If you want
          to pass the ownership to the Scene instance, you have to set
          passOwnerShip to true.
          */
      void addObject(SceneObject *object, bool passOwnerShip=false);
    
      /// removed object at given index
      /** The object is deleted if it's ownwership was passed */
      void removeObject(int idx);
    
      /// removes given top-level object from scene (not recursive)
      /** The object is deleted if it's ownwership was passed */
      void removeObject(const SceneObject *obj);

      /// removed object at given indices
      /** The object's are deleted if their ownwership was passed */
      void removeObjects(int startIndex, int endIndex=-1);
    
      /// returns the number of top-level objects
      inline int getObjectCount() const { return m_objects.size(); }
    
      /// returns contained object at given index
      /** If the index is not valid, an exception is thrown */
      SceneObject *getObject(int index) throw (utils::ICLException);

      /// returns contained object at given index (const)
      /** If the index is not valid, an exception is thrown */
      const SceneObject *getObject(int index) const throw (utils::ICLException);
    
      /// returns a child that is deeper in the scene graph
      /** e.g. if recursiveIndices is [1,2,3], then first, the Scene's object at 
          index 1 is used. Then this objects child at index 2. And finally that
          objects child at index 3 is returned. 
          An exception is thrown if one of the indices is wrong.
          */
      SceneObject *getObject(const std::vector<int> recursiveIndices) throw (utils::ICLException);

      /// returns a child that is deeper in the scene graph (const)
      /** e.g. if recursiveIndices is [1,2,3], then first, the Scene's object at 
          index 1 is used. Then this objects child at index 2. And finally that
          objects child at index 3 is returned. 
          An exception is thrown if one of the indices is wrong.
          */
      SceneObject *getObject(const std::vector<int> recursiveIndices) const throw (utils::ICLException);
    
      /// finds the recursive indices for a given object. 
      /** If no exceptions are thrown, getObject(findPath(o)) is always o.
          throws ans exception if the given object cannot be found. */
      std::vector<int> findPath(const SceneObject *o) const throw (utils::ICLException);
    
      /// deletes and removes all objects, handlers and callbacks
      /** If camerasToo is set to true, also all cameras are removed */
      void clear(bool camerasToo=false);

#ifdef HAVE_QT
      /// returns a mouse handler that adapts the scene's camera using mouse-interaction
      qt::MouseHandler *getMouseHandler(int camIndex=0);
    
      /// registeres a custom SceneMouseHandler for given camera index
      void setMouseHandler(SceneMouseHandler* sceneMouseHandler, int camIndex=0);
    
      /// returns a callback that is used to render the scene into a GL-context
      /** please see ICLQt::ICLDrawWidget3D::callback */
      qt::ICLDrawWidget3D::GLCallback *getGLCallback(int camIndex);
    
#ifdef HAVE_GLX
      enum DepthBufferMode{
        RawDepth01,      //!< raw core::depth buffer in range [0,1]
        DistToCamPlane,  //!< core::depth buffer values define distance to the camera's z=0 plane
        DistToCamCenter  //!< core::depth buffer values define distanct to the camera center
      };
    
    
      /// This function must be called when rendering a Scene from two different GLContexts
      /** This does in particular happen, when a single Scene is rendered on a widget and into
          an offsceen buffer. In this case, stored texture-ID's are only valid in a single thread,
          which is why, textures need to be updated everytime, they are drawn. 
        
          \section FIX Possible Fix in the Future
          We plan to fix this issue with a more elaborated texture ID handling in the ICLQt 
          GLImg class.
          */
      static void enableSharedOffscreenRendering();
    
      /// re-enables the dirty flag in the GLImg class 
      static void disableSharedOffscreenRendering();
    
      /// renders the current scene using an instance of glx pbuffer
      /** This method is currently only supported on linux systems, since
          the used pbuffer (OpenGL offscreen framebuffer object) 
        
          The method trys to create a default r8 g8 b8 pbuffer
          with 24Bit depthbuffer. If this is not supported, 
          an at least 4 4 4 16 context is tryed to be created. If this does also fail,
          an exception will be thrown.

          \section SOR Shared Offsceen Rendering
          When a single scene is used for both on- and offscreen rendering, an internal optimation
          needs to be deactivated by calling Scene::enableSharedOffscreenRendering.
        
          <b>Please note:</b> The rendering pbuffer is allocated on the graphics card. Per definition,
          pbuffers are located in the screenbuffer memory segment which might be much smaller than the
          actual memory of your graphics card. Therefore, it is strongly recommended to free all pbuffers
          when they are no longer used.
          The pbuffer that is created to render the image in this method is stored internally and it
          will remain allocated for later use. If you need to render images of different sizes
          (the output image is rendered to the image size that the camera at given index has), you should
          free the pbuffers from time to time using 
          Scene::freeAllPBuffers and Scene::freePBuffer(const utils::Size&).
        
          If a non-null depthBuffer parameter is provided, it is filled with data from the scene core::depth buffer
          usually, the raw core::depth buffer does not provide useful information for further processing. 
          In OpenGL, the standard core::depth buffer values are highly non-linearily distributed within the
          near and far clipping plane. If the core::depth buffer mode is not RawDepth01, this is compensated
          and the core::depth buffer is filled with (metric) core::depth values that denote either a pixel's distance
          to the z=0 plane in camera space or the distance to the camera center. For correcting the
          linearized core::depth buffer values that are computed for the DistToCamPlane mode, a set of correction 
          coefficients has to be computed, which entails creation of all camera viewrays. This can last
          about a second in the first call. In later calls, the values are just re-used. 
        
          */
      const core::Img8u &render(int camIndx, const core::ImgBase *background=0, core::Img32f *depthBuffer=0, 
                                DepthBufferMode mode=DistToCamCenter) const throw (utils::ICLException);
    
      /// frees all pbuffers allocated before
      void freeAllPBuffers();
    
      /// frees the pbffer associated with given size (if there is one)
      void freePBuffer(const utils::Size &size);
#endif
#endif

      /// sets wheter cameras are also visualized in scenes.
      /** This means, that you will be able to see e.g. camera 1 in the view of camera 0 */
      void setDrawCamerasEnabled(bool enabled);
    
      /// returns wether cameras are visualized
      bool getDrawCamerasEnabled() const;
    
      /// sets wheter a coordinate frame is automatically inserted into the scene
      void setDrawCoordinateFrameEnabled(bool enabled, float axisLength=100, 
                                         float axisThickness=5, bool simpleGeometry=false);
    
      /// returns wheter a coordinate frame is automatically shown in the scene
      /** Optionally, destination pointers can be given to query the current coordinate frames parameters */
      bool getDrawCoordinateFrameEnabled(float *axisLength=0,float *axisThickness=0) const;

      /// returns a reference to a light with given index
      /** The returned reference cam be used to set lighting parameters.
          Since OpenGL does only support 8 lights, allowed indices are 0-7.
          If another index is passed, an exception is thrown. */
      SceneLight &getLight(int index) throw (utils::ICLException);

      /// returns a const reference to a light with given index
      /** Since OpenGL does only support 8 lights, allowed indices are 0-7.
          If another index is passed, an exception is thrown. */
      const SceneLight &getLight(int index) const throw (utils::ICLException);

    
      /// sets whether OpenGL's lighting is globally activated
      /** by default, lighting is activated */
      void setLightingEnabled(bool flag);

      /// picks the closest contained scene-object clicked at given ScreenPosition
      /** returns 0 if no object was hit, if contactPos is not 0, the 3D-contact position
          is stored there. */
      inline Hit findObject(int camIndex, int xScreen, int yScreen){
        return findObject(getCamera(camIndex).getViewRay(utils::Point(xScreen,yScreen)));
      }
    
      /// picks the first object that is hit by the given viewray
      /** The first object that is returned has the smallest distance to the
          given viewRay's offset. If contactPos is not 0, the contact point is stored there. */
      Hit findObject(const ViewRay &v);
    
    
      /// retunrs all objects intersected by the given viewray
      std::vector<Hit> findObjects(const ViewRay &v);
    
      /// retunrs all objects on that are intersected by the defined cameras viewray through given x and y
      inline std::vector<Hit> findObjects(int camIndex, int xScreen, int ySceen){
        return findObjects(getCamera(camIndex).getViewRay(utils::Point(xScreen,ySceen)));
      }
    
      /// sets the expected bounds of contained objects
      /** The set information is used when a mouse-handler is created, in order to
          estimate appropriate step sizes. If the scene bounds were not explicitly,
          the scene tries to infer this information from the contained objects\n
          Each [min-max] range where min=max, is automatically set to [minX,maxX].
          If additionally, minX is equal to maxX, max is is set to -minX.
          If the resulting x-range has still zero length, the bounds are deleted
          internally */
      void setBounds(float minX, float maxX=0, float minY=0, float mayY=0, float minZ=0, float maxZ=0);

      protected:

      /// creates a displaylist for the given object
      void createDisplayList(SceneObject *o) const;

      /// frees the display list, that is associated with an object
      void freeDisplayList(SceneObject *o) const;

#ifdef HAVE_QT
      /// internally used rendering method
      void renderScene(int camIndex, qt::ICLDrawWidget3D *widget=0) const;
#endif
      /// internally used rendering method for recursive rendering of the scene graph
      void renderSceneObjectRecursive(SceneObject *o) const;

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
      std::vector<utils::SmartPtr<SceneObject> > m_objects;

      /// internal list of top-level camera objects used for camera visualization
      std::vector<utils::SmartPtr<SceneObject> > m_cameraObjects;

#ifdef HAVE_QT
      /// internally used list of mouse handlers
      std::vector<utils::SmartPtr<SceneMouseHandler> > m_mouseHandlers;

      /// internally used list of callbacks
      std::vector<utils::SmartPtr<GLCallback> > m_glCallbacks;
#endif


      /// internal class for offscreen rendering
      struct PBuffer;
    
      struct PBufferIndex : public utils::Size{
        unsigned int threadID;
        PBufferIndex(const utils::Size &size);
        bool operator<(const PBufferIndex &other) const;
      };
    
      /// intenal list of of offscreen rendering buffers
      mutable std::map<PBufferIndex,PBuffer*> m_pbuffers;

      /// internally used flag
      bool m_drawCamerasEnabled;
    
      /// internally used flag
      bool m_drawCoordinateFrameEnabled;

      /// internally used scene object
      utils::SmartPtr<SceneObject> m_coordinateFrameObject;

      /// internally used flag that indicates whether lighting is globally activated or not
      bool m_lightingEnabled;
    
      /// internal list of lights
      utils::SmartPtr<SceneLight> m_lights[8];
    
      /// optionally given bounds of the scene
      utils::SmartArray<utils::Range32f> m_bounds;

      private:
      /// called from the SceneObject class
      static void freeDisplayList(void *handle);
    };
  } // namespace geom

}

#endif
