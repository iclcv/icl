#ifndef SVS_H
#define SVS_H

/*
  SVS.h

  Written by: Andre Justus (2007)
              University of Bielefeld
              AG Neuroinformatik
              ajustus@techfak.uni-bielefeld.de
*/



#include <iclImg.h>
#include <iclImgBase.h> 
//#include "/vol/vision/SVS/4.2/src/svsclass.h"
#include <svsclass.h>
namespace icl{

/// Wrapper-Class for SVS (Small Vision System)
/**
  The SVS class provides functionality for getting disparity Information out of a stereo-image pair.
  Therefor two grayscale icl8u images (left-img, right-img) will be loaded and two icl16s images can be calculated: 
- a disparity image, having high intensity, where the disparity is high, and having low intensity, where the disparity is low.
- a confidence image, showing how significant the disparity is.

Both images have the same size and the same orientation as the input images.
The left input image it the referenceimage.


In order to get a usefull result you have to callibrate the cameras externally by the provided callibration binary (smallvcal), save the calibraion information to a File and load it with the load_calibration() function.

Benchmarking results

corrsize - corr window size pixels:15
thresh - confidence threshold:7
unique - uniqueness threshold:10
ndisp - number of pixel:32
dpp - disparities per pixel:16
offx - Horopter offset:2

ix - Subimage start column:0
iy - Subimage start row:0
width - Subimage width:320
height - Subimage height:240
-- Pentium-M (Celeron) 1,6 Ghz 
DEBUG OFF
~8 sec for 1000 iterations => 8ms per operation => 125hz

*/
  class SVS{
    public:
      /// parameterlist
      enum svsparam{
        corrsize,
        confidence,
        unique,
        ndisp,
        dpp,
        offx,
        ix,
        iy,
        width,
        height
      };
			/// initiates the necessary SVS Objects.
      SVS();
      /// prints the current SVS parameters
      void printvars();
      /// Makes a stereoimage-pair available for SVS
      /**
        @param lim left image
        @param rim right image
      */
      void Load(const Img8u* lim, const Img8u* rim);
      /// Makes a stereoimage-pair available for SVS that represents only a subimage of the cameraimage
      /**
        @param lim left image
        @param rim right image
        @param offset the offset between the overgiven images (lim, rim) and the original camera image, used to have the right internal parameters (lens distortion)
      */
      void Load(const Img8u* lim,const Img8u* rim, Point offset);
      /// Cuts a stereoimage-pair and makes it available for SVS
      /**
        @param lim left image
        @param rim right image
        @param offset the offset for the cutted image
				@param Size the size for the cutted image
      */
      void Load_cut(const Img8u* lim,const Img8u* rim, Point offset,Size iDim);

      /// Calculates disparity and confidence images
      void do_stereo();

      /// Returns the disparity image
      /**
        @returns the disparity image
      */
      Img16s* get_disparity();

      /// Returns the disparity image
      /**
        @returns the disparity image
      */
      Img16s* get_confidence();

      /// Loads a calibration file
      /**
        normally, it has .ini as suffix
        @param filename  the filename where the calibration file is stored
      */
      void load_calibration(char *filename);

      ///Sets a SVS parameter
      /**
        @param p @see svsparam
        @param value the value, the parameter will be set to
      */
      void setParam(svsparam p, int value);

      ///Gets a SVS parameter
      /**
        @param p @see svsparam
        @returns the value of the parameter
      */
      int getParam(svsparam p);
    private:
      svsStereoImage* m_si;
      format m_fmt;
      Size m_size;
      Img16s *m_di;
      svsStoredImages *m_svsI;
      svsStereoProcess *m_svsP;
  }; //class
} // namespace icl
#endif
