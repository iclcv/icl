#ifndef ICLQT_H
#define ICLQT_H

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
    
    \section commonClasses Common classes
    - <b>ICLWidget</b> basic Widget component
    - <b>ICLDrawWidget</b> generalized drawing widget providing a high performance drawing 
      state machine to annotate images in real time applications
    - <b>PaintEngine</b> Paint Engine class interface
    - <b>MouseInteractionReceiver</b> interface class for receiving mouse interaction events
    - <b>QImageConverter</b> Class for converting QImages to ICL images and back with optimized
      conversion routines and memory handling.
    - <b>GUI</b> basic GUI class

    \section HandleClasses GUI Handle classes
    - <b>jfdsjf</b>
    
    <b>\ref COMMON</b> 

    <b>\ref HANDLES</b> 
    
    \ref UNCOMMON 

    
    
*/

#endif
