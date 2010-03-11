/*************************************************************************** 
**                                                                        **
** Copyright (C) 2006-2010 neuroinformatics group (vision)                **
**                         University of Bielefeld                        **
**                         nivision@techfak.uni-bielefeld.de              **
**                                                                        **
** This file is part of the ICLBlob module of ICL                         **
**                                                                        **
** Commercial License                                                     **
** Commercial usage of ICL is possible and must be negotiated with us.    **
** See our website www.iclcv.org for more details                         **
**                                                                        **
** GNU General Public License Usage                                       **
** Alternatively, this file may be used under the terms of the GNU        **
** General Public License version 3.0 as published by the Free Software   **
** Foundation and appearing in the file LICENSE.GPL included in the       **
** packaging of this file.  Please review the following information to    **
** ensure the GNU General Public License version 3.0 requirements will be **
** met: http://www.gnu.org/copyleft/gpl.html.                             **
**                                                                        **
***************************************************************************/ 

#include <ICLBlob/Region.h>
#include <ICLBlob/RegionDetector.h>
#include <ICLUtils/Macros.h>
#include <limits>
#include <set>
#include <algorithm>
#include <ICLCore/CornerDetectorCSS.h>
#include <ICLUtils/StringUtils.h>

namespace icl{ 

  struct RegionImpl{
    RegionImpl(icl64f val, const ImgBase *image,const std::vector<Region> *allRegions):
      pixcount(0),val(val),image(image),bb(0),
      pcainfo(0),boundary(0),thinned_boundary(0),pixels(0),boundary_length(-1),
      cornerDetector(0),accurateCenter(0),allRegions(allRegions),
      directSubRegions(0),allSubRegions(0),
      directSurroundingRegions(0),allSurroundingRegions(0),publicNeighbours(0)
    {
      scanlines.reserve(100);
    }
    ~RegionImpl(){
      ICL_DELETE(bb);
      ICL_DELETE(pcainfo);
      ICL_DELETE(boundary);
      ICL_DELETE(thinned_boundary);
      ICL_DELETE(pixels);
      ICL_DELETE(cornerDetector);
      ICL_DELETE(accurateCenter);
      ICL_DELETE(directSubRegions);
      ICL_DELETE(allSubRegions);
      ICL_DELETE(directSurroundingRegions);
      ICL_DELETE(allSurroundingRegions);
      ICL_DELETE(publicNeighbours);
    }
    std::vector<ScanLine> scanlines;
    icl32s pixcount;
    icl64f val;
    Point32f cog;
    const ImgBase *image;
    Rect *bb;
    RegionPCAInfo *pcainfo;
    std::vector<Point> *boundary;
    std::vector<Point> *thinned_boundary;
    std::vector<Point> *pixels;
    float boundary_length;
    CornerDetectorCSS *cornerDetector;
    Point32f *accurateCenter;

    // TODO: these parameters can be combined in an extra structure, which is 0 if
    // the parent region detector has no tree information
    const std::vector<Region> *allRegions;
    std::vector<Region> *directSubRegions;
    std::vector<Region> *allSubRegions;
    std::vector<Region> *directSurroundingRegions;
    std::vector<Region> *allSurroundingRegions;
    std::set<Region*> neighbours;
    std::vector<Region> *publicNeighbours;
  };

  void RegionImplDelOp::delete_func( RegionImpl* impl){
    ICL_DELETE( impl );
  }

  inline Region::ID Region::id() const { return impl.get(); }

  Region::Region(RegionPart *p, int maxSize,icl64f val, const ImgBase *image,
                 const std::vector<Region> *allRegions):
    // {{{ open
    ShallowCopyable<RegionImpl,RegionImplDelOp>(new RegionImpl(val,image,allRegions)){
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
    const_cast<RegionImpl*>(impl.get())->bb = new Rect(minX,minY,maxX-minX,maxY-minY+1);
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

  int Region::getBoundaryPointCount(bool thinned) const {
    // {{{ open

    return (int)getBoundary(thinned).size();
  }

  // Estimates the boundary length by counting how often the three
  // 3-pixel gradients 0 deg, 26.57 deg and 45 deg occour in the boundary and
  // summing these segments' lengths together.
  // 0 deg gradient | 26.57 deg grad | 45 deg grad
  // ooo      oxo   | oox      ooo   | oox      ooo
  // xXx  OR  xXo   | xXo  OR  xXo   | oXo  OR  oXo
  // ooo      ooo   | ooo      xoo   | xoo      xox
  // Also, instead of just iterating over the boundary points, we need to leave
  // out some of them, since e.g. a 45 deg line looks like this:
  // ooxx                ooox
  // oxxo   but we need  ooxo
  // xxoo                oxoo
  float Region::getBoundaryLength() const {
    if (impl->boundary_length != -1) return impl->boundary_length;
    
    const std::vector<Point> &b = getBoundary(true); // thinned boundary
    if (b.size() < 2) return b.size();
    
    static const float length[3] = {1, 1/cos(atan(0.5)), 1/cos(atan(1))}; // length of segment types
    int grad[3] = {0}, type; // counters for segment types
    Point pre = b[b.size()-2];
    Point cur = b[b.size()-1];
    Point post = b[0];
    
    for (unsigned i=0; i < b.size(); i++) {
      type = 0;
      if ((pre.x != cur.x) && (pre.y != cur.y)) type++;
      if ((post.x != cur.x) && (post.y != cur.y)) type++;
      grad[type]++;

      // set pre, cur and post to new values
      pre = cur;
      cur = post;
      post = b[i];
    }
    
    const_cast<RegionImpl*>(impl.get())->boundary_length = length[0]*grad[0] + length[1]*grad[1] + length[2]*grad[2];
    return impl->boundary_length;
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
    
    const_cast<RegionImpl*>(impl.get())->pcainfo = new RegionPCAInfo(2*sqrt(fP + fD),2*sqrt(fP - fD),atan2(fA-fSxx,fSxy),avgX,avgY);

    return *impl->pcainfo;
  }

  // }}}

  const std::vector<Point> &Region::getBoundary(bool thinned) const {
    // {{{ open
    
    if (!impl->boundary) {
      const_cast<RegionImpl*>(impl.get())->boundary = new std::vector<Point>;
      switch(impl->image->getDepth()) {
#define ICL_INSTANTIATE_DEPTH(D) case depth##D: calculateBoundaryIntern(*impl->image->asImg<icl##D>()); break;
      ICL_INSTANTIATE_ALL_DEPTHS;
#undef ICL_INSTANTIATE_DEPTH
      }
    }

    if (thinned) {
      calculateThinnedBoundaryIntern();
      return *impl->thinned_boundary;
    } else return *impl->boundary;
  }
  
  // The function expects a closed boundary in *impl->boundary.
  void Region::calculateThinnedBoundaryIntern() const{
    if (!impl->thinned_boundary) const_cast<SmartPtrBase<RegionImpl,RegionImplDelOp>&>(impl)->thinned_boundary = new std::vector<Point>;
    else impl->thinned_boundary->clear();
    
    std::vector<Point> &boundary=*impl->boundary;
    std::vector<Point> &thinned=*impl->thinned_boundary;
    
    unsigned int N = boundary.size();
    if (N < 3) { // we need at least 3 points in the boundary for thinning
      thinned = boundary;
      return;
    }
    
    // first add the first point in boundary to thinned boundary
    Point last_added = boundary.front();
    thinned.push_back(last_added);
  
    // now iterate through the boundary and decide which points we can drop
    unsigned int i = 2;
    while (i < N) {
      // check whether we can skip the point after 'last_added'
      // this is possible if the point two points after 'last_added' is still in
      // the 8 neighbourhood of 'last_added'
      if ((abs(boundary[i].x - last_added.x) > 1) ||
          (abs(boundary[i].y - last_added.y) > 1)) i -= 1;
      last_added = boundary[i];
      thinned.push_back(last_added);
      i += 2;
    }
  
    // ensure that first and last point of thinned boundary are connected
    if ((abs(thinned.front().x - last_added.x) > 1) ||
        (abs(thinned.front().y - last_added.y) > 1)) thinned.push_back(boundary.back());
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
    const_cast<RegionImpl*>(impl.get())->pixels = new std::vector<Point>;
    
    for(unsigned int i=0;i<impl->scanlines.size();++i){
      ScanLine &sl = const_cast<RegionImpl*>(impl.get())->scanlines.at(i);
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
    Channel<T> channel = image[0];
    
    std::for_each(s.begin(),s.end(),DrawScanLine<T>(&channel,val));
  }

  // }}}
  
  const std::vector<Point32f> &Region::getBoundaryCorners(float angle_thresh,
                                                          float rc_coeff, 
                                                          float sigma, 
                                                          float curvature_cutoff, 
                                                          float straight_line_thresh) const{
    bool needReDetection = false;
    
    if(!impl->cornerDetector){
      const_cast<RegionImpl*>(impl.get())->cornerDetector = new CornerDetectorCSS(angle_thresh,rc_coeff,sigma,curvature_cutoff,straight_line_thresh);
      needReDetection = true;
    }else{
#define ONE_PARAM(Y,P)                            \
      if(impl->cornerDetector->get##Y() != P){    \
        needReDetection = true;                   \
        impl->cornerDetector->set##Y(P);          \
      }
      ONE_PARAM(AngleThreshold,angle_thresh);
      ONE_PARAM(RCCoeff,rc_coeff);
      ONE_PARAM(Sigma,sigma);
      ONE_PARAM(CurvatureCutoff,curvature_cutoff);
      ONE_PARAM(StraightLineThreshold,straight_line_thresh);
#undef ONE_PARAM
    }
    if(needReDetection){
      return impl->cornerDetector->detectCorners(getBoundary());
    }else{
      return impl->cornerDetector->getLastCorners();
    }
  }

  template<class T>
  const Point32f &Region::getAccurateCOG(const Img<T> &grayImage, int bbMargin, 
                                         const T &minThreshold, const T &maxThreshold,
                                         bool darkBlob) const{
    if(impl->accurateCenter) return *impl->accurateCenter;
    const_cast<SmartPtrBase<icl::RegionImpl, icl::RegionImplDelOp>&>(impl)->accurateCenter = new Point32f(-1,-1);
    Point32f &p = *const_cast<SmartPtrBase<icl::RegionImpl, icl::RegionImplDelOp>&>(impl)->accurateCenter;
    ICLASSERT_RETURN_VAL(grayImage.getChannels(),p);
    Rect r = getBoundingBox().enlarged(bbMargin) & grayImage.getImageRect();
    const Channel<T> c = grayImage[0];
    
    double sum = 0;
    double cx=0,cy=0;
    for(int x=r.x; x<r.right();++x){
      for(int y=r.y;y<r.bottom();++y){
        double v = icl::clip<T>(c(x,y),minThreshold,maxThreshold)-double(minThreshold);
        if(darkBlob){
          v = (maxThreshold-minThreshold)-v;
        }
        sum += v;
        cx += float(x)*v;
        cy += float(y)*v;
      }
    }

    p.x = cx/sum;
    p.y = cy/sum;
    
    return p;
  }

  static inline bool contains_full(const Rect &a, const Rect &b) {
    return a.x < b.x && 
           a.y < b.y && 
           a.right()  > b.right() && 
           a.bottom() > b.bottom();
  }


  bool region_search_zero(std::set<Region::ID> &buf, // buf contains outer
                          const Region *inner){
    std::set<Region*> &inb = inner->getNeighbourSet();
    if(*inb.begin() == 0) return true;
    for(std::set<Region*>::iterator it = inb.begin();it != inb.end();++it){
      if(!buf.count((*it)->id())){
        buf.insert((*it)->id());
        if(region_search_zero(buf, *it)){
          return true;
        }
      }
    }
    return false;
  }

  bool is_region_contained(Region *outer, const Region *inner){
    if(!inner) return false;
    std::set<Region::ID> buf;
    buf.insert(outer->id());
    return !region_search_zero(buf,inner);
  }


  

  void collect_subregions_recursive(std::set<Region::IDRegion> &all,Region *r){
    const std::vector<Region> &ds = r->getSubRegions(true); 
    for(unsigned int i=0;i<ds.size(); ++i){
      Region::IDRegion id (ds[i].id(),const_cast<Region*>(&ds[i]));
      if(!all.count(id)){
        all.insert(id);
        collect_subregions_recursive(all,id.r);
      }
    }
  }
  
  const std::vector<Region> &Region::getSubRegions(bool directOnly) const{
    if(directOnly && impl->directSubRegions) return *impl->directSubRegions;
    if(!directOnly && impl->allSubRegions) return *impl->allSubRegions;
    
    if(!impl->directSubRegions){
      const_cast<SmartPtrBase<icl::RegionImpl, icl::RegionImplDelOp>&>(impl)->directSubRegions = new std::vector<Region>;
      
      for(std::set<Region*>::iterator it = impl->neighbours.begin(); it != impl->neighbours.end();++it){
        if(is_region_contained(const_cast<Region*>(this),*it)){
          impl->directSubRegions->push_back(*const_cast<Region*>(*it));
        }
      }
    }
    if(directOnly){
      return *impl->directSubRegions;
    }else{
      std::set<IDRegion> all;
      collect_subregions_recursive(all,const_cast<Region*>(this));
      
      const_cast<SmartPtrBase<icl::RegionImpl, icl::RegionImplDelOp>&>(impl)->allSubRegions = new std::vector<Region>(all.size());
      int i=0;
      for(std::set<IDRegion>::iterator it = all.begin();it!=all.end();++it){
        impl->allSubRegions->operator[](i++) = *it->r;
      }
      return *impl->allSubRegions;
    }
    


  }

  const std::vector<Region> &Region::getSurroundingRegions(bool directOnly) const{
    if(!directOnly){
      static std::vector<Region> dummy;
      ERROR_LOG("this function is not yet implemented for directOnly=false mode (returning empty buffer)");
      return dummy;
      
      /* here, we will again need a recursive collection function, which collects 
         surrounding regions of surrounding regions and so on, into a sorted set. 
         This can be implemented easily, fi it's needed*/
    }
    if(directOnly && impl->directSurroundingRegions) return *impl->directSurroundingRegions;
    //    if(!directOnly && impl->allSurroundingRegions) return impl->allSurroundingRegions;
    const_cast<SmartPtrBase<icl::RegionImpl, icl::RegionImplDelOp>&>(impl)->directSurroundingRegions = new std::vector<Region>;
    
    for(std::set<Region*>::iterator it = impl->neighbours.begin(); it != impl->neighbours.end();++it){
      if(!*it) continue;
      const std::vector<Region> &itsub = (*it)->getSubRegions(true);
      for(unsigned int j=0;j<itsub.size();++j){
        if(itsub[j].id() == id()){
          impl->directSurroundingRegions->push_back(**it);
          break;
        }
      }
    }
    return *impl->directSurroundingRegions;
  }

  const std::vector<Region> &Region::getNeighbours(bool *isAtBorder) const{
    if(impl->publicNeighbours) return *impl->publicNeighbours;
    const_cast<SmartPtrBase<icl::RegionImpl, icl::RegionImplDelOp>&>(impl)->publicNeighbours = new std::vector<Region>;
    std::set<Region*>::const_iterator it = impl->neighbours.begin();
    unsigned int n = impl->neighbours.size();
    
    if(n){
      if(isAtBorder) *isAtBorder = !*it;
      if(!*it){
        it++;
        n--;
      }
    }
    impl->publicNeighbours->resize(n);
    for(int i=0;it != impl->neighbours.end();++it,++i){
      impl->publicNeighbours->operator[](i) = **it;
    }
    return *impl->publicNeighbours;
  }

#define ICL_INSTANTIATE_DEPTH(D)                                        \
  template void Region::drawTo<icl##D>(Img<icl##D>&,icl##D)const;       \
  template const Point32f &Region::getAccurateCOG<icl##D>(const Img<icl##D>&,int, const icl##D&, const icl##D&, bool) const;

  ICL_INSTANTIATE_ALL_DEPTHS;
#undef ICL_INSTANTIATE_DEPTH


  void Region::addNeighbour(Region *n){
    impl->neighbours.insert(n);
  }


  std::set<Region*> &Region::getNeighbourSet() const{
    return const_cast<std::set<Region*>&>(impl->neighbours);
  }


}
