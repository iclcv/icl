// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include <ICLUtils/CompatMacros.h>
#include <ICLUtils/Lockable.h>
#include <ICLUtils/Uncopyable.h>
#include <ICLUtils/VisualizationDescription.h>
#include <ICLQt/MouseHandler.h>

namespace icl::qt {
    /// Special MouseHandler Implementation that allows for defining a quadrangle
    /** The resulting quadrangle (defined by it's four corners points) can be
        restricted to be always convex. The quadrangle is initialized as a rectangle
        that is 20 px smaller than the given max-size rectangle.
        */
    class ICLQt_API DefineQuadrangleMouseHandler : public qt::MouseHandler,
          public utils::Lockable{
      struct Data;  //!< pimpl type
      Data *m_data; //!< pimpl pointer

      public:
      DefineQuadrangleMouseHandler(const DefineQuadrangleMouseHandler&) = delete;
      DefineQuadrangleMouseHandler& operator=(const DefineQuadrangleMouseHandler&) = delete;


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
      void setQuadrangle(const utils::Point ps[4]);

      /// returns whether this mousehandler has been initialized yet
      inline bool isNull() const { return !m_data; }

      /// returns a visualization
      utils::VisualizationDescription vis() const;

    };
  }