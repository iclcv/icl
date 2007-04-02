#ifndef ICL_Unicap_GRAB_ENGINE_H
#define ICL_Unicap_GRAB_ENGINE_H
#include <iclTypes.h>
#include <string>
#include <unicap.h>
#include <iclImgParams.h>

namespace icl{
  /// Interface class for UnicapGrabEngines
  /** Base implemetation is the UnicapDefaultGrabEngine */
  struct UnicapGrabEngine{
    virtual ~UnicapGrabEngine(){}
    // locks the grabbers image
    virtual void lockGrabber() = 0;

    // unlocks the grabber image
    virtual void unlockGrabber() =0;

    /// returns a converted frame
    virtual void getCurrentFrameConverted(const ImgParams &desiredParams, depth desiredDepth,ImgBase **ppoDst) = 0;

    /// returns an unconverted frame
    virtual const icl8u *getCurrentFrameUnconverted() = 0;

    /// returns whether the grab engine is able to provide given image params
    virtual bool isAbleToProvideParams(const ImgParams &desiredParams, depth desiredDepth)const = 0;
    
    /// retruns whether this engine is able to provide converted frames or not
    virtual bool needsConversion() const = 0;
  };
}

#endif
