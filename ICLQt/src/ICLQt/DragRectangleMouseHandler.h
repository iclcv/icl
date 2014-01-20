/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLQt/src/ICLQt/DragRectangleMouseHandler.h            **
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

#include <ICLUtils/CompatMacros.h>
#include <ICLUtils/Lockable.h>
#include <ICLUtils/Rect.h>
#include <ICLCore/Color.h>
#include <ICLQt/MouseHandler.h>

namespace icl{
  namespace qt{
  
    /** \cond */
    class ICLDrawWidget;
    /** \endcond */
  
    /// Special MouseHandler implementation that allows for dragging a Rectangle
    /** This mousehandler can be installed on an instance of ICLWidget. This widget
        then enables the user to use a mouse-based drag gesture to define a
        rectangle on top of the current image. The DragRectangleMouseHandler's
        visualize-method can be used to show the current result.
        The rectangle can be defined by holding the left mouse button. The right
        mouse button deletes the currently define rectangle.
        There are two Rectangles, that can be accessed from outside:
        the 'dragged-rect' is the rectangle, that is currently dragged. It can be
        accessed with hasDraggedRect and getDraggedRect. Once a rectangle is defined,
        it can be accessed with hasRect and hasDraggedRect. 
  
        \section COL Colors
        For each of these two rectangles, three different colors can be set.
        The edge color of the rectangle, the fill color of the rectangle and
        the color that is used to fill everything else then the rectangle. Each
        of these colors can have an alpha component of zero; in this case, the
        component is not drawn.
        Since setting these colors is not crucial for threading, public access
        to the color memerber is granted.
  
        \section MD Minimal Rectangle Dimension
        In order to avoid to capture simple mouse-clicks, that lead
        to very small or even zero-sized rectangles, a minimum 
        rectangle dimension can be given in the constructor. The
        minimum rectangle dimension is only used for the 'real' 
        rectangle, not for the dragged one.
    */
    class ICLQt_API DragRectangleMouseHandler : public MouseHandler, public utils::Lockable{
      protected:
      utils::Point m_origin; //!< point where the drag gesture started
      utils::Point m_curr;   //!< current mouse-position while dragging
      int m_minDim;   //!< minimum rectangle size
      utils::Rect m_rect;    //!< last defined rectangle
  
      public:
      core::Color4D m_edge; //!< edge color for the defined rectangle
      core::Color4D m_fill; //!< fill color for the defined rectangle
      core::Color4D m_outer;//!< color that is used for everything outside the rectangle
  
      core::Color4D m_edgeWhileDrag; //!< edge color for the dragged rectangle
      core::Color4D m_fillWhileDrag; //!< fill color for the dragged rectangle
      core::Color4D m_outerWhileDrag;//!< as m_outer, but for the dragged rectangle
  
      /// Default constructor with optionally given minimum dimension
      /** \see \ref MD */
      DragRectangleMouseHandler(int minDim=4);
      
      /// overwritten mouse-event handler function
      virtual void process(const MouseEvent &e);
      
      /// visualization method that can be used to visualized the MouseHandlers current state
      /** This method needs an already locked instance of ICLDrawWidget */
      void visualize(ICLDrawWidget &w);
      
      /// returns wether a valid rectangle was defined
      bool hasRect() const;
  
      /// returns the currrent rectangle
      utils::Rect getRect() const;
  
      /// returns whether the user currently defines a rectangle (using a drag gesture)
      bool hasDraggedRect() const;
  
      /// returns the currently dragged rectangle
      utils::Rect getDragggedRect() const;
  
      /// sets the minimum rectangle size
      inline void setMinDim(int minDim) { m_minDim = minDim; }
  
      /// returns the minimum rectangle size
      inline int getMinDim() const{ return m_minDim; }
    };
  } // namespace qt
}

