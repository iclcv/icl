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
    
  
    
    /// converts a grabbed frame \ingroup DC_G
    /** the desired params are only hints, which can - but do not have to - be
        regarded !! */
    void extract_image_to(dc1394video_frame_t *f,
                          dc1394color_filter_t bayerLayout, 
                          ImgBase **ppoDst, 
                          const Size &desiredSizeHint, 
                          format desiredFormatHint,
                          depth desiredDepthHint,
                          std::vector<icl8u> &dataBuffer,
                          dc1394bayer_method_t bayerMethod);
    
    /// extracts the given frame (in most appropiate way)
    /** here, no desired params are necessary */
    void extract_image_to_2(dc1394video_frame_t *f,
                            dc1394color_filter_t bayerLayout,
                            ImgBase **ppoDst, 
                            std::vector<icl8u> &dataBuffer,
                            dc1394bayer_method_t bayerMethod);

    /// determins if the desired parameters can be fullfilled by extract_image_to(..) \ingroup DC_G
    /** TODO !!*/
    bool can_extract_image_to(dc1394video_frame_t *f,
                              const Size &desiredSizeHint, 
                              format desiredFormatHint,
                              depth desiredDepthHint);

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

    
    /// sets iso transmission speed for the wrapped dc1394 context
    /** @param mbits mbits value (usefull might be 800 whereby 400 is default)
                     values that do not make sense are ignored.
                     The current libdc library documentation lists the following
                     available speed values: 100, 200, 400, 800, 1600, 3200. But 
                     please note, that 1600 and 3200 are not supported by any camera 
                     yet. These values are only reserved for later technologies
    */
    void set_iso_speed(dc1394camera_t *cam, int mbits);
  }
}
    
