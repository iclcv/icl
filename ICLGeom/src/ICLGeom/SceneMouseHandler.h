/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLGeom/src/ICLGeom/SceneMouseHandler.h                **
** Module : ICLGeom                                                **
** Authors: Christof Elbrechter, Erik Weitnauer, Daniel Dornbusch  **
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

#if !defined(ICL_HAVE_OPENGL) || !defined(ICL_HAVE_QT)
  #ifdef WIN32
#pragma WARNING("this header must not be included if ICL_HAVE_OPENGL or ICL_HAVE_QT is not defined")
  #else
    #warning "this header must not be included if ICL_HAVE_OPENGL or ICL_HAVE_QT is not defined"
  #endif
#else


#include <ICLUtils/CompatMacros.h>
#include <ICLGeom/Camera.h>
#include <ICLQt/MouseHandler.h>

namespace icl{

  /** \cond */
  namespace qt{
    class GUI;
  }
  /** \endcond */

  namespace geom{

    /// forward declaration of scene class
    class Scene;

    /// Mouse action function pointer
    /** parameters:
        const qt::MouseEvent&               pMouseEvent,
        const utils::Point32f&              pCurrentMousePosition,
        const utils::Point32f&              pDeltaMousePosition,
        Camera&                             pCamera,
        Scene&                              pScene,
        void*                               pData
        **/
    typedef void (*MouseActionCallback)(const qt::MouseEvent&,const utils::Point32f&,
                                        const utils::Point32f&,Camera&,Scene&,void* );



    /// mouse mapping table entry
    struct MouseMappingTableEntry{
      /// pointer to mouse action function
      MouseActionCallback mMouseAction;

      /// pointer to additional data
      void* mData;
    };


    /// mouse sensitivities
    struct MouseSensitivities{
      /// sensitivity factor for translation (e.g. mParentScene->getMaxSceneDim())
      float mTranslation;

      /// sensitivity factor for rotations (e.g. 1.0)
      float mRotation;

      /// sensitivity of mouse movements (e.g. 1.0)
      float mMouse;

      /// sensitivity of mouse wheel (e.g. 0.001)
      float mWheel;
    };


    /// mouse sensitivities modifier
    enum MouseSensitivitiesModifier{
      LowMouseSensitivity    = 0,
      NormalMouseSensitivity = 1,
      HighMouseSensitivity   = 2,
      MAX_MOUSE_SENSITIVITY  = HighMouseSensitivity
    };


    /// Class providing a mouse handler for class scene.

    /** Create your own mouse handler by inherting from this class and
        overloading function setMouseMappings().

        Default mouse mappings for scene objects
        - left mouse button: free view
        - middle mouse button: strafe
        - right mouse button: rotation around origin
        - mouse wheel: roll and camera movement forward / backward
        - left + right mouse button: same as mouse wheel

        Mouse sensitivity modifiers in combination with mouse actions
        - Shift: low sensitivity
        - Control: high sensitivity

        New mouse mappings
        - inherit a mouse handler from SceneMouseHandler class
        - if desired, define new static mouse action functions (like strafe)
        - overload function setMouseMappings() to bind mouse action functions to mouse actions
        - mouse action functions can be assigned to all combinations of
        - mouse events (mouse move, button pressed down and held,
          button pressed, button released, area entered, area left, mouse wheel)
        - mouse buttons (left, middle, right),
        - keyboard modifiers (shift, control, alt)
        - mouse action functions will be called with an additional data pointer (void*)
        to grant access to all kind of desired data.
        */
    class SceneMouseHandler : public qt::MouseHandler{
      protected:

      ///  Mouse mapping table:
      /** - array dimensions: [MouseEventType] [LeftMouseButton]
            [MiddleMouseButton] [RightMouseButton] [Shift] [Control] [Alt]
          - qt::MouseEventType: MouseMoveEvent, MouseDragEvent,
            MousePressEvent, MouseReleaseEvent, MouseEnterEvent, MouseLeaveEvent
          - Mouse buttons, keyboard modifiers: true, false
          */
      MouseMappingTableEntry mMouseMappingTable[qt::MAX_MOUSE_EVENT + 1][2][2][2][2][2][2];

      /// mouse sensitivities
      MouseSensitivities mMouseSensitivities[MAX_MOUSE_SENSITIVITY + 1];

      /// backup of old camera
      Camera mCameraBackup;

      /// starting mouse position for dragging
      utils::Point32f mAnchor;

      /// pointer to parent scene
      Scene* mParentScene;

      /// index of camera in scene
      int mCameraIndex;

      /// backup of old keyboard modifiers
      int mKeyboardModifiersBackup;

      /// GUI for the adaption of scene properties
      qt::GUI *mGUI;

      public:

      /// Constructor.
      /** @param pCameraIndex index of camera in scene
          @param pParentScene pointer to parent scene
          */
      ICLGeom_API SceneMouseHandler(const int pCameraIndex, Scene* pParentScene );

      /// Copy constructor.
      /** @param pSceneMouseHandler source */
      SceneMouseHandler(const SceneMouseHandler& pSceneMouseHandler){
        *this = pSceneMouseHandler;
      }

      /// Destructor
      ICLGeom_API ~SceneMouseHandler();

      /// Assignment operator.
      ICLGeom_API SceneMouseHandler& operator=(const SceneMouseHandler& pSceneMouseHandler);

      /// Set parent scene.
      void setParentScene(Scene* pParentScene){
        mParentScene = pParentScene;
      }

      /// Get parent scene.
      Scene* getParentScene(){
        return mParentScene;
      }

      /// Set mouse & wheel sensitivities, modifier factor and factors for rotation and translation
      /** @param pTranslation sensitivity factor for translation (e.g. mParentScene->getMaxSceneDim())
          @param pRotation sensitivity factor for rotations (e.g. 1.0)
          @param pMouse sensitivity of mouse movements (e.g. 1.0)
          @param pWheel sensitivity of mouse wheel (e.g. 0.001)
          @param pModifier factor to modify sensitivity (e.g. 10.0)
          */
      ICLGeom_API void setSensitivities(const float pTranslation, const float pRotation = 1.0,
                            const float pMouse = 1.0, const float pWheel = 0.001,
                            const float pModifier = 10.0 );


      /// Get mouse and wheel sensitivities (low, normal, high).
      /** * @return sensitivity */
      MouseSensitivities getSensitivities(MouseSensitivitiesModifier pMouseSensitivitiesModifier){
        return mMouseSensitivities[pMouseSensitivitiesModifier];
      };


      /// Set camera index.
      /** @param pCameraIndex new camera index */

      void setCameraIndex(const int pCameraIndex){
        mCameraIndex = pCameraIndex;
      }


      /// Get camera index.
      /** @return camera index */
      int getCameraIndex(){
        return mCameraIndex;
      }


      /// Set mouse mappings.
      /** Inherit from this class and overload this function to define new mouse mappings. */
      ICLGeom_API virtual void setMouseMappings();


      /// Set one mouse mapping.
      /** @param pMouseEventType type of mouse event
          @param pLeftMouseButton left mouse button
          @param pMiddleMouseButton middle mouse button
          @param pRightMouseButton right mouse button
          @param pShift shift
          @param pControl control
          @param pAlt alt
          @param pMouseActionCallback function pointer that should be called
          @param pData pointer to additional data */
      ICLGeom_API void setMouseMapping(const qt::MouseEventType pMouseEventType,
                           const bool pLeftMouseButton,
                           const bool pMiddleMouseButton,
                           const bool pRightMouseButton,
                           const bool pShift,
                           const bool pControl,
                           const bool pAlt,
                           MouseActionCallback pMouseActionCallback,
                           void* pData=0);


      /// Free view
      /** Vertical mouse movement: pitch.
          Horizontal mouse movement: yaw.
          @param pMouseEvent mouse event
          @param pCurrentMousePosition current mouse position
          @param pDeltaMousePosition delta compared to last mouse position
          @param pCamera camera
          @param pData pointer for additional data used to set sensitivity
          @param pInverseX inverse x-axis
          @param pInverseY inverse y-axis */

      ICLGeom_API static void freeView(const qt::MouseEvent &pMouseEvent,
                           const utils::Point32f &pCurrentMousePosition,
                           const utils::Point32f &pDeltaMousePosition,
                           Camera &pCamera, Scene &pScene, void *pData,
                           bool pInverseX, bool pInverseY);


    /// Free view with normal axes.
    /** Vertical mouse movement: pitch.
        Horizontal mouse movement: yaw.
        @param pMouseEvent mouse event
        @param pCurrentMousePosition current mouse position
        @param pDeltaMousePosition delta compared to last mouse position
        @param pCamera camera
        @param pData pointer for additional data used to set sensitivity */
      static void freeView(const qt::MouseEvent &pMouseEvent,
                           const utils::Point32f &pCurrentMousePosition,
                           const utils::Point32f &pDeltaMousePosition,
                           Camera &pCamera, Scene &pScene, void *pData){
        freeView( pMouseEvent, pCurrentMousePosition, pDeltaMousePosition, pCamera, pScene, pData, false, false );
      }


      /// Free view with inversed x-axis.
      /** Vertical mouse movement: pitch.
          Horizontal mouse movement: yaw.
          @param pMouseEvent mouse event
          @param pCurrentMousePosition current mouse position
          @param pDeltaMousePosition delta compared to last mouse position
          @param pCamera camera
          @param pData pointer for additional data used to set sensitivity */
      static void freeViewInverseMouseX(const qt::MouseEvent &pMouseEvent,
                                        const utils::Point32f &pCurrentMousePosition,
                                        const utils::Point32f &pDeltaMousePosition,
                                        Camera &pCamera, Scene &pScene, void *pData){
        freeView( pMouseEvent, pCurrentMousePosition, pDeltaMousePosition, pCamera, pScene, pData, true, false );
      }


      /// Free view with inversed y-axis.
      /** Vertical mouse movement: pitch.
          Horizontal mouse movement: yaw.
          @param pMouseEvent mouse event
          @param pCurrentMousePosition current mouse position
          @param pDeltaMousePosition delta compared to last mouse position
          @param pCamera camera
          @param pData pointer for additional data used to set sensitivity */
      static void freeViewInverseMouseY(const qt::MouseEvent &pMouseEvent,
                                        const utils::Point32f &pCurrentMousePosition,
                                        const utils::Point32f &pDeltaMousePosition,
                                        Camera &pCamera, Scene &pScene, void *pData){
        freeView( pMouseEvent, pCurrentMousePosition, pDeltaMousePosition, pCamera, pScene, pData, false, true );
      }


      /// Free view with inversed axes.
      /** Vertical mouse movement: pitch.
          Horizontal mouse movement: yaw.
          @param pMouseEvent mouse event
          @param pCurrentMousePosition current mouse position
          @param pDeltaMousePosition delta compared to last mouse position
          @param pCamera camera
          @param pData pointer for additional data used to set sensitivity */

      static void freeViewInverseBoth(const qt::MouseEvent &pMouseEvent,
                                      const utils::Point32f &pCurrentMousePosition,
                                      const utils::Point32f &pDeltaMousePosition,
                                      Camera &pCamera, Scene &pScene, void *pData){
        freeView( pMouseEvent, pCurrentMousePosition, pDeltaMousePosition, pCamera, pScene, pData, true, true );
      }


      /// Rotate around origin and correct camera orientation.
      /** Vertical mouse movement: vertical rotation.
          Horizontal mouse movement: horizontal rotation.
          @param pMouseEvent mouse event
          @param pCurrentMousePosition current mouse position
          @param pDeltaMousePosition delta compared to last mouse position
          @param pCamera camera
          @param pData pointer for additional data used to set sensitivity */
      ICLGeom_API static void rotateAroundOrigin(const qt::MouseEvent &pMouseEvent,
                                     const utils::Point32f &pCurrentMousePosition,
                                     const utils::Point32f &pDeltaMousePosition,
                                     Camera &pCamera, Scene &pScene, void *pData);

      /// Strafe (camera movement up, down, left, right).
      /** Vertical mouse movement: move camera along right vector.
          Horizontal mouse movement: move camera along up vector.
          @param pMouseEvent mouse event
          @param pCurrentMousePosition current mouse position
          @param pDeltaMousePosition delta compared to last mouse position
          @param pCamera camera
          @param pData pointer for additional data used to set sensitivity */
      ICLGeom_API static void strafe(const qt::MouseEvent &pMouseEvent,
                         const utils::Point32f &pCurrentMousePosition,
                         const utils::Point32f &pDeltaMousePosition,
                         Camera &pCamera, Scene &pScene, void *pData);

      /// Roll and distance (camera movement forward and backward).
      /** Vertical mouse movement: move camera along front vector.
          Horizontal mouse movement: roll camera.
          @param pMouseEvent mouse event
          @param pCurrentMousePosition current mouse position
          @param pDeltaMousePosition delta compared to last mouse position
          @param pCamera camera
          @param pData pointer for additional data used to set sensitivity */
      ICLGeom_API static void rollAndDistance(const qt::MouseEvent &pMouseEvent,
                                  const utils::Point32f &pCurrentMousePosition,
                                  const utils::Point32f &pDeltaMousePosition,
                                  Camera &pCamera, Scene &pScene, void *pData);

      /// Place the cursor for rotational Origin
      /** @param pMouseEvent mouse event
          @param pCurrentMousePosition current mouse position
          @param pDeltaMousePosition delta compared to last mouse position
          @param pCamera camera
          @param pData pointer for additional data used to set sensitivity */
      ICLGeom_API static void placeCursor(const qt::MouseEvent &pMouseEvent,
                                  const utils::Point32f &pCurrentMousePosition,
                                  const utils::Point32f &pDeltaMousePosition,
                                  Camera &pCamera, Scene &pScene, void *pData);


      /// Process mouse event using mouse mapping table.
      /** @param pMouseEvent mouse event */
      ICLGeom_API virtual void process(const qt::MouseEvent &pMouseEvent);
    };

  } // namespace geom
}

#endif
