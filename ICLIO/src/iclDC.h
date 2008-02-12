#include <dc1394/control.h>
#include <dc1394/conversions.h>
#include <iclTypes.h>
#include <iclSize.h>
#include <string>
#include <vector>



namespace icl{
  /** \cond */
  class ImgBase;
  class DCDevice;
  class DCDeviceOptions;
  /** \endcond */
  
  /// internal used namespace for libdc1394 dependent help functions and classes \ingroup DC_G
  namespace dc{
    
    /// translate a dc1394video_mode_t into a string representation \ingroup DC_G
    std::string to_string(dc1394video_mode_t vm);

    /// translate a dc1394framerate_t into a string representation \ingroup DC_G
    std::string to_string(dc1394framerate_t fr);

    /// translate a dc1394color_filter_t into a string representation \ingroup DC_G
    std::string to_string(dc1394color_filter_t f);
    
    /// translate a dc1394bayer_method_t into a string representation \ingroup DC_G
    std::string to_string(dc1394bayer_method_t bm);
    
    /// translate a dc1394feature_t into a string representation \ingroup DC_G
    std::string to_string(dc1394feature_t f);

    /// translate a dc1394video_mode_t from a string representation \ingroup DC_G
    dc1394video_mode_t videomode_from_string(const std::string &s);
    
    /// translate a dc1394framerate_t from a string representation \ingroup DC_G
    dc1394framerate_t framerate_from_string(const std::string &s);
    
    /// translate a dc1394bayer_method_t from a string representation \ingroup DC_G
    dc1394bayer_method_t bayermethod_from_string(const std::string &s);

    /// translate a dc1394feature_t from a string representation \ingroup DC_G
    dc1394feature_t feature_from_string(const std::string &s);
    
    /// returns a list of all currently implemented dc1394 features \ingroup DC_G
    const std::vector<std::string> &getListOfAllFeatures();
    
    /// checks if a camera has the firefly mv color model id \ingroup DC_G
    /** @param c camera to check 
        @return string(c->model) == "Firefly MV FFMV-03MTC"
    */
    bool is_firefly_color(dc1394camera_t* c);

    /// checks if a camera has the firefly mv mono model id \ingroup DC_G
    /** @param c camera to check 
        @return string(c->model) == "Firefly MV FFMV-03MTM"
    */
    bool is_firefly_mono(dc1394camera_t* c);
    
    /// (TODO) sets up a camera to some useful defaults dependent on its model id \ingroup DC_G
    void initialize_dc_cam(dc1394camera_t *c, int nDMABuffers, DCDeviceOptions *options);

    /// stops the capturing and streaming process \ingroup DC_G
    void release_dc_cam(dc1394camera_t *c);
    
    /// ensures iso streaming of the given camera to have the given state (on or off) \ingroup DC_G
    void set_streaming(dc1394camera_t* c, bool on);
    
    /// gets the newest frame from a given camera \ingroup DC_G
    /** Internally this function flushes the ring buffer, queues a brand new
        frame, and then waits for it to be filled with the newest frame.
        <b>NOTE:</b>This is not the procedure that is used by the DCGrabber!
    */
    dc1394video_frame_t *get_newest_frame(dc1394camera_t* c);
    
    /// gets a new frame from the given camera \ingroup DC_G
    /** The camera MUST be initialized, running and streaming */
    dc1394video_frame_t *get_a_frame(dc1394camera_t* c);
    
    /// extracts a dc1394video_frame_t struct into a given ImgBase* \ingroup DC_G
    /** Yet, the given frame must be grabbed from a firefly camera.

        <b>TODO:</b> Adapt this function to be more flexible: 
        - additional parameters could be:
          - a destination format (e.g. to force gray even when grabbing from an RGB device)
          - bayer interpolation mode (nearest, simple, bilin, hq, and <b>subsample</b>
          - ...
        - additional features should be:
          - better handling of different source cameras (f->cam)
          - handling of the above parameters
          - ...
        
        
        @param f given video frame to convert
        @param ppoDst destination images (adapted to icl8u,size of the given frame, and 
                      color format, which depend on the dc1394camera_t, which is associated
                      with it)
        @param rgbInterleavedBuffer internal used static data buffer, which is resized if
                                    necessary and only for rgb images. 

        */
    void extract_image_to(dc1394video_frame_t *f, ImgBase **ppoDst, std::vector<icl8u> &rgbInterleavedBuffer);
    
    /// converts a grabbed frame \ingroup DC_G
    /** the desired params are only hints, which can - but do not have to - be
        regarded !! */
    void extract_image_to(dc1394video_frame_t *f,
                          const DCDevice &dev, 
                          ImgBase **ppoDst, 
                          const Size &desiredSizeHint, 
                          format desiredFormatHint,
                          depth desiredDepthHint,
                          std::vector<icl8u> &dataBuffer,
                          dc1394bayer_method_t bayerMethod);

    /// determins if the desired parameters can be fullfilled by extract_image_to(..) \ingroup DC_G
    /** TODO !!*/
    bool can_extract_image_to(dc1394video_frame_t *f,
                              const DCDevice &dev, 
                              const Size &desiredSizeHint, 
                              format desiredFormatHint,
                              depth desiredDepthHint);

    
    /// utility function to grab a single frame from a camera into a given ImgBase** \ingroup DC_G
    /** <b>DEPRECATED</b>*/
    void grab_frame(dc1394camera_t* c, ImgBase **image);

    /// creates a signal handler for the SIGINT signal \ingroup DC_G
    /** The signal handler will enshure, that the SIGINT-signal,
        which is send when CTRL+C is pressed to kill a the programm,
        is caught. The new signal handler function ensures, that
        all camera threads are stopped before the programm exits */
    void install_signal_handler();
    
    /// internally used bayer conversion function
    void bayer2gray(icl8u *src, icl8u*dst, int w, int h);
    
    /// since rc9 of libdc, a libary context was introduced
    dc1394_t *get_static_context();

    /// (since rc 9 of libdc) releases the static dc-context \ingroup DC_G
    /** E.g. when when trying to access a dc-devices using unicap, after the 
        device was opened by a DCGrabber (note, that getDeviceList must be called)
    **/
    void free_static_context();
  }
}
    
