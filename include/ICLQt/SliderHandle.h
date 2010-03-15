/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2010 Neuroinformatics, CITEC                 **
**                         University of Bielefeld                 **
**                Contact: nivision@techfak.uni-bielefeld.de       **
**                Website: www.iclcv.org                           **
**                                                                 **
** File   : include/ICLQt/SliderHandle.h                           **
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

#ifndef ICL_SLIDER_HANDLE_H
#define ICL_SLIDER_HANDLE_H

#include <ICLQt/GUIHandle.h>


namespace icl{
  
  /** \cond */
  class ThreadedUpdatableSlider;
  /** \endcond */
 
  /// Handle class for slider componets \ingroup HANDLES
  class SliderHandle : public GUIHandle<ThreadedUpdatableSlider>{
    public:
    /// Creates and empty slider handle
    SliderHandle(){}

    /// create a slider handle
    SliderHandle(ThreadedUpdatableSlider *sl, GUIWidget *w):GUIHandle<ThreadedUpdatableSlider>(sl,w){}
    
    /// set the min value
    void setMin(int min);

    /// set the max value
    void setMax(int max);

    /// set the range of the slider
    void setRange(int min, int max);
    
    /// set the current value of the slider
    void setValue(int val);

    /// sets all parameters of a slider
    void setAll(int min ,int max, int val){ setRange(min,max); setValue(val); }

    /// returns the current min. of the slider
    int getMin() const;

    /// returns the current max. of the slider
    int getMax() const;

    /// returns the current value of the slider
    int getValue() const;
    
    /// assigns a new value to the slider (equal to setValue)
    void operator=(int val) { setValue(val); }
  };
  
}

#endif
