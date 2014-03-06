/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLQt/src/ICLQt/DefineQuadrangleMouseHandler.h         **
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
#include <ICLUtils/Uncopyable.h>
#include <ICLUtils/VisualizationDescription.h>
#include <ICLQt/MouseHandler.h>

namespace icl{
  namespace qt{
    /// Special MouseHandler Implementation that allows for defining a quadrangle
    /** The resulting quadrangle (defined by it's four corners points) can be
        restricted to be always convex. The quadrangle is initialized as a rectangle
        that is 20 px smaller than the given max-size rectangle.
        */
    class ICLQt_API DefineQuadrangleMouseHandler : public qt::MouseHandler,
          public utils::Uncopyable, public utils::Lockable{
      struct Data;  //!< pimpl type
      Data *m_data; //!< pimpl pointer
     
      public:

      /// empty constructor (creates a null handler)
      DefineQuadrangleMouseHandler();

      /// constructor with given max size
      /** The maxSize must be identical to the image the mousehandler is used
          as a overlay with. Convex means that the quadrangle is convex and that
          its opposing edges do not intersect */
      DefineQuadrangleMouseHandler(const utils::Size &maxSize, bool convexOnly=true);
      
      /// MouseHandler interface
      virtual void process(const qt::MouseEvent &e);
      
      /// destructor
      virtual ~DefineQuadrangleMouseHandler();

      /// deferred intialization method
      void init(const utils::Size &maxSize, bool convexOnly=true);
      
      /// sets an offset that shifts both in- and outputs
      void setOffset(const utils::Point &p);

      /// sets the size of the handles
      void setHandleSize(float size);
      
      /// returns the current quadrangle
      std::vector<utils::Point> getQuadrangle() const;
      
      /// sets the current quadrangle
      /** If the given quadrangle is not completely covered by the 
          image rect, or the convexOnly flag is activated and the
          given quadrangle is not convex, an exception is thrown*/
      void setQuadrangle(const utils::Point ps[4]) throw (utils::ICLException);
      
      /// returns whether this mousehandler has been initialized yet
      inline bool isNull() const { return !m_data; }

      /// returns a visualization 
      utils::VisualizationDescription vis() const;

    };
  }
}
