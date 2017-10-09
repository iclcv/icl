/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLGeom/src/ICLGeom/GridSceneObject.h                  **
** Module : ICLGeom                                                **
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

#ifndef ICL_HAVE_OPENGL
#if WIN32
#pragma WARNING("this header must not be included if ICL_HAVE_OPENGL is not defined")
#else
#warning "this header must not be included if ICL_HAVE_OPENGL is not defined"
#endif
#else

#include <ICLUtils/CompatMacros.h>
#include <ICLGeom/SceneObject.h>


namespace icl{
  namespace geom{

    /// SceneObject specialization for grid like objects
    /** The GridSceneObject implements a 2D grid in 3D space. The grid consits of
        2D array of normal vectors. All attributes remain default SceneObject attributs.
        The grid cells are created as quads. */
    class ICLGeom_API GridSceneObject : public SceneObject{
      int nXCells; //!< grid width
      int nYCells; //!< grid height

      /// internal initialization method
      void init(int nXCells, int nYCells, const std::vector<Vec> &allGridPoints, bool lines, bool quads) throw (utils::ICLException);

      public:
      /// creates a GridSceneObject with a given set of vertices
      /** Note: allGridPoints must have nXCells + nYCells elements and its elements must
          be ordered in row-major order */
      GridSceneObject(int nXCells, int nYCells, const std::vector<Vec> &allGridPoints,
                      bool lines=true, bool quads=true) throw (utils::ICLException);

      /// creates a GridSceneObject where the grid is a regular grid in 3D space
      /** the grid nodes are created automatically: node(x,y) = origin + x*dx + y*dy */
      GridSceneObject(int nXCells, int nYCells, const Vec &origin, const Vec &dx, const Vec &dy,
                      bool lines=true, bool quads=true)throw (utils::ICLException);

      /// returns the node index of given node x/y position
      inline int getIdx(int x, int y) const { return x+nXCells*y; }

      /// returns node at given x/y grid position
      Vec &getNode(int x, int y){ return m_vertices[getIdx(x,y)]; }

      /// returns node at given x/y grid position (const)
      const Vec &getNode(int x, int y) const { return m_vertices[getIdx(x,y)]; }

      /// returns node at given  grid position
      Vec &getNode(const utils::Point &p) { return getNode(p.x,p.y); }

      /// returns node at given  grid position (const)
      const Vec &getNode(const utils::Point &p) const { return getNode(p.x,p.y); }

      /// returns grid dimension (nXCells * nYCells)
      inline utils::Size getSize() const { return utils::Size(nXCells,nYCells); }

      /// returns grid width
      inline int getWidth() const { return nXCells; }

      /// returns grid height
      inline int getHeight() const { return nYCells; }
    };


  } // namespace geom
}

#endif
