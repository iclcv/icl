/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2010 Neuroinformatics, CITEC                 **
**                         University of Bielefeld                 **
**                Contact: nivision@techfak.uni-bielefeld.de       **
**                Website: www.iclcv.org                           **
**                                                                 **
** File   : include/ICLQt/FSliderHandle.h                          **
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

#ifndef ICL_FSLIDER_HANDLE_H
#define ICL_FSLIDER_HANDLE_H

#include <ICLQt/GUIHandle.h>

/** \cond */
class QSlider;
/** \endcond */

namespace icl{
  
  /// Handle class for slider componets \ingroup HANDLES
  class FSliderHandle : public GUIHandle<QSlider>{
    public:
    /// Create an empty slider handle
    FSliderHandle();

    /// create a slider handle
    FSliderHandle(QSlider *sl,float *minV, float *maxV, float *M, float *B,int range, GUIWidget *w);
    
    /// set the min value
    void setMin(float min);

    /// set the max value
    void setMax(float max);

    /// set the range of the slider
    void setRange(float min, float max){ setMin(min); setMax(max); }
    
    /// set the current value of the slider
    void setValue(float val);

    /// sets all parameters of a slider
    void setAll(float min ,float max, float val){ setRange(min,max); setValue(val); }

    /// returns the current min. of the slider
    float getMin() const;

    /// returns the current max. of the slider
    float getMax() const;

    /// returns the current value of the slider
    float getValue() const;
    
    /// assigns a new value to the slider (equal to setValue)
    void operator=(float val) { setValue(val); }
    
    private:
    
    /// internal utiltiy function which update the param for the slider equation
    void updateMB();
   
    /// current min value
    float *m_fMin;
    
    /// current max value
    float *m_fMax;
    
    /// utility function tranlating from valued to slider coordinates
    int f2i(float f) const{
      return (int)((f-*m_fB)/ *m_fM);
    }

    /// utility function tranlating from slider to value coordinates
    float i2f(int i) const{
      return *m_fM*i+*m_fB;
    } 

    /// Slope for the internal slider equation 
    float *m_fM;

    /// Bias for the internal slider equation 
    float *m_fB;
    
    /// accumulator of the current slider range
    int m_iSliderRange;
  };
  
}

#endif
