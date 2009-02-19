#include <iclRegion.h>
#include <iclRegionDetector.h>
#include <iclMacros.h>
#include <limits>
#include <algorithm>

namespace icl{ 

  struct RegionImpl{
    RegionImpl(icl64f val, const ImgBase *image):pixcount(0),val(val),image(image),bb(0),pcainfo(0),boundary(0),pixels(0){
      scanlines.reserve(100);
    }
    ~RegionImpl(){
      ICL_DELETE(bb);
      ICL_DELETE(pcainfo);
      ICL_DELETE(boundary);
      ICL_DELETE(pixels);
    }
    std::vector<ScanLine> scanlines;
    icl32s pixcount;
    icl64f val;
    Point32f cog;
    const ImgBase *image;
    Rect *bb;
    RegionPCAInfo *pcainfo;
    std::vector<Point> *boundary;
    std::vector<Point> *pixels;
  };

  void RegionImplDelOp::delete_func( RegionImpl* impl){
    ICL_DELETE( impl );
  }
  

  Region::Region(RegionPart *p, int maxSize,icl64f val, const ImgBase *image):
    // {{{ open
    ShallowCopyable<RegionImpl,RegionImplDelOp>(new RegionImpl(val,image)){
    collect(p,maxSize);

    impl->cog.x /= impl->pixcount;
    impl->cog.y /= impl->pixcount;
  }

  // }}}
  
  
  int Region::getSize() const{ 
    return impl->pixcount; 
  }

  icl64f Region::getVal() const { 
    return impl->val; 
  }    

  const Point32f &Region::getCOG() const { 
    return impl->cog;
  }

  const std::vector<ScanLine> &Region::getScanLines() const { 
    return impl->scanlines;
  }

  void Region::collect(RegionPart *p,int maxSize){
    // {{{ open

    //printf("collecting part %p with %d scanlines\n",p,p->scanlines.size());
    for(unsigned int i=0;i<p->scanlines.size();++i){
      ScanLine &sl = p->scanlines[i];
      impl->scanlines.push_back(sl);
      impl->pixcount += sl.len;
      if(impl->pixcount > maxSize) return; /// direct break here
      impl->cog.x += sl.len * ( sl.x + (0.5*(sl.len-1)) );
      impl->cog.y += sl.len * sl.y;
    }
    //    printf("collecting part %p with %d other parts\n",p,p->parts.size());
    for(unsigned int i=0;i<p->parts.size();++i){
      collect(p->parts[i],maxSize);
    }
  }

  // }}}
  
  const Rect & Region::getBoundingBox() const{
    // {{{ open

    if(impl->bb){
      return *impl->bb;
    }
    register int minX = std::numeric_limits<int>::max();
    register int minY = minX;
    register int maxX = std::numeric_limits<int>::min();
    register int maxY = maxX;
    for(std::vector<ScanLine>::const_iterator it = impl->scanlines.begin(); it!= impl->scanlines.end(); ++it){
      minX = iclMin(minX,it->x);
      maxX = iclMax(maxX,it->x+it->len);
      minY = iclMin(minY,it->y);
      maxY = iclMax(maxY,it->y);
    }
    impl->bb = new Rect(minX,minY,maxX-minX,maxY-minY+1);
    return *impl->bb;
  }

  // }}}
  
  namespace{
    inline bool scanline_cmp_y(const ScanLine &a, const ScanLine &b) { 
      // {{{ open

      return a.y < b.y; 
    }

    // }}}
    inline double eval_sum_of_squares(double k){
      // {{{ open (  retuns sum(i=0..k) i*i  )

      return k*(k+1)*(2*k+1)/6.0;
    }    

    // }}}
    inline double eval_sum(double k){
      // {{{ open (  returns sum(i=0..k) i  ) 

      return k*(k+1)/2.0;
    }

    // }}}
  }
  
  Point Region::getUpperLeftPixel()const{
    // {{{ open

    Point p;
    //    p.y = std::numeric_limits<int>::max();
    //for(unsigned int i=0;i<impl->scanlines.size();++i){
    //  p.y = iclMin(impl->scanlines[i].y,p.y);
    //}

    p.y = std::min_element(impl->scanlines.begin(),impl->scanlines.end(),scanline_cmp_y)->y;

    p.x = std::numeric_limits<int>::max();
    for(unsigned int i=0;i<impl->scanlines.size();++i){
      if(impl->scanlines.at(i).y == p.y){
        p.x = iclMin(impl->scanlines.at(i).x,p.x);
      }
    }      
    return p;
  }

  // }}}

  int Region::getBoundaryLength() const {
    // {{{ open

    return (int)getBoundary().size();
  }

  // }}}

  float Region::getFormFactor() const {
    // {{{ open
    float U = getBoundaryLength();
    float A = getSize();
    return (U*U)/(4*M_PI*A);
  }

  // }}}

  const RegionPCAInfo &Region::getPCAInfo() const {
    // {{{ open
    if(impl->pcainfo) return *impl->pcainfo;
    
    register int x,end,len;
    register double y;
    
    register double avgX = getCOG().x;
    register double avgY = getCOG().y;
    register double avgXX(0),avgXY(0),avgYY(0);
    register int nPts = getSize();
    
    for(unsigned int i=0;i<impl->scanlines.size();++i){
      x = impl->scanlines.at(i).x;
      y = impl->scanlines.at(i).y;
      len = impl->scanlines.at(i).len;
      end = impl->scanlines.at(i).x+len-1;

      /// XX = sum(k=x..end) k²
      avgXX += eval_sum_of_squares(end) - eval_sum_of_squares(x-1);
      
      /// YY = sum(k=x..end) y²
      avgYY += len*y*y;
      
      /// XY = sum(k=x..end) xy = y * sum(k=x..end) k 
      avgXY += y * ( eval_sum(end) - eval_sum(x-1) );
    }
    avgXX/=nPts;
    avgYY/=nPts;
    avgXY/=nPts;
    
    
    double fSxx = avgXX - avgX*avgX;
    double fSyy = avgYY - avgY*avgY;
    double fSxy = avgXY - avgX*avgY;
    
    double fP = 0.5*(fSxx+fSyy);
    double fD = 0.5*(fSxx-fSyy);
    fD = sqrt(fD*fD + fSxy*fSxy);
    double fA  = fP + fD;
    
    impl->pcainfo = new RegionPCAInfo(2*sqrt(fP + fD),2*sqrt(fP - fD),atan2(fA-fSxx,fSxy),avgX,avgY);

    return *impl->pcainfo;
  }

  // }}}

  const std::vector<Point> &Region::getBoundary() const {
    // {{{ open

    if(impl->boundary) return *impl->boundary;
    impl->boundary = new std::vector<Point>;
    switch(impl->image->getDepth()){
#define ICL_INSTANTIATE_DEPTH(D) case depth##D: calculateBoundaryIntern(*impl->image->asImg<icl##D>()); break;
      ICL_INSTANTIATE_ALL_DEPTHS;
#undef ICL_INSTANTIATE_DEPTH
    }
    return *impl->boundary;
  }

  // }}}

  template<class T>
  void Region::calculateBoundaryIntern(const Img<T> &image) const {
    // {{{ open
    const Rect &imageROI = image.getROI();
    register int xMin = imageROI.x;
    register int xMax = imageROI.right()-1;
    register int yMin = imageROI.y;
    register int yMax = imageROI.bottom()-1;
    register int w = image.getWidth();

    impl->boundary->clear();
    Point ul = getUpperLeftPixel();
    int xStart = ul.x;
    int yStart = ul.y;
    
    
    if(getSize() == 1){
      impl->boundary->push_back(Point(xStart,yStart));
      return;
    }
    T v = getVal();
    const T *data = image.getData(0);
    
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
    //   2 c 0     -1  0  
    //     1           w
    //
    //
      
    //                        0  1  2  3  4  5  6  7  
    register int dirs[] =   {-w, 1, w,-1,-w, 1, w,-1 };
    //                        3  0  1  2  3  0  1  2 

    static  int xdirs[] = {  0, 1, 0,-1, 0, 1, 0,-1 };
    static int ydirs[] = { -1, 0, 1, 0,-1, 0, 1, 0 };
    static int jumps[] = {  3, 0, 1, 2, 3, 0, 1, 2 };
    
    register int dirIdx=0;

    const register T *pStart = data+xStart+yStart*w;
    const register T *p = pStart;
    register int x=xStart;
    register int y=yStart;
    
    const register T *cp(0);
    register int cx(0), cy(0);
    register bool posValid(false);
    
    const register T *pBreak(0);
    register int dirIdxBreak(0);

      /// seach 2nd pixel -> criterion for end loop
    impl->boundary->push_back(Point(x,y));
    do{
      cx = x+xdirs[dirIdx];
      cy = y+ydirs[dirIdx];
      cp = p+dirs[dirIdx];
      posValid = cx >= xMin && cx <= xMax && cy >= yMin && cy <= yMax;
      dirIdx++;
    }while(!posValid || *cp!=v);
    p = cp;
    x = cx;
    y = cy;
    pBreak = p;
    dirIdx = jumps[dirIdx-1];   
    dirIdxBreak = dirIdx;
    
    do{
      impl->boundary->push_back(Point(x,y));
      do{
        cx = x+xdirs[dirIdx];
        cy = y+ydirs[dirIdx];
        cp = p+dirs[dirIdx];
        posValid = cx >= xMin && cx <= xMax && cy >= yMin && cy <= yMax;
        dirIdx++;
      }while(!posValid || *cp!=v);
      p = cp;
      x = cx;
      y = cy;
      dirIdx = jumps[dirIdx-1];
    }while ( (p != pBreak) || (dirIdx != dirIdxBreak) );
    impl->boundary->pop_back();
  }

    
  
  const std::vector<Point> &Region::getPixels() const {
    if(impl->pixels) return *impl->pixels;
    impl->pixels = new std::vector<Point>;
    
    for(unsigned int i=0;i<impl->scanlines.size();++i){
      ScanLine &sl = impl->scanlines.at(i);
      for(int x=sl.x;x<sl.x+sl.len;++x){
        impl->pixels->push_back(Point(x,sl.y));
      }
    }
    return *impl->pixels;
  }

  // }}}

    
  namespace{
    template<class T> struct DrawScanLine{
      // {{{ open

      Channel<T> *c;
      T val;
      inline DrawScanLine(Channel<T> *c, T val):c(c),val(val){}
      inline void operator()(const ScanLine &sl){
        T *p = &((*c)(sl.x,sl.y));
        std::fill(p,p+sl.len,val);
      }
    };

    // }}}
  }
  template<class T>
  void Region::drawTo(Img<T> &image, T val) const{
    // {{{ open
    ICLASSERT_RETURN(image.getChannels()>0);

    const std::vector<ScanLine> &s = getScanLines();
    Channel<T> channel = image.extractChannel(0);
    
    std::for_each(s.begin(),s.end(),DrawScanLine<T>(&channel,val));
  }

  // }}}
  


#define ICL_INSTANTIATE_DEPTH(D) template void Region::drawTo<icl##D>(Img<icl##D>&,icl##D)const;
  ICL_INSTANTIATE_ALL_DEPTHS;
#undef ICL_INSTANTIATE_DEPTH
}
