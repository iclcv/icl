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
      switch(vm){
        MODE_SWITCH(DC1394_VIDEO_MODE_160x120_YUV444);
        MODE_SWITCH(DC1394_VIDEO_MODE_320x240_YUV422);
        MODE_SWITCH(DC1394_VIDEO_MODE_640x480_YUV411);
        MODE_SWITCH(DC1394_VIDEO_MODE_640x480_YUV422);
        MODE_SWITCH(DC1394_VIDEO_MODE_640x480_RGB8);
        MODE_SWITCH(DC1394_VIDEO_MODE_640x480_MONO8);
        MODE_SWITCH(DC1394_VIDEO_MODE_640x480_MONO16);
        MODE_SWITCH(DC1394_VIDEO_MODE_800x600_YUV422);
        MODE_SWITCH(DC1394_VIDEO_MODE_800x600_RGB8);
        MODE_SWITCH(DC1394_VIDEO_MODE_800x600_MONO8);
        MODE_SWITCH(DC1394_VIDEO_MODE_1024x768_YUV422);
        MODE_SWITCH(DC1394_VIDEO_MODE_1024x768_RGB8);
        MODE_SWITCH(DC1394_VIDEO_MODE_1024x768_MONO8);
        MODE_SWITCH(DC1394_VIDEO_MODE_800x600_MONO16);
        MODE_SWITCH(DC1394_VIDEO_MODE_1024x768_MONO16);
        MODE_SWITCH(DC1394_VIDEO_MODE_1280x960_YUV422);
        MODE_SWITCH(DC1394_VIDEO_MODE_1280x960_RGB8);
        MODE_SWITCH(DC1394_VIDEO_MODE_1280x960_MONO8);
        MODE_SWITCH(DC1394_VIDEO_MODE_1600x1200_YUV422);
        MODE_SWITCH(DC1394_VIDEO_MODE_1600x1200_RGB8);
        MODE_SWITCH(DC1394_VIDEO_MODE_1600x1200_MONO8);
        MODE_SWITCH(DC1394_VIDEO_MODE_1280x960_MONO16);
        MODE_SWITCH(DC1394_VIDEO_MODE_1600x1200_MONO16);
        MODE_SWITCH(DC1394_VIDEO_MODE_EXIF);
        MODE_SWITCH(DC1394_VIDEO_MODE_FORMAT7_0);
        MODE_SWITCH(DC1394_VIDEO_MODE_FORMAT7_1);
        MODE_SWITCH(DC1394_VIDEO_MODE_FORMAT7_2);
        MODE_SWITCH(DC1394_VIDEO_MODE_FORMAT7_3);
        MODE_SWITCH(DC1394_VIDEO_MODE_FORMAT7_4);
        MODE_SWITCH(DC1394_VIDEO_MODE_FORMAT7_5);
        MODE_SWITCH(DC1394_VIDEO_MODE_FORMAT7_6);
        MODE_SWITCH(DC1394_VIDEO_MODE_FORMAT7_7);
        default: return "unknown video mode";
      }
    }

    // }}}
    string to_string(dc1394framerate_t fr){
      // {{{ open

      switch(fr){
        MODE_SWITCH(DC1394_FRAMERATE_1_875);
        MODE_SWITCH(DC1394_FRAMERATE_3_75);
        MODE_SWITCH(DC1394_FRAMERATE_7_5);
        MODE_SWITCH(DC1394_FRAMERATE_15);
        MODE_SWITCH(DC1394_FRAMERATE_30);
        MODE_SWITCH(DC1394_FRAMERATE_60);
        MODE_SWITCH(DC1394_FRAMERATE_120);
        MODE_SWITCH(DC1394_FRAMERATE_240);
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

      MODE_SWITCH_IF(DC1394_VIDEO_MODE_160x120_YUV444);
      MODE_SWITCH_ELSE(DC1394_VIDEO_MODE_320x240_YUV422);
      MODE_SWITCH_ELSE(DC1394_VIDEO_MODE_640x480_YUV411);
      MODE_SWITCH_ELSE(DC1394_VIDEO_MODE_640x480_YUV422);
      MODE_SWITCH_ELSE(DC1394_VIDEO_MODE_640x480_RGB8);
      MODE_SWITCH_ELSE(DC1394_VIDEO_MODE_640x480_MONO8);
      MODE_SWITCH_ELSE(DC1394_VIDEO_MODE_640x480_MONO16);
      MODE_SWITCH_ELSE(DC1394_VIDEO_MODE_800x600_YUV422);
      MODE_SWITCH_ELSE(DC1394_VIDEO_MODE_800x600_RGB8);
      MODE_SWITCH_ELSE(DC1394_VIDEO_MODE_800x600_MONO8);
      MODE_SWITCH_ELSE(DC1394_VIDEO_MODE_1024x768_YUV422);
      MODE_SWITCH_ELSE(DC1394_VIDEO_MODE_1024x768_RGB8);
      MODE_SWITCH_ELSE(DC1394_VIDEO_MODE_1024x768_MONO8);
      MODE_SWITCH_ELSE(DC1394_VIDEO_MODE_800x600_MONO16);
      MODE_SWITCH_ELSE(DC1394_VIDEO_MODE_1024x768_MONO16);
      MODE_SWITCH_ELSE(DC1394_VIDEO_MODE_1280x960_YUV422);
      MODE_SWITCH_ELSE(DC1394_VIDEO_MODE_1280x960_RGB8);
      MODE_SWITCH_ELSE(DC1394_VIDEO_MODE_1280x960_MONO8);
      MODE_SWITCH_ELSE(DC1394_VIDEO_MODE_1600x1200_YUV422);
      MODE_SWITCH_ELSE(DC1394_VIDEO_MODE_1600x1200_RGB8);
      MODE_SWITCH_ELSE(DC1394_VIDEO_MODE_1600x1200_MONO8);
      MODE_SWITCH_ELSE(DC1394_VIDEO_MODE_1280x960_MONO16);
      MODE_SWITCH_ELSE(DC1394_VIDEO_MODE_1600x1200_MONO16);
      MODE_SWITCH_ELSE(DC1394_VIDEO_MODE_EXIF);
      MODE_SWITCH_ELSE(DC1394_VIDEO_MODE_FORMAT7_0);
      MODE_SWITCH_ELSE(DC1394_VIDEO_MODE_FORMAT7_1);
      MODE_SWITCH_ELSE(DC1394_VIDEO_MODE_FORMAT7_2);
      MODE_SWITCH_ELSE(DC1394_VIDEO_MODE_FORMAT7_3);
      MODE_SWITCH_ELSE(DC1394_VIDEO_MODE_FORMAT7_4);
      MODE_SWITCH_ELSE(DC1394_VIDEO_MODE_FORMAT7_5);
      MODE_SWITCH_ELSE(DC1394_VIDEO_MODE_FORMAT7_6);
      MODE_SWITCH_ELSE(DC1394_VIDEO_MODE_FORMAT7_7);
      return (dc1394video_mode_t)-1;
    }

    // }}}
    dc1394framerate_t framerate_from_string(const string &s){
      // {{{ open

      MODE_SWITCH_IF(DC1394_FRAMERATE_1_875);
      MODE_SWITCH_ELSE(DC1394_FRAMERATE_3_75);
      MODE_SWITCH_ELSE(DC1394_FRAMERATE_7_5);
      MODE_SWITCH_ELSE(DC1394_FRAMERATE_15);
      MODE_SWITCH_ELSE(DC1394_FRAMERATE_30);
      MODE_SWITCH_ELSE(DC1394_FRAMERATE_60);
      MODE_SWITCH_ELSE(DC1394_FRAMERATE_120);
      MODE_SWITCH_ELSE(DC1394_FRAMERATE_240);
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

    bool is_firefly_color(dc1394camera_t* c){
      // {{{ open

      return string(c->model) == "Firefly MV FFMV-03MTC";
    }

    // }}}
    bool is_firefly_mono(dc1394camera_t* c){
      // {{{ open

      return string(c->model) == "Firefly MV FFMV-03MTM";
    }

    // }}}
    
    
    class DCSignalHandler : public SignalHandler{
    public:
      DCSignalHandler():SignalHandler("SIGINT,SIGSEGV"){}
      virtual void handleSignals(const string &signal){
        // {{{ open
        printf("DCSignalHandler caught the %s signal!\n",signal.c_str());
        printf("releasing cameras ...\n");
        m_oMutex.lock();
        DCGrabberThread::stopAllGrabberThreads();
        m_oMutex.unlock();
        printf("done! (please ignore \"Hangup\" statement)\n");
        killCurrentProcess();
        exit(0);
        
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
    
    Img8u extract_image(dc1394video_frame_t *f, format fmt){
      // {{{ open

      ICLASSERT_RETURN_VAL( f , Img8u());
      unsigned char *data = f->image;
      int width = (int)f->size[0];
      int height = (int)f->size[1];
      dc1394color_coding_t cc = f->color_coding;
      //      dc1394color_filter_t cf = f->color_filter;
      uint32_t yuv_byte_order = f->yuv_byte_order;
      // PRE7:  uint32_t data_depth      = f->data_depth;
      uint32_t data_depth      = f->data_depth;
      // uint32_t stride         = f->stride;
      //dc1394video_mode_t vm   = f->video_mode;
      //uint64_t wholedatasize  = f->total_bytes;
      //uint32_t datasize       = f->image_bytes;
      //uint32_t padding_bytes  = f->padding_bytes;
      //uint32_t bpp            = f->bytes_per_packet;
      //uint32_t ppf            = f->packets_per_frame;
      //uint64_t timestamp      = f->timestamp;
      //uint32_t frames_behind  = f->frames_behind;
      //dc1394camera_t *cam     = f->camera;
      //uint32_t ringbufferid   = f->id;
      //uint64_t allocated_bytes= f->allocated_image_bytes;
  
      if(fmt == formatGray){
        Img8u i(Size(width,height),formatGray);
        dc1394_convert_to_MONO8(data, i.getData(0),width,height,yuv_byte_order, cc, data_depth);
        i.setTime(Time(f->timestamp));
        return i;
      }else if(fmt == formatRGB){
        Img8u i(Size(width,height),formatRGB);
        icl8u *buf = new icl8u[width*height*4];
        dc1394_convert_to_RGB8(data,buf,width,height,yuv_byte_order, cc, data_depth);
        interleavedToPlanar(buf,&i);
        i.setTime(Time(f->timestamp));
        return i;
      }else{
        printf("extract_image unsupported format !\n");
      }
  
      return Img8u();
    }

    // }}}
    
    void extract_image_to(dc1394video_frame_t *f, ImgBase **ppoDst, vector<icl8u> &rgbInterleavedBuffer){
      // {{{ open

      ICLASSERT_RETURN( f );
      unsigned char *data = f->image;
      int width = (int)f->size[0];
      int height = (int)f->size[1];
      dc1394color_coding_t cc = f->color_coding;
      //dc1394color_filter_t cf = f->color_filter;
      uint32_t yuv_byte_order = f->yuv_byte_order;
      uint32_t data_depth      = f->data_depth;
      //uint32_t stride         = f->stride;
      //dc1394video_mode_t vm   = f->video_mode;
      //uint64_t wholedatasize  = f->total_bytes;
      //uint32_t datasize       = f->image_bytes;
      //uint32_t padding_bytes  = f->padding_bytes;
      //uint32_t bpp            = f->bytes_per_packet;
      //uint32_t ppf            = f->packets_per_frame;
      //uint64_t timestamp      = f->timestamp;
      //uint32_t frames_behind  = f->frames_behind;
      //dc1394camera_t *cam     = f->camera;
      //uint32_t ringbufferid   = f->id;
      //uint64_t allocated_bytes= f->allocated_image_bytes;


      if( is_firefly_color(f->camera) ){
        bool want_gray_images = false;
        if(want_gray_images){
          ensureCompatible(ppoDst,depth8u, Size(width,height),formatGray);
          dc1394_convert_to_MONO8(data, (*ppoDst)->asImg<icl8u>()->getData(0),width,height,yuv_byte_order, cc, data_depth);
        }
        else{
          ensureCompatible(ppoDst,depth8u, Size(width,height),formatRGB);
          if((int)rgbInterleavedBuffer.size() < width*height*3){
            rgbInterleavedBuffer.resize(width*height*3);
          }
          icl8u *buf = &(rgbInterleavedBuffer[0]);
      
      
          dc1394_bayer_decoding_8bit(data,buf,width,height,DC1394_COLOR_FILTER_GBRG,DC1394_BAYER_METHOD_BILINEAR);
          //      dc1394_convert_to_RGB8(data,buf,width,height,yuv_byte_order, cc, data_depth);
          interleavedToPlanar(buf,(*ppoDst)->asImg<icl8u>());
        }
      }
      else if( is_firefly_mono(f->camera) ){
        ensureCompatible(ppoDst,depth8u, Size(width,height),formatGray);
        dc1394_convert_to_MONO8(data, (*ppoDst)->asImg<icl8u>()->getData(0),width,height,yuv_byte_order, cc, data_depth);
      }
      else{
        ERROR_LOG("unsupported camera!");
      }
      if(ppoDst && *ppoDst){
        (*ppoDst)->setTime(Time(f->timestamp));
      }
    }

    // }}}
    
    void extract_image_to_gray(dc1394video_frame_t *f,
                                const DCDevice &dev, 
                                ImgBase **ppoDst, 
                                const Size &desiredSizeHint, 
                                depth desiredDepthHint,
                                std::vector<icl8u> &dataBuffer,
                               dc1394bayer_method_t bayerMethod){
      // {{{ open
      DEBUG_LOG("color_filter:" << f->color_filter);
      (void)desiredDepthHint;
      Size frameSize(f->size[0],f->size[1]);
      ensureCompatible(ppoDst,depth8u,frameSize,formatGray);
      dc1394_convert_to_MONO8(f->image, 
                              (*ppoDst)->asImg<icl8u>()->getData(0),
                              frameSize.width,
                              frameSize.height,
                              f->yuv_byte_order, 
                              f->color_coding,
                              f->data_depth);
      //  old }
      if(ppoDst && *ppoDst){
        (*ppoDst)->setTime(Time(f->timestamp));
      }
    }

    // }}}
    
    void extract_image_to_rgb(dc1394video_frame_t *f,
                              const DCDevice &dev, 
                              ImgBase **ppoDst, 
                              const Size &desiredSizeHint, 
                              depth desiredDepthHint,
                              std::vector<icl8u> &dataBuffer,
                              dc1394bayer_method_t bayerMethod){
      
      // {{{ open
      DEBUG_LOG("color_filter:" << f->color_filter);
      (void)desiredDepthHint;
      Size frameSize(f->size[0],f->size[1]);
      ensureCompatible(ppoDst,depth8u, frameSize,formatRGB);
     
      if(dev.needsBayerDecoding()){
      //      if(f->color_filter){ unfortunately this is not set for some cams??
        if((int)dataBuffer.size() < frameSize.getDim()*3){
            dataBuffer.resize(frameSize.getDim()*3);
        }
        dc1394_bayer_decoding_8bit(f->image,
                                   dataBuffer.data(),
                                   frameSize.width,
                                   frameSize.height,
                                   dev.getBayerFilterLayout(),
                                   //f->color_filter,
                                   bayerMethod);
        interleavedToPlanar(dataBuffer.data(),(*ppoDst)->asImg<icl8u>());
      }else{
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
                          const DCDevice &dev, 
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
          extract_image_to_gray(f,dev,ppoDst,desiredSizeHint,desiredDepthHint,dataBuffer,bayerMethod);
          break;
        default: // rgb is returned .. and converted lateron
          extract_image_to_rgb(f,dev,ppoDst,desiredSizeHint,desiredDepthHint,dataBuffer,bayerMethod);
          break;
      }
    }

    // }}}

    void extract_image_to_2(dc1394video_frame_t *f,
                            const DCDevice &dev, 
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
    
      if(dev.needsBayerDecoding()){
        ensureCompatible(ppoDst,depth8u, Size(width,height),formatRGB);
        if((int)dataBuffer.size() < width*height*3){
          dataBuffer.resize(width*height*3);
        }
        dc1394_bayer_decoding_8bit(data,dataBuffer.data(),width,height,dev.getBayerFilterLayout(),bayerMethod);
        //      dc1394_convert_to_RGB8(data,buf,width,height,yuv_byte_order, cc, data_depth);
        interleavedToPlanar(dataBuffer.data(),(*ppoDst)->asImg<icl8u>());
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
                              const DCDevice &dev, 
                              const Size &desiredSizeHint, 
                              format desiredFormatHint,
                              depth desiredDepthHint){
      // {{{ open

      if(desiredDepthHint != depth8u) return false;
      if(desiredFormatHint == formatGray && f->data_depth == 8) return true;
      if(desiredSizeHint != Size(f->size[0],f->size[1])) return false;
      if(desiredFormatHint != formatRGB) return false;
      return true;
    }

    // }}}


    void grab_frame(dc1394camera_t* c, ImgBase **image){
      // {{{ open

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
    }

    // }}}


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
