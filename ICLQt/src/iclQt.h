#ifndef ICLQT_H
#define ICLQT_H

#include <iclWidget.h>
#include <iclDrawWidget.h>
#include <iclDrawWidget3D.h>
#include <iclGUI.h>
#include <iclMouseInteractionReceiver.h>

#include <QApplication>
#include <iclThread.h>
#include <iclThreadUtils.h>
#include <iclMutex.h>

#include <iclButtonHandle.h>
#include <iclBoxHandle.h>
#include <iclBorderHandle.h>
#include <iclButtonGroupHandle.h>
#include <iclLabelHandle.h>
#include <iclSliderHandle.h>
#include <iclFSliderHandle.h>
#include <iclIntHandle.h>
#include <iclFloatHandle.h>
#include <iclStringHandle.h>
#include <iclComboHandle.h>
#include <iclSpinnerHandle.h>
#include <iclImageHandle.h>
#include <iclDrawHandle.h>
#include <iclDrawHandle3D.h>
#include <iclDispHandle.h>
#include <iclFPSHandle.h>
#include <iclMultiDrawHandle.h>
#include <iclTabHandle.h>
#include <iclSplitterHandle.h>


/** 
    \defgroup COMMON "Most common classes"
    \defgroup HANDLES "GUI Component Handle classes"
    \defgroup UNCOMMON "Uncommon classes (internally used)"

    
    \mainpage ICLQt Package for embedding image visualization Wigets into QWidgets
    This package provides some Widget classes, which allow to visualize ICL images in 
    Qt based applications.\n
    In addition, the new ICL GUI Framework can be used to create GUI based applications
    quickly and conveniently.
    
    \section CF Core Features
    - Hardware acceleration using OpenGL
    - X11 fallback implementation
    - "Self organizing" and "low cost" On Screen Display (OSD) for all visualization components
    - PaintEngine interface for abstracting from the underlying draw engine (OpenGL / X11)
    - Mouse interaction interface for design of custom interactive applications
    - ICL GUI API to create GUI applications 

    \section MOD Modules

    \ref COMMON
    - <b>ICLWidget</b> basic Widget component
    - <b>ICLDrawWidget</b> generalized drawing widget providing a high performance drawing 
      state machine to annotate images in real time applications
    - <b>PaintEngine</b> Paint Engine class interface
    - <b>MouseInteractionReceiver</b> interface class for receiving mouse interaction events
    - <b>QImageConverter</b> Class for converting QImages to ICL images and back with optimized
      conversion routines and memory handling.
    - <b>GUI</b> basic GUI class
    - <b>ChromaGUI</b> dedicated GUI component to ajust cromaticity-space segmentation parameters


    \ref HANDLES
    - <b>GUIHandle</b> Abstract base template class for all other Handle classes.
    - <b>BorderHandle</b> Handle for a container widget with a titled border. The handle can be used to set the
      border title.
    - <b>BoxHandle</b> Handle for container components providing access to the underlying widget and its layout
    - <b>ButtonGroupHandle</b> Handle that wraps a QButtonGroup and which provides some direct access function to
      This group
    - <b>ButtonHandle</b> Handle which wraps a QPushButton. In addition it provides an interface to register callbacks.
    - <b>ComboHandle</b> Handle which wraps a QComboBox.
    - <b>DispHandle</b> Handle providing a convenient access to a matrix of LabelHandles
    - <b>DrawHandle</b> Handle wrapping an embedded ICLDrawWidget 
    - <b>FloatHandle</b> Handle wrapping a QLineEdit which accepts only float values
    - <b>FSliderHandle</b> Handle wrapping a QSlider which values are translated into float values
    - <b>ImageHandle</b> Handle wrapping an embedded ICLWidget component
    - <b>IntHandle</b> Handle wrapping a QLineEdit which accepts only integer values
    - <b>LabelHandle</b> Handle wrapping a QLable to display integers, floats and strings
    - <b>SliderHandle</b> Handle wrapping a QSlider component
    - <b>SpinnerHandle</b> Handle wrapping a QSpinner component
    - <b>StringHandle</b> Handle wrapping a QLineEdit which accepts any input
    
    For developers: \ref UNCOMMON 

    
    
*/

#endif
