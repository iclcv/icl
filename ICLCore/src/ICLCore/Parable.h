/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLCore/src/ICLCore/Parable.h                          **
** Module : ICLCore                                                **
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
#include <ICLUtils/Point32f.h>
#include <stdio.h>

namespace icl{
  namespace core{

    /// Utility class for the parable-based chromaticity segmentation \ingroup UNCOMMON
    /** A parabolic function is defined by 3 parameters a,b and c:
        \f[
        f(x) = ax^2 + bx + c
        \f]
        Parables can be created by given parameters a,b and c
        or by 3 given points.
    */
    struct Parable{

      /// create an empty parable (a=b=c=0)
      Parable():a(0),b(0),c(0){}

      /// create a parable with given parameters a,b and c
      Parable(float a, float b,float c):a(a),b(b),c(c){}

      /// create a parable with given 3 points
      /** To avoid numerical problems, the x coordinates of the given points
          are increased by a small value (0.00000001 - 0.00000003). This
          is harmless, as the Parable struct is used for the ChromaWidget, where
          given input point are defined by relative widget coordination which
          much less resolution

      **/
      Parable(utils::Point32f p1,utils::Point32f p2,utils::Point32f p3){
        p1.x+=0.00000001; // avoid numerical problems
        p2.x+=0.00000002;
        p3.x+=0.00000003;

        float xx1 = p1.x * p1.x;
        float xx2 = p2.x * p2.x;
        float xx3 = p3.x * p3.x;
        b = ((p3.y-p1.y)/(xx3-xx1)-(p2.y-p1.y)/(xx2-xx1)) /
        ((p1.x-p2.x)/(xx2-xx1)-(p1.x-p3.x)/(xx3-xx1));

        a = (b*(p1.x-p2.x)+(p2.y-p1.y)) / (xx2-xx1);

        c = p1.y-a*xx1-b*p1.x;
      }

      float a; /**!< first parameter a */
      float b; /**!< second parameter a */
      float c; /**!< third parameter a */

      /// Evaluate this parable at a given location x
      float operator()(float x) const{ return a*x*x+b*x+c; }

      /// shows this parable to std::out
      void show()const{
        printf("Parable:\n");
        printf("f(x)=%f*x²+%fx+%f\n",a,b,c);
      }
    };

  } // namespace core
}

