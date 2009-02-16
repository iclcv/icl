#ifdef HAVE_XCF
#ifndef ICL_XCF_GRABBER_BASE_H
#define ICL_XCF_GRABBER_BASE_H

#include <iclGrabber.h>
#include <string>
#include <xmltio/Location.hpp>
#include <xcf/CTU.hpp>
#include <iclBayer.h>

namespace icl {

  

   /// Grabber to access XCF Image Server and XCF Publisher \ingroup GRABBER_G
   /** XCFGrabberBase serves as a base class to XCFServerGrabber and 
       XCFPublisherGrabber. 
                  1GBit           100MBit
       Server     48 Hz           10 Hz
       Publisher  69 Hz           12 Hz
   */
  
   class XCFGrabberBase : public Grabber {
   public:
    
      /// Base constructor
    XCFGrabberBase();

    /// Destructor
    ~XCFGrabberBase();
    
    /// grabbing function  
    /** \copydoc icl::Grabber::grab(ImgBase**)*/
    virtual const ImgBase* grabUD(ImgBase **ppoDst=0);
      
    /// grabbing a whole image set, e.g. from a stereo camera
    /** The vector of images is resized to match the number of grabbed
          images. Existing images in the vector are adapted to the desired
        output depth and parameters. Ownership for all grabbed images goes
        to the caller! 
     */
    void grab (std::vector<ImgBase*>& vGrabbedImages);

    /// just imported from parent class
    Grabber::grab;
  
    /// TODO: use internally
    void setIgnoreDesiredParams(bool flag){
      m_bIgnoreDesired = flag;
    }
    
    /// TODO: use internally
    bool getIgnoreDesiredParams() const { 
      return m_bIgnoreDesired; 
    }
    
    protected:   
    /// retrieve most current image set in provided composite transport unit
    virtual void receive (XCF::CTUPtr& result) = 0;
    
    private:
    void makeOutput (const xmltio::Location& l, ImgBase* poOutput);
      
    XCF::CTUPtr          m_result;
    ImgBase*             m_poSource;
    BayerConverter*	 m_poBayerConverter;
    ImgBase*             m_poBayerBuffer;
    ImgBase*             m_poDesiredParamsBuffer;
    bool                 m_bIgnoreDesired;
  };
  
}

#endif
#endif // HAVE_XCF
