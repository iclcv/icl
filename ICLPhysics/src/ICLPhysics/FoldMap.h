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
