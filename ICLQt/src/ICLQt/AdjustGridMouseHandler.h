/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLQt/src/ICLQt/DefineRectanglesMouseHandler.h         **
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

#include <ICLUtils/Lockable.h>
#include <ICLUtils/Uncopyable.h>
#include <ICLUtils/VisualizationDescription.h>
#include <ICLQt/MouseHandler.h>
#include <ICLCore/Line32f.h>
#include <vector>

namespace icl{
  namespace qt{

    /// Special MouseHandler Implementation that allows several quadrangular grids to be defined
    class AdjustGridMouseHandler : public MouseHandler, public utils::Uncopyable, public utils::Lockable{
      struct Data;  //!< pimpl type
      Data *m_data; //!< pimpl pointer
    
      public:
    
      /// empty constructor (creates a null handler)
      AdjustGridMouseHandler();
    
      /// constructor with given max size
      /** The maxSize must be identical to the image the mousehandler is used
          as a overlay with. Convex means that the quadrangle is convex and that
          its opposing edges do not intersect */
      AdjustGridMouseHandler(const utils::Rect &bounds, bool convexOnly=true);
    
      /// MouseHandler interface
      virtual void process(const MouseEvent &e);
    
      /// destructor
      virtual ~AdjustGridMouseHandler();
    
      /// deferred intialization method
      void init(const utils::Rect &bounds, bool convexOnly=true);

      void init(const utils::Rect &bounds,
                const std::vector<std::vector<utils::Point> > &grids,
                bool convexOnly=true);

      void clear();
    
      /// sets the size of the handles
      void setHandleSize(float size);

      /// returns number of internal grids
      size_t getNumGrids() const;

      /// returns the internal bounds rectangle
      const utils::Rect &getBounds() const;
    
      void defineGridTexture(size_t idx, const utils::Size32f &dim,
                             const std::vector<core::Line32f> &lines);

      std::vector<utils::Point32f> mapPoints(size_t idx,
                                             const std::vector<utils::Point32f> &ps) const;
    
      /// returns the current quadrangle
      std::vector<utils::Point> getGrid(size_t idx) const throw (utils::ICLException);
    
      /// sets the current quadrangle
      /** If the given quadrangle is not completely covered by the 
          image rect, or the convexOnly flag is activated and the
          given quadrangle is not convex, an exception is thrown*/
      void setGrid(size_t idx, const utils::Point ps[4]) throw (utils::ICLException);
    
      /// returns whether this mousehandler has been initialized yet
      inline bool isNull() const { return !m_data; }
    
      /// returns a visualization 
      utils::VisualizationDescription vis() const;
    };
  }
}
