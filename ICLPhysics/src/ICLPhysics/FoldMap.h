/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2014 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLPhysics/src/ICLPhysics/FoldMap.h                    **
** Module : ICLPhysics                                             **
** Author : Christof Elbrechter, Matthias Esau                     **
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

#include <ICLCore/Img.h>

namespace icl{
  namespace physics{


    /// discretized paper space representing folds
    /** Each pixel contains a stiffness value of the paper at that point.
        For each link that is created on the paper, the minimum intersected
        stiffness value is used. By these means, all constraints, that intersect
        fold lines are weak.

        For memorization, an extra mechanism was implemented. Memorized links
        (of first-order-type) will have a negative stiffness entry in the fold map.

        A bending constraint gets a either a  potitive stiffness value or, if
        all stiffness values were 1 (i.e. no non-memorized fold was intersected),
        but it crosses a memorized (<0) link, it gets the memorization property.
    */
    class ICLPhysics_API FoldMap{
      core::Img32f m;
      float initialValue;
      void draw_fold(const utils::Point32f &a, const utils::Point32f &b, float val);

      public:
      FoldMap(const utils::Size &resolution=utils::Size(200,300),float intialValue=1);

      void clear();

      /// memorized folds are nagative
      void addFold(const utils::Point32f &a, const utils::Point32f &b, float value);

      /// sets fold pixels to 1
      void removeFold(const utils::Point32f &a, const utils::Point32f &b);

      /// return a value that can be used as fold stiffness
      /** preferres non-1 positive values in the fold map
          Only if the minimum of these is 1, the maximum negative value (closest to 0)
          is returned

      */
      float getFoldValue(const utils::Point32f &a, const utils::Point32f &b);

      /// current fold map
      const core::Img32f &getImage() const { return m; }
    };
  }
}
