#include <ICLRegionDetectorTypes.h>
#ifndef PIXELLINE_H
#define PIXELLINE_H

namespace icl{
  namespace regiondetector{
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
      inline RegionDetectorScanLine(int line=0, int start=0, int end=0, int iImageWidth=-1, unsigned char *pucImageData=0):
        line(line),start(start),end(end),imagewidth(iImageWidth),imagedata(pucImageData){

        //s_iReferenceCounter++;
      };
      inline ~RegionDetectorScanLine(){
        //s_iReferenceCounter--;
      }
      
      inline float x(){return ((float)start+(float)end)/2.0;}
      inline int y(){return line;}
      inline unsigned char val(){return imagedata[imagewidth*line+start];}
      inline int size(){ return end-start+1; }
      inline int getStart(){return start;}
      inline int getEnd(){return end;}
      
      inline void update(int line, int start, int end, int iImageWidth,unsigned char *pucImageData){
        this->line = line;
        this->start = start;
        this->end = end;
        this->imagewidth = iImageWidth;
        this->imagedata = pucImageData;
      }
      inline void drawIntoImage(unsigned char *pucDst){
        unsigned char *pucStart = pucDst+line*imagewidth+start;
        unsigned char *pucEnd = pucDst+line*imagewidth+end;
        while(pucStart < pucEnd){
          *pucStart++=255;
        }
      }
     
      static void showReferenceCounter(){
        printf("RegionDetectorScanLine #=%d \n",s_iReferenceCounter);
      }
      inline unsigned char *getImageDataPtr() { return imagedata; }
      protected:
      int line,start,end; 
      int imagewidth;
      unsigned char *imagedata;
      
      private:
      static int s_iReferenceCounter;
    };
  }
}
#endif
