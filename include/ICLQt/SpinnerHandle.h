/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2010 Neuroinformatics, CITEC                 **
**                         University of Bielefeld                 **
**                Contact: nivision@techfak.uni-bielefeld.de       **
**                Website: www.iclcv.org                           **
**                                                                 **
** File   : include/ICLQt/SpinnerHandle.h                          **
** Module : ICLQt                                                  **
** Authors: Christof Elbrechter                                    **
**                                                                 **
**                                                                 **
** Commercial License                                              **
** ICL can be used commercially, please refer to our website       **
** www.iclcv.org for more details.                                 **
**                                                                 **
** GNU General Public License Usage                                **
** Alternatively, this file may be used under the terms of the     **
** GNU General Public License version 3.0 as published by the      **
** Free Software Foundation and appearing in the file LICENSE.GPL  **
** included in the packaging of this file.  Please review the      **
** following information to ensure the GNU General Public License  **
** version 3.0 requirements will be met:                           **
** http://www.gnu.org/copyleft/gpl.html.                           **
**                                                                 **
*********************************************************************/

#ifndef ICL_SPINNER_HANDLE_H
#define ICL_SPINNER_HANDLE_H

#include <ICLQt/GUIHandle.h>

/** \cond */
class QSpinBox;
/** \endcond */


namespace icl{
  
  /// Handle class for spinner components \ingroup HANDLES
  class SpinnerHandle : public GUIHandle<QSpinBox>{

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
}

#endif
