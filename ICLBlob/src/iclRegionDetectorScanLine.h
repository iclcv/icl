#ifndef PIXELLINE_H
#define PIXELLINE_H

#include <iclRegionDetectorTypes.h>

namespace icl{
  namespace regiondetector{
    /// Utility class for a scan line used in the RegionDetector class \ingroup G_RD
    /**
        The RegionDetectorScanLine class represents a set of pixels that are detected in the same line
        of an image, and that each has the same value. 
        It described by 
        - the corresponding image data (as unsigned char*)
        - the width (in pixels) of the corresponding image
        - the line of the image, where this RegionDetectorScanLine was detected
        - the x-position where the RegionDetectorScanLine starts
        - the x-position where the RegionDetectorScanLine ends
    */
    class RegionDetectorScanLine{
      public:
      /// Creates a new RegionDetectorScanline with given parameters
      inline RegionDetectorScanLine(int line=0, int start=0, int end=0, int iImageWidth=-1, unsigned char *pucImageData=0):
        line(line),start(start),end(end),imagewidth(iImageWidth),imagedata(pucImageData){
      };
      /// Destructor
      inline ~RegionDetectorScanLine(){}
      
      /// x-center of the scan line
      inline float x(){return ((float)start+(float)end)/2.0;}

      /// y-location of the scan line
      inline int y(){return line;}

      /// current pixel value of the scan line 
      inline unsigned char val(){return imagedata[imagewidth*line+start];}

      /// length of the scan line
      inline int size(){ return end-start+1; }

      /// x start index
      inline int getStart(){return start;}

      /// x end index
      inline int getEnd(){return end;}
      
      /// resets the scan line internally to new parameters
      inline void update(int line, int start, int end, int iImageWidth,unsigned char *pucImageData){
        this->line = line;
        this->start = start;
        this->end = end;
        this->imagewidth = iImageWidth;
        this->imagedata = pucImageData;
      }
      /// draws this scanline into an image
      inline void drawIntoImage(unsigned char *pucDst){
        unsigned char *pucStart = pucDst+line*imagewidth+start;
        unsigned char *pucEnd = pucDst+line*imagewidth+end;
        while(pucStart < pucEnd){
          *pucStart++=255;
        }
      }
      /// returns the associated image data pointer
      inline unsigned char *getImageDataPtr() { return imagedata; }

      protected:
      int line; ///!< y-location of this scan line
      int start;///!< x start index of this scan line
      int end;  ///!< x end index of this scan line
      int imagewidth; /// !< associated image width
      unsigned char *imagedata;/// !< associated image data
    };
  }
}
#endif
