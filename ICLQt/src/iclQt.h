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
    setting different visualisation parameters (explained in the icl::ICLWidget documentation)  as well as 
    an embedded user interface for interactive adaption of these parameters. Furthermore, the ICLWidgets 
    provides a function <tt>setImage(ImgBase*)</tt> to make it show a new image. Internally, the image
    is buffered into a mutex-protected interleaved intermediate format, which can more easily be transferred
    to the graphics buffer. By this means, <tt>setImage</tt> can simply be called from the applications
    working thread without any explicit synchronization. Now, the ICLWidget instances can be setup to
    visualize this new image by calling <tt>updateFromOtherThread()</tt> (please note, that the QWidget's
    <tt>update()</tt> function is not threadsafe and therefore it must not be called from outside Qt's
    GUI-thread). Calls to <tt>updateFromOtherThread</tt> are translated into Qt-Events, which are processed
    by the current QApplication asynchronously. 

    For image annotation, the icl::ICLDrawWidget is provided. It works like a drawing state-machine
    that automatically synchronized image annotations with Qt's event loop. Internally, this is achieved
    by using a mutex protected draw-command queue, which can be filled from the working thread.
    
    On the last level, the icl::ICLDrawWidget3D provides an interfaces for drawing OpenGL-based 3D
    primitives as overlay onto an image;
    

    

    </TD><TD valign="top">
    \image html drawing-layers.png "Collaboration in ICL's visualisation and annotation framework"
    </TD></TR></TABLE>
    
    \section OSM The ICLWidget's On-Screen-Menu
    
    <TABLE border=0><TR><TD>
    
    The icl::ICLWidget's on-screen-menu provides a lot of basic features to adjust image
    visualisation parameters, such as brightness-contrast adjustment, or zooming to a certain
    sub-rectangle of the whole image. Furthermore, it provides basic functionalities for
    capturing images, to select single image channels and to show an online updated image
    histogram.\n
    The on-screen-menu can be activated by the user by mouse. The menu control buttons appear
    automatically when the mouse is hovered above the widget and disappear automatically when
    the mouse leaves the widget screen area. In cases where the on-screen-menu isn't needed
    the automatic appearing of the widget control buttons can be deactivated.
    
    The leftmost button shows or hides the menu. By default, the menu is embedded into the 
    widget. The 2nd button detaches the menu from the widget and makes the menu an independent
    window. The next button that shows two diagonally aligned filled squares changes
    the internal image interpolation mode. By default, the image texture is painted using
    nearest neighbour interpolation, which means that image pixels have hard edges in
    zoomed view. If the interpolation mode button is toggled a linear interpolation
    is used for texture rendering.
    The iterative zoom button, which is left to the image interpolation button, can be
    used to drag a zooming area. If the image zoom is active, the image zoom button is
    tainted red. Another click on the image zoom button disabled the image zoom to re-activated
    the original mode.
    The rightmost button is only visible if the widget's content is currently recorded
    from the widgets menu. It can be used as a shotcut to stop recording even if the
    menu is currently not visible. A detailed explanation of the on-screen-menu itself
    is provided in the icl::ICLWidget's documentation section \ref ICLWIDGET_OSM.
    
    </TD><TD>
    \image html osm-1.png "Widget control buttons"
    </TD></TR></TABLE>

    \section MOD Modules

    - \ref COMMON
    - \ref HANDLES
    - (for ICL-developers) \ref UNCOMMON
    
*/

#endif
