// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

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
	  class ICLQt_API AdjustGridMouseHandler : public MouseHandler, public utils::Lockable{
      struct Data;  //!< pimpl type
      Data *m_data; //!< pimpl pointer

      public:
      AdjustGridMouseHandler(const AdjustGridMouseHandler&) = delete;
      AdjustGridMouseHandler& operator=(const AdjustGridMouseHandler&) = delete;


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
      std::vector<utils::Point> getGrid(size_t idx) const;

      /// sets the current quadrangle
      /** If the given quadrangle is not completely covered by the
          image rect, or the convexOnly flag is activated and the
          given quadrangle is not convex, an exception is thrown*/
      void setGrid(size_t idx, const utils::Point ps[4]);

      /// returns whether this mousehandler has been initialized yet
      inline bool isNull() const { return !m_data; }

      /// returns a visualization
      utils::VisualizationDescription vis() const;
    };
  }
}
