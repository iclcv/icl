/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2010 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLGeom/src/SceneMouseHandler.cpp                      **
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

#include <ICLGeom/SceneMouseHandler.h>


#include <ICLQuick/Quick.h>
#include <ICLGeom/GeomDefs.h>
#include <ICLGeom/Scene.h>


namespace icl{

#ifdef HAVE_QT


SceneMouseHandler::SceneMouseHandler(
    const int                       pCameraIndex,
    Scene*                          pParentScene )
:   mParentScene( pParentScene ),
    mCameraIndex( pCameraIndex ),
    mKeyboardModifiersBackup( 0 )
{
    // clear mouse mapping table (2^6 = 64)
    memset( mMouseMappingTable, 0, sizeof( MouseMappingTableEntry ) * ( MAX_MOUSE_EVENT + 1 ) * 64 );

    // mouse sensitivities
    setSensitivities( 10.0, 1.0, 1.0, 0.001, 10.0 );

    // default mouse mappings
    setMouseMappings();
}



SceneMouseHandler& SceneMouseHandler::operator=(
    const SceneMouseHandler&        pSceneMouseHandler )
{
    // copy mouse mapping table (2^6 = 64)
    memcpy( mMouseMappingTable, pSceneMouseHandler.mMouseMappingTable,
            sizeof( MouseMappingTableEntry ) * ( MAX_MOUSE_EVENT + 1 ) * 64 );

    // copy mouse sensitivity settings
    for ( unsigned int i(0); i < MAX_MOUSE_SENSITIVITY + 1; ++i )
        mMouseSensitivities[i] = pSceneMouseHandler.mMouseSensitivities[i];

    mCameraBackup            = pSceneMouseHandler.mCameraBackup;
    mAnchor                  = pSceneMouseHandler.mAnchor;
    mParentScene             = pSceneMouseHandler.mParentScene;
    mCameraIndex             = pSceneMouseHandler.mCameraIndex;
    mKeyboardModifiersBackup = pSceneMouseHandler.mKeyboardModifiersBackup;

    return *this;
}



void SceneMouseHandler::setSensitivities(
    const float                     pTranslation,
    const float                     pRotation,
    const float                     pMouse,
    const float                     pWheel,
    const float                     pModifier )
{
    // check translation input
    if ( pTranslation == 0.0 )
    {
        setSensitivities( 10.0, pRotation, pMouse, pWheel, pModifier );
        WARNING_LOG( "pTranslation was 0.0. That is not allowed. Changed to 10.0. \n" <<
                     "Please add objects to scene before calling setSensitivities." );
        return;
    }

    // check modifier input
    if ( pModifier == 0.0 )
    {
        setSensitivities( pTranslation, pRotation, pMouse, pWheel, 10.0 );
        WARNING_LOG( "pModifier was 0.0. That is not allowed. Changed to 10.0." );
        return;
    }

    // calculate low sensitivites
    mMouseSensitivities[LowMouseSensitivity].mTranslation    = pTranslation;
    mMouseSensitivities[LowMouseSensitivity].mRotation       = pRotation;
    mMouseSensitivities[LowMouseSensitivity].mMouse          = pMouse       / pModifier;
    mMouseSensitivities[LowMouseSensitivity].mWheel          = pWheel       / pModifier;

    // calculate normal sensitivites
    mMouseSensitivities[NormalMouseSensitivity].mTranslation = pTranslation;
    mMouseSensitivities[NormalMouseSensitivity].mRotation    = pRotation;
    mMouseSensitivities[NormalMouseSensitivity].mMouse       = pMouse;
    mMouseSensitivities[NormalMouseSensitivity].mWheel       = pWheel;

    // calculate high sensitivites
    mMouseSensitivities[HighMouseSensitivity].mTranslation   = pTranslation;
    mMouseSensitivities[HighMouseSensitivity].mRotation      = pRotation;
    mMouseSensitivities[HighMouseSensitivity].mMouse         = pMouse       * pModifier;
    mMouseSensitivities[HighMouseSensitivity].mWheel         = pWheel       * pModifier;
}



void SceneMouseHandler::setMouseMappings()
{
    // Mapping:
    //   mouse event type
    //   mouse button: left, right, middle
    //   keyboard modifier: shift, control, alt
    //   function to call
    //   additional data

    // left mouse button: free view
    setMouseMapping( MouseDragEvent, true, false, false, false, false, false, &freeView, &mMouseSensitivities[NormalMouseSensitivity] );
    setMouseMapping( MouseDragEvent, true, false, false, true,  false, false, &freeView, &mMouseSensitivities[LowMouseSensitivity] );
    setMouseMapping( MouseDragEvent, true, false, false, false, true,  false, &freeView, &mMouseSensitivities[HighMouseSensitivity] );

    // middle mouse button: strafe
    setMouseMapping( MouseDragEvent, false, true, false, false, false, false, &strafe, &mMouseSensitivities[NormalMouseSensitivity] );
    setMouseMapping( MouseDragEvent, false, true, false, true,  false, false, &strafe, &mMouseSensitivities[LowMouseSensitivity] );
    setMouseMapping( MouseDragEvent, false, true, false, false, true,  false, &strafe, &mMouseSensitivities[HighMouseSensitivity] );

    // right mouse button: rotate around origin
    setMouseMapping( MouseDragEvent, false, false, true, false, false, false, &rotateAroundOrigin, &mMouseSensitivities[NormalMouseSensitivity] );
    setMouseMapping( MouseDragEvent, false, false, true, true,  false, false, &rotateAroundOrigin, &mMouseSensitivities[LowMouseSensitivity] );
    setMouseMapping( MouseDragEvent, false, false, true, false, true,  false, &rotateAroundOrigin, &mMouseSensitivities[HighMouseSensitivity] );

    // mouse wheel: roll and distance
    setMouseMapping( MouseWheelEvent, false, false, false, false, false, false, &rollAndDistance, &mMouseSensitivities[NormalMouseSensitivity] );
    setMouseMapping( MouseWheelEvent, false, false, false, true,  false, false, &rollAndDistance, &mMouseSensitivities[LowMouseSensitivity] );
    setMouseMapping( MouseWheelEvent, false, false, false, false, true,  false, &rollAndDistance, &mMouseSensitivities[HighMouseSensitivity] );

    // left + right mouse button: roll and distance
    setMouseMapping( MouseDragEvent, true, false, true, false, false, false, &rollAndDistance, &mMouseSensitivities[NormalMouseSensitivity] );
    setMouseMapping( MouseDragEvent, true, false, true, true,  false, false, &rollAndDistance, &mMouseSensitivities[LowMouseSensitivity] );
    setMouseMapping( MouseDragEvent, true, false, true, false, true,  false, &rollAndDistance, &mMouseSensitivities[HighMouseSensitivity] );
}



void SceneMouseHandler::setMouseMapping(
    const MouseEventType            pMouseEventType,
    const bool                      pLeftMouseButton,
    const bool                      pMiddleMouseButton,
    const bool                      pRightMouseButton,
    const bool                      pShift,
    const bool                      pControl,
    const bool                      pAlt,
    MouseActionCallback             pMouseActionCallback,
    void*                           pData )
{
    // set callback function
    mMouseMappingTable[pMouseEventType]
                      [pLeftMouseButton]
                      [pMiddleMouseButton]
                      [pRightMouseButton]
                      [pShift]
                      [pControl]
                      [pAlt].mMouseAction = pMouseActionCallback;

    // set additional data
    mMouseMappingTable[pMouseEventType]
                      [pLeftMouseButton]
                      [pMiddleMouseButton]
                      [pRightMouseButton]
                      [pShift]
                      [pControl]
                      [pAlt].mData = pData;
}



void SceneMouseHandler::freeView(
    const MouseEvent&               pMouseEvent,
    const Point32f&                 pCurrentMousePosition,
    const Point32f&                 pDeltaMousePosition,
    Camera&                         pCamera,
    void*                           pData,
    bool                            pInverseX,
    bool                            pInverseY )
{
    // sensitivities given by additional data pointer
    MouseSensitivities* tMouseSensitivities = (MouseSensitivities*) pData;
    ICLASSERT( tMouseSensitivities );

    // factors
    float tRotationFactor = tMouseSensitivities->mRotation;
    float tDeviceFactor = ( pMouseEvent.isWheelEvent() ) ? tMouseSensitivities->mWheel
                                                         : tMouseSensitivities->mMouse;
    float tInverseXFactor = pInverseX ? -1.0 : 1.0;
    float tInverseYFactor = pInverseY ? -1.0 : 1.0;

    // rotate norm around up (by -pDeltaMousePosition.x)
    Vec tNorm = pCamera.getNorm();
    Vec tUp   = pCamera.getUp();
    tNorm = rotate_vector( tUp, -tInverseXFactor * tRotationFactor * tDeviceFactor * pDeltaMousePosition.x, tNorm );

    // rotate norm and up around horz (by delta.y)
    tNorm = rotate_vector( pCamera.getHoriz(), tInverseYFactor * tRotationFactor * tDeviceFactor * pDeltaMousePosition.y, tNorm );
    tUp   = rotate_vector( pCamera.getHoriz(), tInverseYFactor * tRotationFactor * tDeviceFactor * pDeltaMousePosition.y, tUp );

    // adjust camera
    pCamera.setNorm( tNorm );
    pCamera.setUp( tUp );
}



void SceneMouseHandler::rotateAroundOrigin(
    const MouseEvent&               pMouseEvent,
    const Point32f&                 pCurrentMousePosition,
    const Point32f&                 pDeltaMousePosition,
    Camera&                         pCamera,
    void*                           pData )
{
    // sensitivities given by additional data pointer
    MouseSensitivities* tMouseSensitivities = (MouseSensitivities*) pData;
    ICLASSERT( tMouseSensitivities );

    // factors
    float tRotationFactor = 2.0 * tMouseSensitivities->mRotation;
    float tDeviceFactor = ( pMouseEvent.isWheelEvent() ) ? tMouseSensitivities->mWheel
                                                         : -tMouseSensitivities->mMouse;

    // get camera parameters
    Vec tPosition = pCamera.getPosition();
    Vec tNorm     = pCamera.getNorm();
    Vec tUp       = pCamera.getUp();
    Vec tHoriz    = pCamera.getHoriz();

    // rotate camera around origin
    tPosition = rotate_vector( tUp,   -tRotationFactor * tDeviceFactor * pDeltaMousePosition.x, tPosition );
    tPosition = rotate_vector( tHoriz, tRotationFactor * tDeviceFactor * pDeltaMousePosition.y, tPosition );

    // adjust pitch and yaw
    tNorm = rotate_vector( tHoriz, tRotationFactor * tDeviceFactor * pDeltaMousePosition.y, tNorm );
    tUp   = rotate_vector( tHoriz, tRotationFactor * tDeviceFactor * pDeltaMousePosition.y, tUp );
    tNorm = rotate_vector( tUp,   -tRotationFactor * tDeviceFactor * pDeltaMousePosition.x, tNorm );

    // adjust camera
    pCamera.setPosition( tPosition );
    pCamera.setNorm( tNorm );
    pCamera.setUp( tUp );
}



void SceneMouseHandler::strafe(
    const MouseEvent&               pMouseEvent,
    const Point32f&                 pCurrentMousePosition,
    const Point32f&                 pDeltaMousePosition,
    Camera&                         pCamera,
    void*                           pData )
{
    // sensitivities given by additional data pointer
    MouseSensitivities* tMouseSensitivities = (MouseSensitivities*) pData;
    ICLASSERT( tMouseSensitivities );

    // factors
    float tTranslationFactor = tMouseSensitivities->mTranslation;
    float tDeviceFactor = ( pMouseEvent.isWheelEvent() ) ? tMouseSensitivities->mWheel
                                                         : tMouseSensitivities->mMouse;

    // strafe camera
    Vec tTranslationVector = pCamera.getUp()    * ( -tTranslationFactor * tDeviceFactor * pDeltaMousePosition.y ) +
                             pCamera.getHoriz() * ( -tTranslationFactor * tDeviceFactor * pDeltaMousePosition.x );
    tTranslationVector[3] = 0;

    // adjust camera
    pCamera.translate( tTranslationVector );
}



void SceneMouseHandler::rollAndDistance(
    const MouseEvent&               pMouseEvent,
    const Point32f&                 pCurrentMousePosition,
    const Point32f&                 pDeltaMousePosition,
    Camera&                         pCamera,
    void*                           pData )
{
    // sensitivities given by additional data pointer
    MouseSensitivities* tMouseSensitivities = (MouseSensitivities*) pData;
    ICLASSERT( tMouseSensitivities );

    // factors
    float tRotationFactor = tMouseSensitivities->mRotation;
    float tTranslationFactor = tMouseSensitivities->mTranslation;
    float tDeviceFactor = ( pMouseEvent.isWheelEvent() ) ? tMouseSensitivities->mWheel
                                                         : -tMouseSensitivities->mMouse;

    // rotate up around norm (by pDeltaMousePosition.x)
    Vec tUp = pCamera.getUp();
    tUp = rotate_vector( pCamera.getNorm(), tRotationFactor * tDeviceFactor * pDeltaMousePosition.x, tUp );
    pCamera.setUp( tUp );

    // move camera along norm vector
    Vec tTranslationVector = pCamera.getNorm() * ( tTranslationFactor * tDeviceFactor * pDeltaMousePosition.y );
    tTranslationVector[3] = 0;

    // adjust camera
    pCamera.translate( tTranslationVector );
}



void SceneMouseHandler::process(
    const MouseEvent &pMouseEvent )
{
    Camera &tCamera = mParentScene->getCamera( mCameraIndex );
    int tKeyboardModifiers = pMouseEvent.getKeyboardModifiers();

    // get mouse mapping
    MouseMappingTableEntry tMouseMappingTableEntry =
        mMouseMappingTable[pMouseEvent.getType()]
                          [pMouseEvent.isLeft()]
                          [pMouseEvent.isMiddle()]
                          [pMouseEvent.isRight()]
                          [pMouseEvent.isModifierActive(ShiftModifier)]
                          [pMouseEvent.isModifierActive(ControlModifier)]
                          [pMouseEvent.isModifierActive(AltModifier)];
    MouseActionCallback tMouseActionCallback = tMouseMappingTableEntry.mMouseAction;
    void* tData = tMouseMappingTableEntry.mData;

    // if dragging is started or keyboard modifiers changed, save anchor point and camera parameters
    if ( ( pMouseEvent.isPressEvent() ) || ( tKeyboardModifiers != mKeyboardModifiersBackup ) )
    {
        mAnchor = pMouseEvent.getRelPos();
        mCameraBackup = tCamera;
    }

    // mouse drag events
    if ( pMouseEvent.isDragEvent() )
    {
        Point32f tDeltaMousePosition = pMouseEvent.getRelPos() - mAnchor;
        tCamera = mCameraBackup;

        if ( tMouseActionCallback )
            tMouseActionCallback( pMouseEvent, pMouseEvent.getRelPos(), tDeltaMousePosition, tCamera, tData );
    }

    // mouse wheel events
    if ( pMouseEvent.isWheelEvent() )
    {
        Point32f tDeltaMousePosition = pMouseEvent.getWheelDelta();

        if ( tMouseActionCallback )
            tMouseActionCallback( pMouseEvent, pMouseEvent.getRelPos(), tDeltaMousePosition, tCamera, tData );
    }

    // save keyboard modifiers for later comparison
    mKeyboardModifiersBackup = tKeyboardModifiers;
}

#endif

}

