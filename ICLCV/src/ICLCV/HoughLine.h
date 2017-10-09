/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLCV/src/ICLCV/HoughLine.h                            **
** Module : ICLCV                                                  **
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

#include <ICLUtils/Point32f.h>
#include <ICLCore/ImgBase.h>
#include <vector>

namespace icl{
  namespace cv{
    /// Line Equation structure
    /** Lines can (among others) be represented in parameter form:
        \f[ l: \vec{o}+\lambda \vec{d} \f]

        ,or by given radius \f$r\f$ and angle \f$\alpha \f$.
        \f[ x \cos(\alpha) + y \sin(\alpha) - r = 0 \f]

        This class provides both information.
     */
    struct ICLCV_API HoughLine{
      /// empty constructor (all line values are set to 0)
      HoughLine();

      /// create a line by given parameters
      /** @param offs offset vector
          @param dir direction vector
      */
      HoughLine(const utils::Point32f &offs, const utils::Point32f &dir);

      /// create a line by given distance and angle
      /** @param distance distance from the origin
          @param angle angle of vector perpendicular to the line */
      HoughLine(icl32f distance, icl32f angle);

      /// draws the line into an image
      void sample(core::ImgBase *image, icl32f r, icl32f g, icl32f b, icl32f alpha=255) const;

      /// Computes the intersection between two lines
      /** Assuming two lines given in polar coordinates
          \f[ x \cos(\alpha) + y \sin(\alpha) - r = 0 \f],

          which can be seen geometrically, as a line that is perpendicular
          to the vector \f$ (\cos(\alpha), \sin(\alpha) )^{\tau} \f$
          with distance r to the origin, we can calculate the intersection as
          follows:

          Firstly, we have to transform the line equations into parameter form
          \f[ l: \vec{o}+\lambda \vec{d} \f] (offset- and direction vector) using the
          following rules

          \f[ \vec{o} = r * ( \cos(\alpha) \sin(\alpha) )^{\tau} \f]
          \f[ \vec{d} = ( -\sin(\alpha) \cos(\alpha) ]^{\tau} \f]

          This results in:
          \f[ g_1: \vec{o}_1+ \lambda_1 * \vec{d}_1 \f] (using angle \f$\alpha\f$ and distance \f$r\f$) and
          \f[ g_2: \vec{o}_2+ \lambda_2 * \vec{d}_2 \f] (using angle \f$\beta\f$ and distance \f$s\f$) and

          The intersection can be calculated by solving the equation system

          \f[ g1 = g2 \f], or in other words:

          \f[ \lambda_1\cdot -\sin(\alpha) + \lambda_2 \cdot  \sin(\beta)  = s\cos(\beta) - r\cos(\alpha) \f]
          \f[ \lambda_1\cdot  \cos(\alpha) + \lambda_2 \cdot -\cos(\beta)  = s\sin(\beta) - r\sin(\alpha) \f]

          This is equal to \f[ Mx = b \f]

          With
          \f[
          M = \left(
          \begin{array}{cc}
          -\sin(\alpha) & \sin(\beta) \\
          \cos(\alpha) &  -\cos(\beta) \\
          \end{array} \right)
          \f]

          \f[
          B = \left(
          \begin{array}{c}
          s\cos(\beta) - r\cos(\alpha) \\
          s\sin(\beta) - r\sin(\alpha) \\
          \end{array} \right)
          \f]

          and
          \f[ x = (\lambda_1, \lambda_2)^{\tau} \f]

          Of course \f$ Mx=b \f$ can be solved by \f$x=M^{-1}b\f$

          If the lines do not intersect (\f$\alpha\f$ and \f$\beta\f$ are very similar),
          \f$M\f$ becomes singular and it's inverse cannot be calculated. This function
          returns NO_INTERSECTION in this case.
      */
      static utils::Point32f getIntersection(const HoughLine &a, const HoughLine &b);

      /// calculates all pairwise intersection within a set of lines
      static std::vector<utils::Point32f> getPairwiseIntersections(const std::vector<HoughLine> &lines);

      /// indicator for no found intersection (lines intersect in infinity -- max-float)
      static const utils::Point32f NO_INTERSECTION;

      /// convenence function for transf. into IppPointPolar
      float rho() const { return m_distance; }

      /// convenence function for transf. into IppPointPolar
      float theta() const { return m_angle; }

    private:
      float m_distance;
      float m_angle;
      utils::Point32f m_offset;
      utils::Point32f m_direction;
    };
  } // namespace cv
}

