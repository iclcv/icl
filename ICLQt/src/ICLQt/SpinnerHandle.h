// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include <ICLUtils/CompatMacros.h>
#include <ICLQt/GUIHandle.h>
#include <QtWidgets/QSpinBox>

namespace icl{
  namespace qt{

    /// Handle class for spinner components \ingroup HANDLES
    class ICLQt_API SpinnerHandle : public GUIHandle<QSpinBox>{

      public:

      /// Create an empty spinner handle
      SpinnerHandle(){}

      /// create a new SpinnerHandle with given QSpinBox* to wrap
      SpinnerHandle(QSpinBox *sb, GUIWidget *w) : GUIHandle<QSpinBox>(sb,w){}

      /// set the min value
      void setMin(int min);

      /// set the max value
      void setMax(int max);

      /// set the range of the spin-box
      void setRange(int min, int max) { setMin(min); setMax(max); }

      /// set the current value of the spin-box
      void setValue(int val);

      /// sets all parameters of a spin-box
      void setAll(int min ,int max, int val){ setRange(min,max); setValue(val); }

      /// returns the current min. of the spin-box
      int getMin() const;

      /// returns the current max. of the spin-box
      int getMax() const;

      /// returns the current value of the spin-box
      int getValue() const;

      /// assigns a new value to the spin-box (equal to setValue)
      void operator=(int val) { setValue(val); }

      private:
      /// internally used utility function
      QSpinBox *sb() { return **this; }

      /// internally used utility function
      const QSpinBox *sb() const{ return **this; }
    };
  } // namespace qt
}
