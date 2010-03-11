/*************************************************************************** 
**                                                                        **
** Copyright (C) 2006-2010 neuroinformatics group (vision)                **
**                         University of Bielefeld                        **
**                         nivision@techfak.uni-bielefeld.de              **
**                                                                        **
** This file is part of the ICLIO module of ICL                           **
**                                                                        **
** Commercial License                                                     **
** Commercial usage of ICL is possible and must be negotiated with us.    **
** See our website www.iclcv.org for more details                         **
**                                                                        **
** GNU General Public License Usage                                       **
** Alternatively, this file may be used under the terms of the GNU        **
** General Public License version 3.0 as published by the Free Software   **
** Foundation and appearing in the file LICENSE.GPL included in the       **
** packaging of this file.  Please review the following information to    **
** ensure the GNU General Public License version 3.0 requirements will be **
** met: http://www.gnu.org/copyleft/gpl.html.                             **
**                                                                        **
***************************************************************************/ 

#include <ICLIO/DCFrameQueue.h>
#include <ICLIO/DC.h>
#include <ICLUtils/Macros.h>

namespace icl{
  namespace dc{
    
    DCFrameQueue::DCFrameQueue(dc1394camera_t* c, DCDeviceOptions *options,int nDMABuffers, int nQueuedBuffers):
      m_poCam(c),
      m_iBuffers(nDMABuffers),
      m_iQueuedBuffers(nQueuedBuffers)
    {

      initialize_dc_cam(c,nDMABuffers, options);
      
      /// dequeu all frames once 
#ifdef SYSTEM_APPLE
      for(int i=0;i<m_iBuffers-1;i++){
#else
      for(int i=0;i<m_iBuffers;i++){
#endif
        dc1394video_frame_t *frame=0;
        dc1394error_t err = dc1394_capture_dequeue(m_poCam,DC1394_CAPTURE_POLICY_WAIT,&frame);
 
        (void)err;
        // this does not work dc1394_capture_dequeu crashes when the bus is broken
        //if(err != DC1394_SUCCESS || !frame){
        //  ERROR_LOG("dc1394_capture_dequeue was not successfull -> trying to reset the bus");
        //  resetBus();
        //  dc1394error_t err = dc1394_capture_dequeue(m_poCam,DC1394_CAPTURE_POLICY_WAIT,&frame);
        //  if(err != DC1394_SUCCESS || !frame){
        //    ERROR_LOG("reseting the bus was not successful");
        //  }
        //}
        push(frame);

      }
      for(int i=0;i<m_iQueuedBuffers;i++){
        dc1394video_frame_t *frame = pop();
        dc1394_capture_enqueue(m_poCam,frame); 
      }    
    }
    DCFrameQueue::~DCFrameQueue(){
      //      release_dc_cam(m_poCam);
    }
    void DCFrameQueue::resetBus(){
      dc1394_reset_bus(m_poCam);
    }
    void DCFrameQueue::step(){
      dc1394video_frame_t *frame = pop();
      dc1394_capture_enqueue(m_poCam,frame); 
#define POLICY_TYPE DC1394_CAPTURE_POLICY_POLL
      frame=0;

      // OLD      dc1394_capture_dequeue(m_poCam,DC1394_CAPTURE_POLICY_WAIT,&frame);
      dc1394error_t err = dc1394_capture_dequeue(m_poCam,POLICY_TYPE,&frame);
      // dc1394error_t err = dc1394_capture_dequeue(m_poCam,DC1394_CAPTURE_POLICY_WAIT,&frame);

      // OLD RC7:     while(err !=  DC1394_SUCCESS ){ // sometimes frame was NULL but err was SUCCESS
      // DEBUG_LOG("entering loop");
      int nLoopTimes = 0;
      while(err !=  DC1394_SUCCESS || !frame){
        usleep(500);
        
        err = dc1394_capture_dequeue(m_poCam,POLICY_TYPE,&frame);
        //err = dc1394_capture_dequeue(m_poCam,DC1394_CAPTURE_POLICY_WAIT,&frame);
        
        nLoopTimes++;
      }
      //DEBUG_LOG("loop exited, it was called " << nLoopTimes << " times");
      
      push(frame);
    }    
    
    void DCFrameQueue::push(dc1394video_frame_t *f){
      lock();
      q.push(f);
      unlock();
    }
    
    dc1394video_frame_t *DCFrameQueue::pop(){
      lock();
        dc1394video_frame_t *f = q.front();
        q.pop();
        unlock();
        return f;
      }
      
      
      void DCFrameQueue::showDetails() const{
        dc1394video_mode_t vm;
        dc1394_video_get_mode(m_poCam,&vm);
        printf("using video mode %s \n",to_string(vm).c_str());
        
        dc1394framerate_t fr;
        dc1394_video_get_framerate(m_poCam,&fr);
        printf("using framerate %s \n",to_string(fr).c_str());
      }


  }
}
