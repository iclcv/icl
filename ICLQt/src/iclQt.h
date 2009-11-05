#ifndef ICL_QT_H
#define ICL_QT_H

#include <iclQtMacros.h>

/** 
    \defgroup COMMON "Most common classes"
    \defgroup HANDLES "GUI Component Handle classes"
    \defgroup UNCOMMON "Uncommon classes (internally used)"
    
    \mainpage ICLQt For Rapid Development of GUI-based Computer-Vision Applications
    
    As it's core, this package provides a wrapper API for Qt-based GUI components. The main class of this
    package is the icl::GUI class (documentation: \ref GUI_INTRO) . Further essential components are part of the \ref IVF .

    \section IVF Image Visualisation and Annotation Framework
    
    Development of a QWidget implementation for image visualisation implies finding answers to the following questions:
    - How can images be visualized at all?
    - How can this be done efficiently?
    - How can different image depths be handled (and how can this be done efficiently)?
    - How must images be scaled and moved to fit optimally into a given widget?
    - How can image processing and image visualisation be decoupled to avoid that the GUI gets stuck
      if the processing loop needs a lot of time for each cycle?
    - How can different threads for processing and visualisation be synchronized?
    - How can the user change image visualisation parameters (e.g, brightness or contrast adjustment)?
    - How can images be annotated in online applications (here again, one has to face synchronization issues)?
    - How can the image annotation be abstracted from visualisation features (e.g., current zoom)?
    - How can 3D objects be drawn into a scene (using OpenGL)?
    - ...

    <TABLE border=0><TR><TD>
    ICL's image visualization and annotation framework consists essentially of the three widget classes 
    - icl::ICLWidget
    - icl::ICLDrawWidget 
    - icl::ICLDrawWidget3D.

    At the lowest layer, the icl::GLTextureMapBaseImage provides an interfaces for converting ImgBase
    instances into a grid of squared OpenGL textures, that can be drawn arbitrarily into an OpenGL 
    scene. Internally, the icl::GLTextureMapImage template class is used for different image depths
    depths. Here, OpenGL's pixel-transfer parameters are used for hardware accelerated brightness and 
    contrast adjustment. Furthermore, fitting images into the widget viewport can simply be performed 
    by the graphics hardware. The GLTextureMapBaseImage can also be used as video texture.
    
    The next layer is implemented by the icl::ICLWidget class, which inherits Qt's QGLWidget class for
    creation of an embedded OpenGL context and viewport. The ICLWidget provides a software interface for 
    setting different visualisation parameters (explained as well as a embedded user interface.

 

    </TD><TD>
    \image html drawing-layers.png "Collaboration in ICL's visualisation and annotation framework"
    </TD></TR></TABLE>
    

    
    \section CF Core Features
    - Hardware acceleration using OpenGL
    - X11 fallback implementation
    - "Self organizing" and "low cost" On Screen Display (OSD) for all visualization components
    - PaintEngine interface for abstracting from the underlying draw engine (OpenGL / X11)
    - Mouse interaction interface for design of custom interactive applications
    - ICL GUI API to create GUI applications 

    \section MOD Modules

    - \ref COMMON
    - \ref HANDLES
    - \ref UNCOMMON
    
    
    For developers: \ref UNCOMMON 

    
    
*/

#endif
