#include "iclDC.h"
#include "iclDCDeviceOptions.h"
#include <dc1394/conversions.h>

#include <iclImg.h>
#include <iclCC.h>
#include <iclMutex.h>
#include <iclSignalHandler.h>
#include <iclDCGrabberThread.h>
#include <iclDCDevice.h>
#include <map>
#include <iclMutex.h>
#include <signal.h>

using std::string;
using std::vector;

namespace icl{
  namespace dc{

#define MODE_SWITCH(X) case X: return #X
#define MODE_SWITCH_IF(X) if(s==#X) return X 
#define MODE_SWITCH_ELSE(X) else if(s==#X) return X

    string to_string(dc1394video_mode_t vm){
      // {{{ open
#define MODE_SWITCH_A(S,F) case DC1394_VIDEO_MODE_##S##_##F: return #S "-" #F
#define MODE_SWITCH_B(S) case DC1394_VIDEO_MODE_##S: return #S
      switch(vm){
        MODE_SWITCH_A(160x120,YUV444);
        MODE_SWITCH_A(320x240,YUV422);
        MODE_SWITCH_A(640x480,YUV411);
        MODE_SWITCH_A(640x480,YUV422);
        MODE_SWITCH_A(640x480,RGB8);
        MODE_SWITCH_A(640x480,MONO8);
        MODE_SWITCH_A(640x480,MONO16);
        MODE_SWITCH_A(800x600,YUV422);
        MODE_SWITCH_A(800x600,RGB8);
        MODE_SWITCH_A(800x600,MONO8);
        MODE_SWITCH_A(1024x768,YUV422);
        MODE_SWITCH_A(1024x768,RGB8);
        MODE_SWITCH_A(1024x768,MONO8);
        MODE_SWITCH_A(800x600,MONO16);
        MODE_SWITCH_A(1024x768,MONO16);
        MODE_SWITCH_A(1280x960,YUV422);
        MODE_SWITCH_A(1280x960,RGB8);
        MODE_SWITCH_A(1280x960,MONO8);
        MODE_SWITCH_A(1600x1200,YUV422);
        MODE_SWITCH_A(1600x1200,RGB8);
        MODE_SWITCH_A(1600x1200,MONO8);
        MODE_SWITCH_A(1280x960,MONO16);
        MODE_SWITCH_A(1600x1200,MONO16);
        MODE_SWITCH_B(EXIF);
        MODE_SWITCH_A(FORMAT7,0);
        MODE_SWITCH_A(FORMAT7,1);
        MODE_SWITCH_A(FORMAT7,2);
        MODE_SWITCH_A(FORMAT7,3);
        MODE_SWITCH_A(FORMAT7,4);
        MODE_SWITCH_A(FORMAT7,5);
        MODE_SWITCH_A(FORMAT7,6);
        MODE_SWITCH_A(FORMAT7,7);
#undef MODE_SWITCH_A
#undef MODE_SWITCH_B
        default: return "unknown video mode";
      }
    }

    // }}}
    string to_string(dc1394framerate_t fr){
      // {{{ open
#define FR_SWITCH_A(L) case DC1394_FRAMERATE_##L: return #L "Hz";
#define FR_SWITCH_B(L,R) case DC1394_FRAMERATE_##L##_##R: return #L "." #R "Hz"
      switch(fr){
        FR_SWITCH_B(1,875);
        FR_SWITCH_B(3,75);
        FR_SWITCH_B(7,5);
        FR_SWITCH_A(15);
        FR_SWITCH_A(30);
        FR_SWITCH_A(60);
        FR_SWITCH_A(120);
        FR_SWITCH_A(240);
#undef FR_SWITCH_A
#undef FR_SWITCH_B
        default: return "unknown framerate";
      }
    }

    // }}}
    string to_string(dc1394color_filter_t f){
      // {{{ open

      switch(f){
        MODE_SWITCH(DC1394_COLOR_FILTER_RGGB);
        MODE_SWITCH(DC1394_COLOR_FILTER_GBRG);
        MODE_SWITCH(DC1394_COLOR_FILTER_GRBG);
        MODE_SWITCH(DC1394_COLOR_FILTER_BGGR);
        default: return "unknown color filter";
      }  
    }

    // }}}
    string to_string(dc1394bayer_method_t bm){
      // {{{ open

      switch(bm){
        MODE_SWITCH(DC1394_BAYER_METHOD_NEAREST);
        MODE_SWITCH(DC1394_BAYER_METHOD_SIMPLE);
        MODE_SWITCH(DC1394_BAYER_METHOD_BILINEAR);
        MODE_SWITCH(DC1394_BAYER_METHOD_HQLINEAR);
        MODE_SWITCH(DC1394_BAYER_METHOD_DOWNSAMPLE);
        MODE_SWITCH(DC1394_BAYER_METHOD_EDGESENSE);
        MODE_SWITCH(DC1394_BAYER_METHOD_VNG);
        MODE_SWITCH(DC1394_BAYER_METHOD_AHD);
        default: return "unknown bayer method";
      }
    }

    // }}}
    string to_string(dc1394feature_t f){
      // {{{ open

      switch(f){
        MODE_SWITCH(DC1394_FEATURE_BRIGHTNESS);
        MODE_SWITCH(DC1394_FEATURE_EXPOSURE);
        MODE_SWITCH(DC1394_FEATURE_SHARPNESS);
        MODE_SWITCH(DC1394_FEATURE_WHITE_BALANCE);
        MODE_SWITCH(DC1394_FEATURE_HUE);
        MODE_SWITCH(DC1394_FEATURE_SATURATION);
        MODE_SWITCH(DC1394_FEATURE_GAMMA);
        MODE_SWITCH(DC1394_FEATURE_SHUTTER);
        MODE_SWITCH(DC1394_FEATURE_GAIN);
        MODE_SWITCH(DC1394_FEATURE_IRIS);
        MODE_SWITCH(DC1394_FEATURE_FOCUS);
        MODE_SWITCH(DC1394_FEATURE_TEMPERATURE);
        MODE_SWITCH(DC1394_FEATURE_TRIGGER);
        MODE_SWITCH(DC1394_FEATURE_TRIGGER_DELAY);
        MODE_SWITCH(DC1394_FEATURE_WHITE_SHADING);
        MODE_SWITCH(DC1394_FEATURE_FRAME_RATE);
        MODE_SWITCH(DC1394_FEATURE_ZOOM);
        MODE_SWITCH(DC1394_FEATURE_PAN);
        MODE_SWITCH(DC1394_FEATURE_TILT);
        MODE_SWITCH(DC1394_FEATURE_OPTICAL_FILTER);
        MODE_SWITCH(DC1394_FEATURE_CAPTURE_SIZE);
        MODE_SWITCH(DC1394_FEATURE_CAPTURE_QUALITY);     
        default: return "unknown feature id";
      }
    }

      // }}}
      
    dc1394video_mode_t videomode_from_string(const string &s){
      // {{{ open
#define MODE_SWITCH_B_IF(S,F) if(s==#S "-" #F) return DC1394_VIDEO_MODE_##S##_##F
#define MODE_SWITCH_B_ELIF(S,F) else MODE_SWITCH_B_IF(S,F)
#define MODE_SWITCH_C_ELIF(S) else if(s==#S) return DC1394_VIDEO_MODE_##S
      
      MODE_SWITCH_B_IF(160x120,YUV444);
      MODE_SWITCH_B_ELIF(320x240,YUV422);
      MODE_SWITCH_B_ELIF(640x480,YUV411);
      MODE_SWITCH_B_ELIF(640x480,YUV422);
      MODE_SWITCH_B_ELIF(640x480,RGB8);
      MODE_SWITCH_B_ELIF(640x480,MONO8);
      MODE_SWITCH_B_ELIF(640x480,MONO16);
      MODE_SWITCH_B_ELIF(800x600,YUV422);
      MODE_SWITCH_B_ELIF(800x600,RGB8);
      MODE_SWITCH_B_ELIF(800x600,MONO8);
      MODE_SWITCH_B_ELIF(1024x768,YUV422);
      MODE_SWITCH_B_ELIF(1024x768,RGB8);
      MODE_SWITCH_B_ELIF(1024x768,MONO8);
      MODE_SWITCH_B_ELIF(800x600,MONO16);
      MODE_SWITCH_B_ELIF(1024x768,MONO16);
      MODE_SWITCH_B_ELIF(1280x960,YUV422);
      MODE_SWITCH_B_ELIF(1280x960,RGB8);
      MODE_SWITCH_B_ELIF(1280x960,MONO8);
      MODE_SWITCH_B_ELIF(1600x1200,YUV422);
      MODE_SWITCH_B_ELIF(1600x1200,RGB8);
      MODE_SWITCH_B_ELIF(1600x1200,MONO8);
      MODE_SWITCH_B_ELIF(1280x960,MONO16);
      MODE_SWITCH_B_ELIF(1600x1200,MONO16);
      MODE_SWITCH_C_ELIF(EXIF);
      MODE_SWITCH_B_ELIF(FORMAT7,0);
      MODE_SWITCH_B_ELIF(FORMAT7,1);
      MODE_SWITCH_B_ELIF(FORMAT7,2);
      MODE_SWITCH_B_ELIF(FORMAT7,3);
      MODE_SWITCH_B_ELIF(FORMAT7,4);
      MODE_SWITCH_B_ELIF(FORMAT7,5);
      MODE_SWITCH_B_ELIF(FORMAT7,6);
      MODE_SWITCH_B_ELIF(FORMAT7,7);
#undef MODE_SWITCH_B_ELIF
#undef MODE_SWITCH_B_IF
      return (dc1394video_mode_t)-1;
      
    }

    // }}}
    dc1394framerate_t framerate_from_string(const string &s){
      // {{{ open
#define FR_SWITCH_A(L) if(s== #L "Hz") return DC1394_FRAMERATE_##L
#define FR_SWITCH_B(L,R) if(s== #L "." #R "Hz") return DC1394_FRAMERATE_##L##_##R
      
      FR_SWITCH_B(1,875);
      FR_SWITCH_B(3,75);
      FR_SWITCH_B(7,5);
      FR_SWITCH_A(15);
      FR_SWITCH_A(30);
      FR_SWITCH_A(60);
      FR_SWITCH_A(120);
      FR_SWITCH_A(240);

#undef FR_SWICH_A
#undef FR_SWICH_B
      return (dc1394framerate_t)-1;
    }

    // }}}
    dc1394bayer_method_t bayermethod_from_string(const string &s){
      // {{{ open

      MODE_SWITCH_IF(DC1394_BAYER_METHOD_NEAREST);
      MODE_SWITCH_ELSE(DC1394_BAYER_METHOD_SIMPLE);
      MODE_SWITCH_ELSE(DC1394_BAYER_METHOD_BILINEAR);
      MODE_SWITCH_ELSE(DC1394_BAYER_METHOD_HQLINEAR);
      MODE_SWITCH_ELSE(DC1394_BAYER_METHOD_DOWNSAMPLE);
      MODE_SWITCH_ELSE(DC1394_BAYER_METHOD_EDGESENSE);
      MODE_SWITCH_ELSE(DC1394_BAYER_METHOD_VNG);
      MODE_SWITCH_ELSE(DC1394_BAYER_METHOD_AHD);
      return (dc1394bayer_method_t)-1;
    }

    // }}}
    dc1394feature_t feature_from_string(const string &s){
      // {{{ open

      MODE_SWITCH_IF(DC1394_FEATURE_BRIGHTNESS);
      MODE_SWITCH_ELSE(DC1394_FEATURE_EXPOSURE);
      MODE_SWITCH_ELSE(DC1394_FEATURE_SHARPNESS);
      MODE_SWITCH_ELSE(DC1394_FEATURE_WHITE_BALANCE);
      MODE_SWITCH_ELSE(DC1394_FEATURE_HUE);
      MODE_SWITCH_ELSE(DC1394_FEATURE_SATURATION);
      MODE_SWITCH_ELSE(DC1394_FEATURE_GAMMA);
      MODE_SWITCH_ELSE(DC1394_FEATURE_SHUTTER);
      MODE_SWITCH_ELSE(DC1394_FEATURE_GAIN);
      MODE_SWITCH_ELSE(DC1394_FEATURE_IRIS);
      MODE_SWITCH_ELSE(DC1394_FEATURE_FOCUS);
      MODE_SWITCH_ELSE(DC1394_FEATURE_TEMPERATURE);
      MODE_SWITCH_ELSE(DC1394_FEATURE_TRIGGER);
      MODE_SWITCH_ELSE(DC1394_FEATURE_TRIGGER_DELAY);
      MODE_SWITCH_ELSE(DC1394_FEATURE_WHITE_SHADING);
      MODE_SWITCH_ELSE(DC1394_FEATURE_FRAME_RATE);
      MODE_SWITCH_ELSE(DC1394_FEATURE_ZOOM);
      MODE_SWITCH_ELSE(DC1394_FEATURE_PAN);
      MODE_SWITCH_ELSE(DC1394_FEATURE_TILT);
      MODE_SWITCH_ELSE(DC1394_FEATURE_OPTICAL_FILTER);
      MODE_SWITCH_ELSE(DC1394_FEATURE_CAPTURE_SIZE);
      MODE_SWITCH_ELSE(DC1394_FEATURE_CAPTURE_QUALITY);
      return (dc1394feature_t)-1;
    }

    // }}}

#undef MODE_SWITCH
#undef MODE_SWITCH_IF
#undef MODE_SWITCH_ELSE
    
    const std::vector<std::string> &getListOfAllFeatures(){
      // {{{ open

      static vector<string> v;
      if(!v.size()){
        for(dc1394feature_t f = DC1394_FEATURE_MIN; f<=DC1394_FEATURE_MAX; f=(dc1394feature_t)((int)f+1)){
          v.push_back(to_string(f));
        }
      }
      
      return v;
    }

    // }}}

    
    
    class DCSignalHandler : public SignalHandler{
    public:
      DCSignalHandler():SignalHandler("SIGINT,SIGSEGV"){}
      virtual void handleSignals(const string &signal){
        // {{{ open
        printf("[Unclean break detected. Signal \"%s\"]\n",signal.c_str());

        m_oMutex.lock();
        DCGrabberThread::stopAllGrabberThreads();
        m_oMutex.unlock();
        // printf("done! (please ignore \"Hangup\" statement)\n");
        // 

        killCurrentProcess();
      }

      // }}}
      
    private:
      Mutex m_oMutex;
    };
   
    void install_signal_handler(){
      // {{{ open

      static DCSignalHandler dcs;
    }

    // }}}

    void initialize_dc_cam(dc1394camera_t *c, int nDMABuffers, DCDeviceOptions *options){
      // {{{ open

      dc1394_capture_stop(c);
      set_streaming(c,false);
      //dc1394_cleanup_iso_channels_and_bandwidth(c);
      
      // switch over the camera
      // TODO use max here
      
      set_iso_speed(c,options->isoMBits);

      // old      dc1394_video_set_iso_speed(c,DC1394_ISO_SPEED_400);
      
      
      if((int)options->videomode != -1){
        dc1394_video_set_mode(c,options->videomode);
      }
      if((int)options->framerate != -1){
        dc1394_video_set_framerate(c, options->framerate);
      } // otherwise, the current framerate is used
      
      // falls hier fehler : channel cleanen!
      dc1394_capture_setup(c,nDMABuffers,DC1394_CAPTURE_FLAGS_DEFAULT);
      set_streaming(c,true);
      
    }

    // }}}
    
    void release_dc_cam(dc1394camera_t *c){
      // {{{ open
      set_streaming(c,false);
      dc1394_capture_stop(c);

      //dc1394_cleanup_iso_channels_and_bandwidth(c);
    }

    // }}}

    void set_streaming(dc1394camera_t* c, bool on){
      // {{{ open
      dc1394switch_t currVal=DC1394_OFF;
      dc1394_video_get_transmission(c,&currVal);
      if( currVal == DC1394_OFF && on == true){
        dc1394_video_set_transmission(c,DC1394_ON);
      }else if( currVal == DC1394_ON && on == false){
        dc1394_video_set_transmission(c,DC1394_OFF);
      }
    }

    // }}}
    dc1394video_frame_t *get_newest_frame(dc1394camera_t* c){
      // {{{ open
      //  set_streaming(c,false);
      dc1394video_frame_t *frame=0;
      dc1394error_t err = DC1394_SUCCESS;
      do{
        err = dc1394_capture_dequeue(c,DC1394_CAPTURE_POLICY_POLL,&frame);
        if(err == DC1394_SUCCESS && frame){
          dc1394_capture_enqueue(c,frame);
        }
      }while(err == DC1394_SUCCESS);
  
      //set_streaming(c,true);
      dc1394_capture_dequeue(c,DC1394_CAPTURE_POLICY_WAIT,&frame); 
      return frame;  
    }

    // }}}
    
    dc1394video_frame_t *get_a_frame(dc1394camera_t* c){
      // {{{ open
      dc1394video_frame_t *frame=0;
      dc1394error_t err = dc1394_capture_dequeue(c,DC1394_CAPTURE_POLICY_WAIT,&frame);
      if(err == DC1394_SUCCESS){
        return frame;  
      }else{
        return 0;
      }
    }

    // }}}
    
    namespace{
      void rgb_to_gray_util(const icl8u *src, icl8u *dst, const icl8u *dstEnd, const Size &frameSize){

#ifdef HAVE_IPP
        int step = frameSize.width*sizeof(icl8u);
        ippiRGBToGray_8u_C3C1R(src,step*3,dst,step,frameSize); 
#else
        for(;dst < dstEnd; src+=3, ++dst){
          // we took this conversion from the IPP-manual for compability with IPP
          // function 
          *dst = clipped_cast<float,icl8u>(0.299*src[0]+0.587*src[1]+0.114*src[2]);
        }
#endif
      }

      void convert_YUV411_to_gray8(uint8_t *restrict src, uint8_t *restrict dest, uint32_t width, uint32_t height){
        register int i = (width*height) + ( (width*height) >> 1 )-1;
        register int j = (width*height)-1;
        
        while (i >= 0) {
          dest[j--] = (uint8_t) src[i--];
          dest[j--] = (uint8_t) src[i--];
          i--;
          dest[j--] = (uint8_t) src[i--];
          dest[j--] = (uint8_t) src[i--];
          i--;
        }
      }
      void convert_YUV422_to_gray8(uint8_t *restrict src, uint8_t *restrict dest, uint32_t width, uint32_t height,uint32_t byte_order){
        register int i = ((width*height) << 1)-1;
        register int j = (width*height)-1;
        switch(byte_order){
          case DC1394_BYTE_ORDER_UYVY:
            while (i >= 0) {
              dest[j--]= (uint8_t)src[i--]; i--;
              dest[j--]= (uint8_t)src[i--]; i--;
            }
            break;
          case DC1394_BYTE_ORDER_YUYV:
            while (i >= 0) {
              i--; dest[j--]= (uint8_t)src[i--];
              i--; dest[j--]= (uint8_t)src[i--];
            }
            break;
          default:
            ERROR_LOG("illegal byte_order: " << byte_order);
        }
      }
      void convert_YUV444_to_gray8(uint8_t *restrict src, uint8_t *restrict dest, uint32_t width, uint32_t height){
        register int i = (width*height) + ( (width*height) << 1 ) -1;
        register int j = (width*height)-1;
        
        while (i >= 0) {
          i--; dest[j--] = src[i--]; i--;
        }
      }
    }

    void extract_image_to_gray(dc1394video_frame_t *f,
                               dc1394color_filter_t bayerLayout,
                                ImgBase **ppoDst, 
                                const Size &desiredSizeHint, 
                                depth desiredDepthHint,
                                std::vector<icl8u> &dataBuffer,
                               dc1394bayer_method_t bayerMethod){
      // {{{ open
      //      DEBUG_LOG("color_filter:" << f->color_filter);
      (void)desiredDepthHint;
      Size frameSize(f->size[0],f->size[1]);
      ensureCompatible(ppoDst,depth8u,frameSize,formatGray);

      if(bayerLayout){
        //      if(f->color_filter){ unfortunately this is not set for some cams??
        if((int)dataBuffer.size() < frameSize.getDim()*3){
          dataBuffer.resize(frameSize.getDim()*3);
        }
        dc1394_bayer_decoding_8bit(f->image,
                                   dataBuffer.data(),
                                   frameSize.width,
                                   frameSize.height,
                                   bayerLayout,
                                   bayerMethod);
        icl8u *dst = (*ppoDst)->asImg<icl8u>()->getData(0);
        const icl8u *src = dataBuffer.data();
        const icl8u *dstEnd = dst+frameSize.getDim();

        rgb_to_gray_util(src,dst,dstEnd,frameSize);
        
      }else{
        for(int i=0;i<100;++i) std::cout << (int)(f->image[i]) << " ";
        std::cout << std::endl;
        icl8u *dstData = (*ppoDst)->asImg<icl8u>()->getData(0);
        dc1394error_t err = dc1394_convert_to_MONO8(f->image, 
                                                    dstData,
                                                    frameSize.width,
                                                    frameSize.height,
                                                    f->yuv_byte_order, 
                                                    f->color_coding,
                                                    f->data_depth);
        if(err == DC1394_FUNCTION_NOT_SUPPORTED){
          switch(f->color_coding){
            case DC1394_COLOR_CODING_MONO8:
            case DC1394_COLOR_CODING_MONO16S:
              if(f->data_depth == 8){
                memcpy(dstData,f->image,frameSize.getDim());
              }else{
                icl::convert((icl16s*)f->image,((icl16s*)f->image)+frameSize.getDim(),dstData);
              }
              break;
            case DC1394_COLOR_CODING_YUV411:
              convert_YUV411_to_gray8(f->image,dstData,frameSize.width,frameSize.height);
              break;
            case DC1394_COLOR_CODING_YUV422:
              convert_YUV422_to_gray8(f->image,dstData,frameSize.width,frameSize.height,f->yuv_byte_order);
              break;
            case DC1394_COLOR_CODING_YUV444:
              convert_YUV444_to_gray8(f->image,dstData,frameSize.width,frameSize.height);
              break;
            case DC1394_COLOR_CODING_RGB8:
              rgb_to_gray_util(f->image,dstData,dstData+frameSize.getDim(),frameSize);
              break;
            default:
              ERROR_LOG("unable to convert DC1394_COLOR_CODING-type: " << f->color_coding);
          }
        }
      }
      //  old }
      if(ppoDst && *ppoDst){
        (*ppoDst)->setTime(Time(f->timestamp));
      }
    }

    // }}}
    
    void extract_image_to_rgb(dc1394video_frame_t *f,
                              dc1394color_filter_t bayerLayout,
                              ImgBase **ppoDst, 
                              const Size &desiredSizeHint, 
                              depth desiredDepthHint,
                              std::vector<icl8u> &dataBuffer,
                              dc1394bayer_method_t bayerMethod){
      
      // {{{ open

      (void)desiredDepthHint;
      Size frameSize(f->size[0],f->size[1]);
     
      if(bayerLayout){
        if(f->data_depth == 16){
          ensureCompatible(ppoDst,depth16s, frameSize,formatRGB);
          
          ERROR_LOG("16Bit bayer decoding is not supported yet");
        }else{
          ensureCompatible(ppoDst,depth8u, frameSize,formatRGB);
          if((int)dataBuffer.size() < frameSize.getDim()*3){
            dataBuffer.resize(frameSize.getDim()*3);
          }
          dc1394_bayer_decoding_8bit(f->image,
                                     dataBuffer.data(),
                                     frameSize.width,
                                     frameSize.height,
                                     bayerLayout,
                                     bayerMethod);
          interleavedToPlanar(dataBuffer.data(),(*ppoDst)->asImg<icl8u>());
        }
      }else{
        ensureCompatible(ppoDst,depth8u, frameSize,formatRGB);
        /// rgb works directly TODO
        if((int)dataBuffer.size() < frameSize.getDim()*3){
          dataBuffer.resize(frameSize.getDim()*3);
        }
        dc1394_convert_to_RGB8(f->image,
                               dataBuffer.data(),
                               frameSize.width,
                               frameSize.height,
                               f->yuv_byte_order,
                               f->color_coding,
                               f->data_depth);
        interleavedToPlanar(dataBuffer.data(),(*ppoDst)->asImg<icl8u>());
        if(ppoDst && *ppoDst){
          (*ppoDst)->setTime(Time(f->timestamp));
        }
      }
    }

    // }}}

    void extract_image_to(dc1394video_frame_t *f,
                          dc1394color_filter_t bayerLayout,
                          ImgBase **ppoDst, 
                          const Size &desiredSizeHint, 
                          format desiredFormatHint,
                          depth desiredDepthHint,
                          std::vector<icl8u> &dataBuffer,
                          dc1394bayer_method_t bayerMethod){

      // {{{ open

      // This function must work dynamically --> it must not use is_firefly_mono for example !!
      
      (void)desiredDepthHint;
      Size frameSize(f->size[0],f->size[1]);
      switch(desiredFormatHint){
        case formatGray:
          extract_image_to_gray(f,bayerLayout,ppoDst,desiredSizeHint,desiredDepthHint,dataBuffer,bayerMethod);
          break;
        default: // rgb is returned .. and converted lateron
          extract_image_to_rgb(f,bayerLayout,ppoDst,desiredSizeHint,desiredDepthHint,dataBuffer,bayerMethod);
          break;
      }
    }

    // }}}

    void extract_image_to_2(dc1394video_frame_t *f,
                            dc1394color_filter_t bayerLayout,
                            ImgBase **ppoDst, 
                            std::vector<icl8u> &dataBuffer,
                            dc1394bayer_method_t bayerMethod){
      // {{{ open xxx
      
      ICLASSERT_RETURN( f );
      unsigned char *data = f->image;
      int width = (int)f->size[0];
      int height = (int)f->size[1];
      dc1394color_coding_t cc = f->color_coding;
      //dc1394color_filter_t cf = f->color_filter; // not set (why ever)
      uint32_t yuv_byte_order = f->yuv_byte_order;
      uint32_t data_depth      = f->data_depth;
    
      if(bayerLayout){
        if(data_depth == 16){
          ensureCompatible(ppoDst,depth16s, Size(width,height),formatRGB);
          
          ERROR_LOG("16Bit bayer decoding is not supported yet");
          /** This does not work
              if((int)dataBuffer.size() < width*height*3*2){
              dataBuffer.resize(width*height*3*2);
              }
              dc1394_bayer_decoding_16bit((const uint16_t*)data, (uint16_t*)dataBuffer.data(),
              width,height,bayerLayout, bayerMethod, 0);
              
              interleavedToPlanar((const icl16s*)dataBuffer.data(),(*ppoDst)->asImg<icl16s>());
           **/
        }else{
          ensureCompatible(ppoDst,depth8u, Size(width,height),formatRGB);
          if((int)dataBuffer.size() < width*height*3){
            dataBuffer.resize(width*height*3);
          }
          dc1394_bayer_decoding_8bit(data,dataBuffer.data(),width,height,bayerLayout,bayerMethod);
          interleavedToPlanar(dataBuffer.data(),(*ppoDst)->asImg<icl8u>());
        }


      }else{
        static const dc1394color_coding_t gray_ccs[5] = {
          DC1394_COLOR_CODING_MONO8,
          DC1394_COLOR_CODING_MONO16,
          DC1394_COLOR_CODING_MONO16S,
          DC1394_COLOR_CODING_RAW8,
          DC1394_COLOR_CODING_RAW16
        };
        if(std::find(gray_ccs,gray_ccs+5,cc)==gray_ccs+5){
          ensureCompatible(ppoDst,depth8u, Size(width,height),formatRGB);
          if((int)dataBuffer.size() < width*height*3){
            dataBuffer.resize(width*height*3);
          }
          dc1394_convert_to_RGB8(data,dataBuffer.data(),width,height,yuv_byte_order,cc,data_depth);
          interleavedToPlanar(dataBuffer.data(),(*ppoDst)->asImg<icl8u>());
        }else{
          ensureCompatible(ppoDst,depth8u, Size(width,height),formatGray);
          dc1394_convert_to_MONO8(data, (*ppoDst)->asImg<icl8u>()->getData(0),width,height,yuv_byte_order, cc, data_depth);
        }
      }
      if(ppoDst && *ppoDst){
        (*ppoDst)->setTime(Time(f->timestamp));
      }
    }
    // }}}
    
    bool can_extract_image_to(dc1394video_frame_t *f,
                              const Size &desiredSizeHint, 
                              format desiredFormatHint,
                              depth desiredDepthHint){
      // {{{ open

      if(desiredDepthHint != depth8u) return false;
      if(desiredSizeHint != Size(f->size[0],f->size[1])) return false;
      if(desiredFormatHint == formatGray && f->data_depth == 8) return true;
      if(desiredFormatHint != formatRGB) return false;

      return true;
    }

    // }}}

#if 0
    void grab_frame(int DUMMY,dc1394camera_t* c, ImgBase **image){
      // {{{ open
      ERROR_LOG("this function has been removed!");
      /**
      ICLASSERT_RETURN(image);
      format fmt = *image ? (*image)->getFormat() : formatRGB;
      ICLASSERT_RETURN( fmt == formatGray || fmt == formatRGB );

      //dc1394video_frame_t *f = get_newest_frame(c);
      dc1394video_frame_t *f = get_a_frame(c);
  
      //      dc1394color_filter_t cfilter = f->color_filter;
      Size size = Size(f->size[0],f->size[1]);
  
  
      ensureCompatible(image,depth8u,size,fmt);
      if(fmt == formatGray){
        dc1394_convert_to_MONO8(f->image, (*image)->asImg<icl8u>()->getData(0),size.width,size.height,f->yuv_byte_order,f->color_coding,f->data_depth);
      }else if(fmt == formatRGB){
        static std::vector<icl8u> rgbBuffer;
        if((int)rgbBuffer.size() < size.getDim()*3) rgbBuffer.resize(size.getDim()*3);

        //dc1394_convert_to_RGB8(f->image, &(rgbBuffer[0]),size.width,size.height,f->yuv_byte_order,f->color_coding,f->data_depthyyy);
        dc1394_bayer_decoding_8bit(f->image, 
                                   &(rgbBuffer[0]),
                                   size.width,
                                   size.height,
                                   DC1394_COLOR_FILTER_GBRG,
                                   DC1394_BAYER_METHOD_BILINEAR);

        interleavedToPlanar(&(rgbBuffer[0]),(*image)->asImg<icl8u>());
      }
      dc1394_capture_enqueue(c,f);

      **/
    }

    // }}}
#endif

    class DCContextCreator{
      // {{{ open

    public:
      Mutex mutex;
      dc1394_t *context;
      DCContextCreator():context(0){
      }
      ~DCContextCreator(){
        release();
      }
      void create(){
        mutex.lock();
        if(!context){
          context = dc1394_new();
        }
        mutex.unlock();
      }
      void release(){
        mutex.lock();
        if(context){
          dc1394_free(context);
          context = 0;
        }
        mutex.unlock();
      }
    };

    // }}}
    
    DCContextCreator STATIC_DC_CONTEXT;
    
    dc1394_t *get_static_context(){
      STATIC_DC_CONTEXT.create();
      return STATIC_DC_CONTEXT.context;
    }
    
    void free_static_context(){
      STATIC_DC_CONTEXT.release();
    }

    void set_iso_speed(dc1394camera_t* c, int mbits){
      ICLASSERT_RETURN(c);
      if(!mbits) return; // this is the default value for "do nothing"
      if(mbits > 400 && !c->bmode_capable == DC1394_TRUE){
        ERROR_LOG("device does not support IEEE 1394-B mode");
        return;
      }

      switch(mbits){
#define CASE(X)                                                            \
        case X:                                                            \
        if(mbits>400){                                                     \
          dc1394_video_set_operation_mode(c,DC1394_OPERATION_MODE_1394B);  \
          }else{                                                           \
          dc1394_video_set_operation_mode(c,DC1394_OPERATION_MODE_LEGACY); \
        }                                                                  \
        dc1394_video_set_iso_speed(c,DC1394_ISO_SPEED_##X);                \
        break
        CASE(100); CASE(200); CASE(400); CASE(800);
#undef CASE
        case 1600: case 3200:
          ERROR_LOG("iso speed 1600 and isospeed 3200 are not yet supported"); break;
        default:
          ERROR_LOG("invalid iso speed value: " << mbits << "(supported values are: 100,200,400 and 800)");
      }
    }
  }
  
}
