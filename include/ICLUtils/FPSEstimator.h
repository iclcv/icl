/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2010 Neuroinformatics, CITEC                 **
**                         University of Bielefeld                 **
**                Contact: nivision@techfak.uni-bielefeld.de       **
**                Website: www.iclcv.org                           **
**                                                                 **
** File   : include/ICLUtils/FPSEstimator.h                        **
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
*********************************************************************/

#ifndef ICL_FPS_ESTIMATOR_H
#define ICL_FPS_ESTIMATOR_H

#include <ICLUtils/Time.h>
#include <deque>
#include <string>

namespace icl{
  ///Utility clas for online FPS estimation \ingroup TIME
  class FPSEstimator{
    public:

    ///Constructor
    /** Current FPS values are averaged over given intervall n*/
    FPSEstimator(int n=2);

    /// Sets a new averaging interval
    void reset(int n);
    
    /// pushes current time into the time averaging queue and pop oldest time from the queue
    virtual void tic() const;
    
    /// applies tic() and returns current FPS estimate
    virtual float getFPSVal() const;
     
    /// applies tic() and returns current fps estimate as formated string
    virtual std::string getFPSString(const std::string &fmt="%3.4f fps", int bufferSize=30) const;
     
    /// applies tic() and shows current FPS estimate on std::out
    virtual void showFPS(const std::string &text="") const;
    
    private:
    /// internal time queue
    mutable std::deque<Time> m_qTimes;
    
    /// time-queues size
    int m_iN;
  };

#define FPS_LOG_THIS_FUNCTION(N) static FPSEstimator __FPSEstimator__((N)); \
                                 __FPSEstimator__.showFps(__FUNCTION__);
  
}


#endif
