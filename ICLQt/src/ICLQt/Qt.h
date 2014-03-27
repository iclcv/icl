/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLQt/src/ICLQt/Qt.h                                   **
** Module : ICLQt                                                  **
** Authors: Christof Elbrechter                                    **
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

#include <ICLQt/Widget.h>
#include <ICLQt/DrawWidget.h>
#include <ICLQt/DrawWidget3D.h>
#include <ICLQt/GUI.h>
#include <ICLQt/MouseHandler.h>

#include <QApplication>
#include <ICLUtils/Thread.h>
#include <ICLUtils/Mutex.h>

#include <ICLQt/ButtonHandle.h>
#include <ICLQt/BoxHandle.h>
#include <ICLQt/BorderHandle.h>
#include <ICLQt/ButtonGroupHandle.h>
#include <ICLQt/LabelHandle.h>
#include <ICLQt/SliderHandle.h>
#include <ICLQt/FSliderHandle.h>
#include <ICLQt/IntHandle.h>
#include <ICLQt/FPSHandle.h>
#include <ICLQt/CheckBoxHandle.h>
#include <ICLQt/FloatHandle.h>
#include <ICLQt/StateHandle.h>
#include <ICLQt/StringHandle.h>
#include <ICLQt/ComboHandle.h>
#include <ICLQt/SpinnerHandle.h>
#include <ICLQt/ImageHandle.h>
#include <ICLQt/DrawHandle.h>
#include <ICLQt/DrawHandle3D.h>
#include <ICLQt/DispHandle.h>
#include <ICLQt/FPSHandle.h>
#include <ICLQt/MultiDrawHandle.h>
#include <ICLQt/TabHandle.h>
#include <ICLQt/SplitterHandle.h>
#include <ICLQt/PlotHandle.h>
#include <ICLQt/ContainerGUIComponents.h>

/** 
    \defgroup COMMON "Most common classes"
    \defgroup HANDLES "GUI Component Handle classes"
    \defgroup UNCOMMON "Uncommon classes (internally used)"
    
    As it's core, this package provides a wrapper API for Qt-based GUI components. The main class of this
    package is the icl::GUI class. Further essential components are located in
    the \ref IVF .

    \section _GENERAL_ General Idea of this Package
    When developing computer vision applications, one does usually not want to fight with the common GUI
    issues -- such as layouting GUI components and synchronizing GUI events and values with the applications
    working thread. To avoid theses issues, ICL comes up with a string- and stream-based GUI creation framework,
    that allows for creating most complex GUIs with just a few lines of well readable code.
    However it always provides full access to the underlying Qt-components to enable the user to use 
    <em>traditional</em> Qt-code to implement custom stuff where it is needed. 

    The GUI-framework provides some special components, such as the image visualization and annotation widgets (\ref IVF)
    and the icl::PlotWidget for 2D data visualization. In addition all common Qt-components, such as sliders and buttons,
    are also wrapped.

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
    - How can 3D objects be drawn into a scene so that it matches it's real-world counter part?
    - Can 3D overlay be implemented using OpenGL?
    - ...

    <TABLE border=0><TR><TD>
    ICL's image visualization and annotation framework consists essentially of the three widget classes 
    - icl::ICLWidget
    - icl::ICLDrawWidget 
    - icl::ICLDrawWidget3D.

    At the lowest layer, the icl::GLImg provides an interfaces for converting ImgBase
    instances into an OpenGL texture (if the image is larger than OpenGL's maximum texture sizes, it
    has to be split into several texture) that can be drawn arbitrarily into an OpenGL 
    scene. Internally, the icl::GLImg class is used for different image depths
    depths. Here, OpenGL's pixel-transfer parameters are used for hardware accelerated brightness and 
    contrast adjustment. Furthermore, fitting images into the widget viewport can simply be performed 
    by the graphics hardware. The GLImg can also be used as efficient video texture.
    
    The next layer is implemented by the icl::ICLWidget class, which inherits Qt's QGLWidget class for
    creation of an embedded OpenGL context and viewport. The ICLWidget provides a software interface for 
    setting different visualisation parameters (explained in the icl::ICLWidget documentation)  as well as 
    an embedded user interface for interactive adaption of these parameters. Furthermore, the ICLWidgets 
    provides a function <tt>setImage(core::ImgBase*)</tt> to make it show a new image. Internally, the image
    is buffered into a mutex-protected interleaved intermediate core::format, which can more easily be transferred
    to the graphics buffer. By these means, <tt>setImage</tt> can simply be called from the application's
    working thread without any explicit synchronization. Once an new image is given, the icl::ICLWidget will
    automatically post a Qt-update event by calling the ICLWidget::render() method. In this way, the used OpenGL
    context is actually re-rendered asynchronously in the application's GUI thread.

    For image annotation (such as rendering boxes or symbols on top of the image to visualize image processing
    results), the icl::ICLDrawWidget is provided. It works like a drawing state-machine
    that automatically synchronized image annotations with Qt's event loop. Internally, this is achieved
    by using two thread-safe draw-command queues. One of these queues can be filled with new draw commands,
    while the other queue belongs to the GUI thread and is rendered. Every time, the parent icl::ICLWidget classes
    icl::ICLWidget::render() method is called, the queues are swapped, and the queue that is now being filled
    with new commands is automatically cleared. Here, the icl::ICLDrawWidget adapts the behavior of the parent
    icl::ICLWidget class. <b>The icl::ICLDrawWidget will not automatically call ICLWidget::render() when
    a new background image is given</b>. Since usually setting the background image is followed by posting
    a set of draw-commands, icl::ICLWidget::render() has to be called manually, once the image annotation is
    finished. (TODO add reference to example)
    
    On the last level, the icl::ICLDrawWidget3D, which itself extends the icl::ICLDrawWidget, provides an interfaces
    for rendering 3D stuff on top of an image. The icl::ICLDrawWidget3D provides the icl::ICLDrawWidget3D::link method,
    which links a simple callback function to it. When the icl::ICLDrawWidget3D is rendered, it will also execute
    the linked OpenGL callback function in the GUI Thread, while still being able to render 2D annotations. <b>
    It is highly recommended to use the icl::Scene class to create 3D rendering overlays</b> The scene class can easily
    provide an appropriate OpenGL callback function for it's contained cameras.
        
    Usage examples are given on <a href="http://www.iclcv.org">ICL's webpage</a>. Select info->tutorial in the top-menu.

    </TD><TD valign="top">
    \image html drawing-layers.png "Collaboration in ICL's visualisation and annotation framework"
    </TD></TR></TABLE>
    
    \section OSM The ICLWidget's On-Screen-Menu
    
    <TABLE border=0><TR><TD>
    
    <b>note: this section is not up-to-date</b>\n
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
    is provided in the icl::ICLWidget's documentation.
    
    </TD><TD>
    \image html osm-1.png "Widget control buttons"
    </TD></TR></TABLE>

    \section MOD Modules

    - \ref COMMON
    - \ref HANDLES
    - (for ICL-developers) \ref UNCOMMON
    
*/

