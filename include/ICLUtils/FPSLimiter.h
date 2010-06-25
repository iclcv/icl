/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2010 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : include/ICLUtils/FPSLimiter.h                          **
** Module : ICLUtils                                               **
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
** The development of this software was supported by the           **
** Excellence Cluster EXC 277 Cognitive Interaction Technology.    **
** The Excellence Cluster EXC 277 is a grant of the Deutsche       **
** Forschungsgemeinschaft (DFG) in the context of the German       **
** Excellence Initiative.                                          **
**                                                                 **
*********************************************************************/

#ifndef ICL_FPS_LIMITER_H
#define ICL_FPS_LIMITER_H

#include <ICLUtils/Time.h>
#include <ICLUtils/FPSEstimator.h>
namespace icl{
  
  
  /// An fps limiter can be used to limit online applications FPS \ingroup TIME
  class FPSLimiter : public FPSEstimator{
    float m_maxFPS;
    mutable Time m_lastTime;
    mutable bool m_waitOff;
    
    public:
    /// creates new FPSLimiter instance with given parameter
    FPSLimiter(float maxFPS, int fpsEstimationInterval=10);
    
    /// sets max fps value
    void setMaxFPS(float maxFPS) { m_maxFPS = maxFPS; }

    /// returns max fps value
    float getMaxFPS() const { return m_maxFPS; }
    
    /// waits as long as necessary to reached desired FPS rate
    /** returns time actually waited (in microseconds) */
    float wait() const;
    
    /// as FPSEstimator::tic(), but with preceding wait()-call
    virtual void tic() const;
    
    /// as FPSEstimator::getFPSVal(), but with preceding wait()-call
    virtual float getFPSVal() const;
     
    /// as FPSEstimator::getFpsString, but with preceding wait()-call
    virtual std::string getFPSString(const std::string &fmt="%3.4f fps", int bufferSize=30) const;
    
    /// as FPSEstimator::showPPS, but with preceding wait()-call
    virtual void showFPS(const std::string &text="") const;
  };
}

#endif
