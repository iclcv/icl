#include <ICLRegionDetectorBlob.h>
#include <ICLRegionDetectorScanLine.h>
#include <ICLMacros.h>
#include <ICLArray.h>
#include <math.h>
#include <limits>


using namespace std;

namespace icl{
  namespace regiondetector{
    
    RegionDetectorBlob::RegionDetectorBlob(RegionDetectorBlobPart *r){
      // {{{ open

      m_poPixels = new ScanLineList(0);
      if(r){
        fetch(r);
      }
      m_bSizeDirty = true;
      m_bCOGDirty = true;
      m_bBBDirty = true;
      m_bPCADirty = true;
      m_bBoundaryDirty = true;
      m_bFormFactorDirty = true;

      m_oPCA.resize(4);
    }

    // }}}
   
    RegionDetectorBlob::~RegionDetectorBlob(){
      // {{{ open

      delete m_poPixels;
    }

    // }}}

    int RegionDetectorBlob::getSize(){
      // {{{ open

      calculateSize();
      return m_iSize;
    }

    // }}}
    
    unsigned char *RegionDetectorBlob::getImageDataPtr(){
      // {{{ open

      return m_poPixels->size() ? (*m_poPixels)[0]->getImageDataPtr() : 0;
    }

    // }}}
    
    void RegionDetectorBlob::show(){
      // {{{ open

      Point p = getCOG();
      printf("RegionDetectorBlob::x=%d y=%d  size=%d  npixellines = %d \n",p.x,p.y,getSize(),m_poPixels->size());
    }

    // }}}
   
    void RegionDetectorBlob::update(RegionDetectorBlobPart *r){
      // {{{ open

      m_bSizeDirty = true;
      m_bCOGDirty = true;
      m_bBBDirty = true;
      m_bPCADirty = true;
      m_bBoundaryDirty = true;
      m_bFormFactorDirty = true;
      m_poPixels->clear();
      fetch(r);
    }

    // }}}

    void RegionDetectorBlob::clear(){
      // {{{ open

      m_bSizeDirty = true;
      m_bCOGDirty = true;
      m_bBBDirty = true;
      m_bPCADirty = true;
      m_bBoundaryDirty = true;
      m_bFormFactorDirty = true;
      m_poPixels->clear();
    }

    // }}}

    void RegionDetectorBlob::getFeatures(Point &center, icl8u &val){
      // {{{ open

      center = getCOG();
      val = getVal();
    }

    // }}}

    void RegionDetectorBlob::getAllFeatures(Point &center, icl8u &val, Rect &bb,
                                            float &l1, float &l2, float&arc1, float &arc2){
      // {{{ open

      calculateSizeBBAndPCA();
      l1 = m_oPCA[0];      
      l2 = m_oPCA[1];      
      arc1 = m_oPCA[2];      
      arc2 = m_oPCA[3];      
      
      center = getCOG();
      val = getVal();
      bb = getBoundingBox();
    }

    // }}}
  
    void RegionDetectorBlob::fetch(RegionDetectorBlobPart *r){
      // {{{ open

      if(r->ps){
        for(ScanLineList::iterator it = r->ps->begin();it!= r->ps->end();++it){
          m_poPixels->push_back(*it);
        }
      }
      if(r->rs){
        for(BlobPartList::iterator it = r->rs->begin();it!= r->rs->end();++it){
          fetch(*it);
        }
      }
    }

    // }}}

    inline void RegionDetectorBlob::getStartPixel(int &xStart,int &yStart){
      // {{{ open
      yStart = numeric_limits<int>::max();

      for(ScanLineList::iterator it = m_poPixels->begin();it!= m_poPixels->end();it++){
        yStart= min((*it)->y(),yStart);
      }
      xStart = 10000000;
      for(ScanLineList::iterator it = m_poPixels->begin();it!= m_poPixels->end();it++){
        if((*it)->y() == yStart){
          xStart = min((*it)->getStart(),xStart);
        }
      }      
    }

    // }}}
    
    unsigned char RegionDetectorBlob::getVal(){
      // {{{ open

      return m_poPixels->size() ? (*m_poPixels)[0]->val() : 0;
    }

    // }}}

    const Point &RegionDetectorBlob::getCOG(){
      // {{{ open
      calculateCOG();
      return m_oCOG;
    }

    // }}}
    
    const Rect &RegionDetectorBlob::getBoundingBox(){
      // {{{ open

      calculateBoundingBox();
      return m_oBB;
    }

    // }}}

    const vector<float> &RegionDetectorBlob::getPCAInfo(){
      // {{{ open

      calculatePCA();
      return m_oPCA;
    }    

    // }}}

    int  RegionDetectorBlob::getBoundaryLength(const Size &imageSize){
      // {{{ open

      calculateBoundaryLength(imageSize);
      return m_iBoundaryLength;
    }

    // }}}
    
    const vector<Point> &RegionDetectorBlob::getBoundary(const Size &imageSize){
      // {{{ open

      calculateBoundary(imageSize);
      return m_oBoundary;
    }

    // }}}

    float RegionDetectorBlob::getFormFactor(const Size &imageSize){
      // {{{ open

      calculateFormFactor(imageSize);
      return m_fFormFactor;
    }

    // }}}   
 
    /// retuns sum(i=0..k) i*i
    double eval_sum_of_squares(double k){
      return k*(k+1)*(2*k+1)/6.0;
    }    
    // returns sum(i=0..k) i
    double eval_sum(double k){
      return k*(k+1)/2.0;
    }
    
    void RegionDetectorBlob::calculateCOG(){
      // {{{ open
      if(!m_bCOGDirty) return;
      
      m_oCOG = Point(0,0);
      register float xBuf(0);
      register int nPix(0),s(0);
      
      for(ScanLineList::iterator it = m_poPixels->begin();it!= m_poPixels->end();it++){
        RegionDetectorScanLine *l = *it;
        s = l->size();
        xBuf += l->x() * s;
        m_oCOG.y += l->y() * s;
        nPix += s;
      }
      
      m_oCOG.x = (int)round(xBuf/nPix);
      m_oCOG.y /= nPix;
      m_bCOGDirty = false;
    }

    // }}}
    
    void RegionDetectorBlob::calculateSize(){
      // {{{ open

      if(!m_bSizeDirty) return;
      m_iSize = 0;
      for(ScanLineList::iterator it = m_poPixels->begin(); it!= m_poPixels->end(); it++){
        m_iSize += (*it)->size();
      }
      m_bSizeDirty = false;
    }

    // }}}
    
    void RegionDetectorBlob::calculateBoundingBox(){
      // {{{ open
      if(!m_bBBDirty) return;
      register int minX = numeric_limits<int>::max();
      register int minY = minX;
      register int maxX = numeric_limits<int>::min();
      register int maxY = maxX;
      for(ScanLineList::iterator it = m_poPixels->begin(); it!= m_poPixels->end(); it++){
        minX = min(minX,(*it)->getStart());
        maxX = max(maxX,(*it)->getEnd());
        minY = min(minY,(*it)->y());
        maxY = max(maxY,(*it)->y());
      }
      m_oBB = Rect(minX,minY,maxX-minX,maxY-minY);
      m_bBBDirty = false;
    }
    // }}}
    
    void RegionDetectorBlob::calculatePCA(){
      // {{{ open

      if(!m_bPCADirty) return;
      register int x,end,len;
      register double y;
    
      register double avgY(0),avgX(0),avgXX(0),avgXY(0),avgYY(0);
      register int nPts(0);
      for(ScanLineList::iterator it = m_poPixels->begin(); it!= m_poPixels->end(); it++){
        x = (*it)->getStart();
        y = (*it)->y();
        end = (*it)->getEnd();
        len = (*it)->size();

        nPts += len;
      
        avgX += len*( (*it)->x() );
        avgY += len*y;

        /// XX = sum(k=x..end) k²
        avgXX += eval_sum_of_squares(end) - eval_sum_of_squares(x);
      
        /// YY = sum(k=x..end) y²
        avgYY += len*y*y;
      
        /// XY = sum(k=x..end) xy = y * sum(k=x..end) k 
        avgXY += y * ( eval_sum(end) - eval_sum(x) );
      }
      avgX/=nPts;
      avgY/=nPts;
      avgXX/=nPts;
      avgYY/=nPts;
      avgXY/=nPts;

      if(m_bCOGDirty){
        m_oCOG = Point((int)round(avgX),(int)round(avgY));
        m_bCOGDirty = false;
      }

      double fSxx = avgXX - avgX*avgX;
      double fSyy = avgYY - avgY*avgY;
      double fSxy = avgXY - avgX*avgY;
      
      double fP = 0.5*(fSxx+fSyy);
      double fD = 0.5*(fSxx-fSyy);
      fD = sqrt(fD*fD + fSxy*fSxy);
      double fA  = fP + fD;
      m_oPCA[0] = (float)(2*sqrt(fP + fD));
      m_oPCA[1] = (float)(2*sqrt(fP - fD));
      m_oPCA[2] = (float)atan2(fA-fSxx,fSxy);
      m_oPCA[3] = m_oPCA[2]+M_PI/2.0;
    
      m_bPCADirty = false;
    }

    // }}}
    
    void RegionDetectorBlob::calculateBoundary(const Size &imageSize){
      // {{{ open

      if(!m_bBoundaryDirty) return;
      int w = imageSize.width;
      int h = imageSize.height;
      m_oBoundary.clear();
      int xStart,yStart;
      getStartPixel(xStart,yStart);
    
      if(getSize() == 1){
        m_oBoundary.push_back(Point(xStart,yStart));
        return;
      }
      unsigned char v = getVal();
      unsigned char *data = getImageDataPtr();
    
    
    
      /***********************************
          dirs for 9er neighbourhood:  
          5 6 7        -1-w  -w   1-w 
          4 c 0         -1    0   1    
          3 2 1        w-1    w   1+w   

          //                    0      1  2  3  4  5  6   7   8     9  10 11 12 13 14 15 
          static int dirs[] = {-1-w , -w,1-w,1,1+w,w,w-1,-1,-1-w , -w,1-w,1,1+w,w,w-1,-1};
          //                    5      6  7  0  1  2  3   4   5     6  7  0  1  2  3   4
          static int xdirs[] = { -1, 0, 1, 1, 1, 0,-1,-1,-1, 0, 1, 1, 1, 0,-1,-1 };
          static int ydirs[] = { -1,-1,-1, 0, 1, 1, 1, 0,-1,-1,-1, 0, 1, 1, 1, 0 };
          static int jumps[] = {  6, 7, 0, 1, 2, 3, 4, 5, 6,7, 0, 1, 2, 3, 4 };
 
          *************************************/
    
      // dirs:
      //     3          -w 
      //   2 c 0     -1  0  1
      //     1           w
      //
      //
    
      //                      0  1  2  3  4  5  6  7  
      static int dirs[] =   {-w, 1, w,-1,-w, 1, w,-1 };
      //                      3  0  1  2  3  0  1  2 

      static int xdirs[] = {  0, 1, 0,-1, 0, 1, 0,-1 };
      static int ydirs[] = { -1, 0, 1, 0,-1, 0, 1, 0 };
      static int jumps[] = {  3, 0, 1, 2, 3, 0, 1, 2 };
    
      register int dirIdx=0;
    
      register unsigned char *pStart = data+xStart+yStart*w;
      register unsigned char *p = pStart;
      register int x=xStart;
      register int y=yStart;
    
      register unsigned char *cp(0);
      register int cx(0), cy(0);
      register bool posValid(false);

      register unsigned char *pBreak(0);
      register int dirIdxBreak(0);
   
      /// seach 2nd pixel -> criterion for end loop
      m_oBoundary.push_back(Point(x,y));
      do{
        cx = x+xdirs[dirIdx];
        cy = y+ydirs[dirIdx];
        cp = p+dirs[dirIdx];
        posValid = cx >= 0 && cx < w && cy >= 0 && cy < h;
        dirIdx++;
      }while(!posValid || *cp!=v);
      p = cp;
      x = cx;
      y = cy;
      pBreak = p;
      dirIdx = jumps[dirIdx-1];   
      dirIdxBreak = dirIdx;
   
    
      do{
        m_oBoundary.push_back(Point(x,y));
        do{
          cx = x+xdirs[dirIdx];
          cy = y+ydirs[dirIdx];
          cp = p+dirs[dirIdx];
          posValid = cx >= 0 && cx < w && cy >= 0 && cy < h;
          dirIdx++;
        }while(!posValid || *cp!=v);
        p = cp;
        x = cx;
        y = cy;
        dirIdx = jumps[dirIdx-1];
      }while ( (p != pBreak) || (dirIdx != dirIdxBreak) );
   
      m_oBoundary.pop_back();
    
      if(m_bBoundaryLengthDirty){
        m_iBoundaryLength = (int)m_oBoundary.size();
        m_bBoundaryLengthDirty = false;
      }
      m_bBoundaryDirty = false;
    }

    // }}}
    
    void RegionDetectorBlob::calculateBoundaryLength(const Size &imageSize){
      // {{{ open

      if(!m_bBoundaryLengthDirty) return;
      int w = imageSize.width;
      int h = imageSize.height;
      int xStart,yStart;
      getStartPixel(xStart,yStart);
      m_iBoundaryLength = 0;
      
      if(getSize() == 1){
        m_iBoundaryLength = 1;
        m_bBoundaryLengthDirty = false;
        return;
      }
      unsigned char v = getVal();
      unsigned char *data = getImageDataPtr();
    
      // dirs:
      //     3          -w 
      //   2 c 0     -1  0  1
      //     1           w
      //
      //
    
      //                      0  1  2  3  4  5  6  7  
      static int dirs[] =   {-w, 1, w,-1,-w, 1, w,-1 };
      //                      3  0  1  2  3  0  1  2 

      static int xdirs[] = {  0, 1, 0,-1, 0, 1, 0,-1 };
      static int ydirs[] = { -1, 0, 1, 0,-1, 0, 1, 0 };
      static int jumps[] = {  3, 0, 1, 2, 3, 0, 1, 2 };
    
      register int dirIdx=0;
    
      register unsigned char *pStart = data+xStart+yStart*w;
      register unsigned char *p = pStart;
      register int x=xStart;
      register int y=yStart;
    
      register unsigned char *cp(0);
      register int cx(0), cy(0);
      register bool posValid(false);
  
      register unsigned char *pBreak(0);
      register int dirIdxBreak(0);
   
      /// seach 2nd pixel -> criterion for end loop
      m_iBoundaryLength++;
      do{
        cx = x+xdirs[dirIdx];
        cy = y+ydirs[dirIdx];
        cp = p+dirs[dirIdx];
        posValid = cx >= 0 && cx < w && cy >= 0 && cy < h;
        dirIdx++;
      }while(!posValid || *cp!=v);
      p = cp;
      x = cx;
      y = cy;
      pBreak = p;
      dirIdx = jumps[dirIdx-1];   
      dirIdxBreak = dirIdx;
   
    
      do{
        m_iBoundaryLength++;
        do{
          cx = x+xdirs[dirIdx];
          cy = y+ydirs[dirIdx];
          cp = p+dirs[dirIdx];
          posValid = cx >= 0 && cx < w && cy >= 0 && cy < h;
          dirIdx++;
        }while(!posValid || *cp!=v);
        p = cp;
        x = cx;
        y = cy;
        dirIdx = jumps[dirIdx-1];
      }while ( (p != pBreak) || (dirIdx != dirIdxBreak) );
    
      m_iBoundaryLength--;
    
      m_bBoundaryLengthDirty = false;    

      // unsigned char *dataEnd = data+w*h;
      // dirs   
      //    5 6 7        -1-w  -w   1-w 
      //    4 c 0         -1    0   1    
      //    3 2 1        w-1    w   1+w   
      //
    
      //                    0      1  2  3  4  5  6   7   8     9  10 11 12 13 14 15 
      // static int dirs[] = {-1-w , -w,1-w,1,1+w,w,w-1,-1,-1-w , -w,1-w,1,1+w,w,w-1,-1};
      //                    5      6  7  0  1  2  3   4   5     6  7  0  1  2  3   4
      // static int xdirs[] = { -1, 0, 1, 1, 1, 0,-1,-1,-1, 0, 1, 1, 1, 0,-1,-1 };
      // static int ydirs[] = { -1,-1,-1, 0, 1, 1, 1, 0,-1,-1,-1, 0, 1, 1, 1, 0 };
      // static int jumps[] = {  6, 7, 0, 1, 2, 3, 4, 5, 6,7, 0, 1, 2, 3, 4 };
      // register int dirIdx=1;

    }

    // }}}
    
    void RegionDetectorBlob::calculateFormFactor(const Size &imageSize){
      // {{{ open

      if(!m_bFormFactorDirty) return;
    
      int U = getBoundaryLength(imageSize);
      int A = getSize();
      m_fFormFactor =  float(U*U)/(4*M_PI*A);
      m_bFormFactorDirty = false;
    }

    // }}}
    
    void RegionDetectorBlob::calculateSizeBBAndPCA(){
      // {{{ open

      calculateSize();
      calculateBoundingBox();
      calculatePCA();
    }

    // }}}
  
  }
}

