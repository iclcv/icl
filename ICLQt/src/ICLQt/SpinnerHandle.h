/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLQt/src/ICLQt/SpinnerHandle.h                        **
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

