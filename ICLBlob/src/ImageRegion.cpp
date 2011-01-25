/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2010 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLBlob/src/ImageRegion.cpp                            **
** Module : ICLBlob                                                **
** Authors: Christof Elbrechter, Erik Weitnauer                    **
**                                                                 **
**                                                                 **
** Commercial License                                              **
** ICL can be used commercially, please refer to our website       **
** www.iclcv.org for more details.                                 **
**                                                                 **
** GNU General Public License Usage                                **
** Alternatively, this file may be used under the terms of the     **
** GNU General Public License version 3.0 as published by the      **
** Free Software Foundation and appearing in the file LICENSE.GPL  **
** included in the packaging of this file.  Please review the      **
** following information to ensure the GNU General Public License  **
** version 3.0 requirements will be met:                           **
** http://www.gnu.org/copyleft/gpl.html.                           **
**                                                                 **
** The development of this software was supported by the           **
** Excellence Cluster EXC 277 Cognitive Interaction Technology.    **
** The Excellence Cluster EXC 277 is a grant of the Deutsche       **
** Forschungsgemeinschaft (DFG) in the context of the German       **
** Excellence Initiative.                                          **
**                                                                 **
*********************************************************************/


#include <ICLBlob/ImageRegion.h>
#include <ICLBlob/ImageRegionData.h>

#include <ICLUtils/StringUtils.h>
#include <ICLCore/Img.h>

#include <vector>

namespace icl{
  
  namespace{
    template<class T>
    struct DrawLineSegment{
      // {{{ open
      Channel<T> &c;
      T val;
      inline DrawLineSegment(Channel<T> &c, T val):c(c),val(val){}
      inline void operator()(const LineSegment &sl){
        const icl16u &x = sl.x;
        const icl16u &y = sl.y;
        const icl16u &xend = sl.xend;
        std::fill(&c(x,y),&c(xend,y),val);
      }
      // }}}
    };
  }

  template<class T>
  void sample_image_region(const std::vector<LineSegment> &ls, Img<T> &image, const std::vector<int> &cs){
    for(int c=0;c<image.getChannels() && c<(int)cs.size();++c){
      Channel<T> ch = image[c];
      std::for_each(ls.begin(),ls.end(),DrawLineSegment<T>(ch,(T)cs[c]));
    }
  }

  /// samples the region into a given image
  void ImageRegion::sample(ImgBase *image, int color){
    sample(image,std::vector<int>(1,color));
  }
  
  /// samples the region into a given image
  void ImageRegion::sample(ImgBase *image, const std::vector<int> &channelColors){
    ICLASSERT_RETURN(image);
    const std::vector<LineSegment> &ls = getLineSegments();
    switch(image->getDepth()){
#define ICL_INSTANTIATE_DEPTH(D) case depth##D: sample_image_region<icl##D>(ls,*image->asImg<icl##D>(),channelColors); break;
      ICL_INSTANTIATE_ALL_DEPTHS
#undef ICL_INSTANTIATE_DEPTH
    }
  }

  /** older more complex function that need ICLQuick
  void ImageRegion::sample(Img32f &dst,Img32f *labelDst, int factor, const std::string &label) const{
    Channel32f channel = dst[0];
    std::for_each(data()->segments.begin(),
                  data()->segments.end(),
                  DrawLineSegment(channel,data()->value));
    
    if(label.size() && labelDst){
      color(255,255,255,255);
      fontsize(7);
      for(unsigned int i=0;i<data()->segments.size();++i){
        const LineSegment &s = data()->segments[i];
        for(int x=s.x; x<s.xend; ++x){
          text(*labelDst,factor*x+1,factor*s.y-3,label);
        }
      }
    }
  }
  **/
  int ImageRegion::getSize() const{
    // {{{ open

    int &size = data()->size;
    if(size) return size;
    for(unsigned int i=0;i<data()->segments.size();++i){
      size += data()->segments[i].len();
    }
    return size;
  }

  // }}}
  
  int ImageRegion::getVal() const {
    // {{{ open

    return data()->value;
  }

  // }}}

  int ImageRegion::getID() const{
    // {{{ open
    return m_data->id;
  }
  // }}}


  Point32f ImageRegion::getCOG() const{
    // {{{ open
    ImageRegionData::SimpleInformation *simple = m_data->ensureSimple();
    if(simple->cog) return *simple->cog;
    
    Point32f cog = Point32f::null;
    for(std::vector<LineSegment>::const_iterator it = m_data->segments.begin(); it != m_data->segments.end();++it){
      int l = it->len();
      cog.x += l * ( it->x + (0.5*(l-1)));
      cog.y += l * it->y;
    }
    float s = 1.0/getSize();
    
    return *(simple->cog = new Point32f(s*cog.x,s*cog.y));
  }

  // }}}

  const std::vector<LineSegment> &ImageRegion::getLineSegments() const{
    // {{{ open
    return m_data->segments;
  }
  // }}}
  
  const Rect &ImageRegion::getBoundingBox() const{
    // {{{ open
    ImageRegionData::SimpleInformation *simple = m_data->ensureSimple();
    if(simple->boundingBox) return *simple->boundingBox;
    
    register int minX = std::numeric_limits<int>::max();
    register int minY = minX;
    register int maxX = std::numeric_limits<int>::min();
    register int maxY = maxX;
    
    for(std::vector<LineSegment>::const_iterator it = m_data->segments.begin(); it != m_data->segments.end();++it){
      int x = it->x, y=it->y, xend=it->xend;
      if(x < minX) minX = x;
      if(xend > maxX) maxX = xend;

      if(y < minY) minY = y;
      if(y > maxY) maxY = y;
    }
    return *(simple->boundingBox = new Rect(minX,minY,maxX-minX,maxY-minY+1));
  }

  // }}}
  

  static inline double eval_sum_of_squares(double k){
    // {{{ open (  retuns sum(i=0..k) i*i  )
    
    return k*(k+1)*(2*k+1)/6.0;
  }    
  
    // }}}
  static inline double eval_sum(double k){
    // {{{ open (  returns sum(i=0..k) i  ) 
    
    return k*(k+1)/2.0;
  }
  // }}}

  const RegionPCAInfo &ImageRegion::getPCAInfo() const {
    // {{{ open
    ImageRegionData::SimpleInformation *simple = m_data->ensureSimple();
    if(simple->pcainfo) return *simple->pcainfo;
    
    register int x,end,len;
    register double y;
    
    register double avgX = getCOG().x;
    register double avgY = getCOG().y;
    register double avgXX(0),avgXY(0),avgYY(0);
    register int nPts = getSize();
    
    const std::vector<LineSegment> &segs = m_data->segments;
    for(unsigned int i=0;i<segs.size();++i){
      x = segs[i].x;
      y = segs[i].y;
      len = segs[i].len();
      end = segs[i].x+len-1;

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
    fD = ::sqrt(fD*fD + fSxy*fSxy);
    double fA  = fP + fD;
    
    return *(simple->pcainfo = new RegionPCAInfo(2*::sqrt(fP + fD),2*::sqrt(fP - fD),::atan2(fA-fSxx,fSxy),avgX,avgY));
  }
  // }}}
 

  const std::vector<Point> &ImageRegion::getBoundary(bool thinned) const {
    // {{{ open
    ImageRegionData::SimpleInformation *simple = m_data->ensureSimple();
    
    if (!simple->boundary) {
      simple->boundary = new std::vector<Point>;
      switch(m_data->image->getDepth()) {
#define ICL_INSTANTIATE_DEPTH(D) case depth##D: calculateBoundaryIntern(*m_data->image->asImg<icl##D>()); break;
      ICL_INSTANTIATE_ALL_INT_DEPTHS;
#undef ICL_INSTANTIATE_DEPTH
        default:
          ICL_INVALID_DEPTH;
      }
    }

    if (thinned) {
      calculateThinnedBoundaryIntern();
      return *simple->thinned_boundary;
    }else{
      return *simple->boundary;
    }
  }
  
  // The function expects a closed boundary in *impl->boundary.
  void ImageRegion::calculateThinnedBoundaryIntern() const{
    ImageRegionData::SimpleInformation *simple = m_data->ensureSimple();
    if(simple->thinned_boundary) return;
    m_data->simple->thinned_boundary = new std::vector<Point>;
    
    std::vector<Point> &boundary = *simple->boundary;
    std::vector<Point> &thinned = *simple->thinned_boundary;
    
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
      if ((::abs(boundary[i].x - last_added.x) > 1) ||
          (::abs(boundary[i].y - last_added.y) > 1)) i -= 1;
      last_added = boundary[i];
      thinned.push_back(last_added);
      i += 2;
    }
  
    // ensure that first and last point of thinned boundary are connected
    if ((::abs(thinned.front().x - last_added.x) > 1) ||
        (::abs(thinned.front().y - last_added.y) > 1)) thinned.push_back(boundary.back());
  }

  // }}}

  static inline bool line_segment_cmp_y(const LineSegment &a, const LineSegment &b) { 
    // {{{ open
    
    return a.y < b.y; 
  }
  
  // }}}

  Point ImageRegion::getUpperLeftPixel()const{
    // {{{ open

    Point p;
    p.y = std::min_element(m_data->segments.begin(),m_data->segments.end(),line_segment_cmp_y)->y;

    p.x = std::numeric_limits<int>::max();
    for(unsigned int i=0;i<m_data->segments.size();++i){
      if(m_data->segments[i].y == p.y){
        p.x = iclMin(m_data->segments[i].x,p.x);
      }
    }      
    return p;
  }

  // }}}


  template<class T>
  void ImageRegion::calculateBoundaryIntern(const Img<T> &image) const {
    // {{{ open
    const Rect &imageROI = image.getROI();
    register int xMin = imageROI.x;
    register int xMax = imageROI.right()-1;
    register int yMin = imageROI.y;
    register int yMax = imageROI.bottom()-1;
    register int w = image.getWidth();

    std::vector<Point> &boundary = *m_data->simple->boundary;

    Point ul = getUpperLeftPixel();
    
    int xStart = ul.x;
    int yStart = ul.y;
    
    if(getSize() == 1){
      boundary.push_back(Point(xStart,yStart));
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
    boundary.push_back(Point(x,y));
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
      boundary.push_back(Point(x,y));
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
    boundary.pop_back();
  }
  // }}}
 
  int ImageRegion::getBoundaryPointCount(bool thinned) const{
    // {{{ open

    return (int)getBoundary(thinned).size();
  }

  // }}}


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
  float ImageRegion::getBoundaryLength() const {
    ImageRegionData::SimpleInformation *simple = m_data->ensureSimple();
    if(simple->boundaryLength) return simple->boundaryLength;
    
    const std::vector<Point> &b = getBoundary(true); // thinned boundary
    if (b.size() < 2) return b.size();
    
    static const float length[3] = {1, 1/::cos(::atan(0.5)), 1/::cos(::atan(1))}; // length of segment types
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
    
    return (simple->boundaryLength = length[0]*grad[0] + length[1]*grad[1] + length[2]*grad[2]);
  }

  // }}}

  const std::vector<Point32f> &ImageRegion::getBoundaryCorners() const{
    ImageRegionData::ComplexInformation *complex = m_data->ensureComplex();

    bool needReDetection = false;
    
    if(!complex->cssParams){
      complex->cssParams = new ImageRegionData::CSSParams;
      complex->cssParams->setFrom(m_data->css);
      needReDetection = true;
    }else{
      if(!complex->cssParams->isOk(m_data->css)){
        complex->cssParams->setFrom(m_data->css);
        needReDetection = true;
      }
    }
    if(needReDetection){
      complex->cssParams->resultBuffer = m_data->css->detectCorners(getBoundary());
    }
    return complex->cssParams->resultBuffer;
  }


  float ImageRegion::getFormFactor() const {
    // {{{ open
    float U = getBoundaryLength();
    float A = getSize();
    return (U*U)/(4*M_PI*A);
  }

  // }}}



  const std::vector<Point> &ImageRegion::getPixels() const {
    // {{{ open
    ImageRegionData::SimpleInformation *simple = m_data->ensureSimple();
    if(simple->pixels) return *simple->pixels;
    simple->pixels = new std::vector<Point>(getSize());
    
    int k=0;
    for(unsigned int i=0;i<m_data->segments.size();++i){
      const LineSegment &s = m_data->segments[i];
      for(int x=s.x;x<s.xend;++x, ++k){
        simple->pixels->operator[](k) = Point(x,s.y);
      }
    }
    return *simple->pixels;
  }

  // }}}

    
  namespace{
    template<class T> struct DrawLineSeg{
      // {{{ open

      Channel<T> *c;
      T val;
      inline DrawLineSeg(Channel<T> *c, int val):c(c),val(val){}
      inline void operator()(const LineSegment &sl){
        T *p = &((*c)(sl.x,sl.y));
        std::fill(p,p+sl.len(),val);
      }
    };

    // }}}
  }

  void ImageRegion::drawTo(const ImgBase *image, int val) const{
    // {{{ open
    ICLASSERT_RETURN(image);
    ICLASSERT_RETURN(image->getChannels());

    const std::vector<LineSegment> &s = getLineSegments();
    
    switch(image->getDepth()){
#define ICL_INSTANTIATE_DEPTH(D)                                        \
      case depth##D:{                                                   \
        Channel<icl##D> channel = image->asImg<icl##D>()->operator[](0); \
        std::for_each(s.begin(),s.end(),DrawLineSeg<icl##D>(&channel,val)); \
        break;                                                          \
      }
      ICL_INSTANTIATE_ALL_DEPTHS;
#undef ICL_INSTANTIATE_DEPTH
    }
  }

  // }}}


  // search for a isBorder-region without collecting regions from buf
  bool region_search_border(std::set<ImageRegionData*> &buf, // buf contains outer
                            ImageRegionData *inner){
    
    if(inner->graph->isBorder) return true;
    
    // then: depth first
    for(std::set<ImageRegionData*>::iterator it=inner->graph->neighbours.begin(), 
        itEnd=inner->graph->neighbours.end() ; it != itEnd; ++it){
      //      if((*it)->isBorder) return true; // we check all neighbours first
      if(!buf.count(*it)){
        if((*it)->graph->isBorder) return true;
        buf.insert(*it);
        if(region_search_border(buf, *it)){
          return true;
        }
      }
    }
    return false;
  }



  bool is_region_contained(ImageRegionData *outer, ImageRegionData *inner){
    std::set<ImageRegionData*> buf;
    buf.insert(outer);
    return !region_search_border(buf,inner);
  }

  
  bool is_rect_larger(const Rect &a, const Rect &b){
    return a.x<b.x || a.y<b.y  || a.right() > b.right() || a.bottom() > b.bottom();
  }
  
  // search for a thats bounding box is 'larger' than r without collecting regions from buf
  bool region_search_outer_bb(const Rect &r,
                              std::set<ImageRegionData*> &buf, // buf contains outer
                              ImageRegionData *inner){
    
    if (inner->graph->isBorder || is_rect_larger(ImageRegion(inner).getBoundingBox(),r)) return true;
    
    // then: depth first
    for(std::set<ImageRegionData*>::iterator it=inner->graph->neighbours.begin(), 
        itEnd=inner->graph->neighbours.end() ; it != itEnd; ++it){
      //      if((*it)->isBorder) return true; // we check all neighbours first
      if(!buf.count(*it)){
        if (inner->graph->isBorder || is_rect_larger(ImageRegion(*it).getBoundingBox(),r)) return true;
        buf.insert(*it);
        if(region_search_outer_bb(r,buf, *it)){
          return true;
        }
      }
    }
    return false;
  }



  bool is_region_contained_bb(ImageRegionData *outer, ImageRegionData *inner){
    std::set<ImageRegionData*> buf;
    buf.insert(outer);
    return !region_search_outer_bb(ImageRegion(outer).getBoundingBox(),buf,inner);
  }


  void collect_subregions_recursive(std::set<ImageRegionData*> &all, ImageRegionData *r){
    ImageRegion(r).getSubRegions();
    
    std::vector<ImageRegionData*> &cs = r->graph->children;

    for(std::vector<ImageRegionData*>::iterator it = cs.begin(); it != cs.end(); ++it){
      if(!all.count(*it)){
        all.insert(*it);
        collect_subregions_recursive(all,*it);
      }
    }
  }



  const std::vector<ImageRegion> &ImageRegion::getSubRegions(bool directOnly) const throw (ICLException){
    ICLASSERT_THROW(m_data->graph, ICLException("ImageRegion::getSubRegions: no region graph information available"));
    ImageRegionData::ComplexInformation *complex = m_data->ensureComplex();
    if(directOnly && complex->directSubRegions) return *complex->directSubRegions;
    if(!directOnly && complex->allSubRegions) return *complex->allSubRegions;

    if(!complex->directSubRegions){
      ImageRegionData *r = m_data;
      if(r->graph->neighbours.size() > 1){
        std::set<ImageRegionData*> &nb = r->graph->neighbours;
        for(std::set<ImageRegionData*>::iterator itn = nb.begin();itn != nb.end(); ++itn){
          ImageRegionData *n = *itn;
          if(!n->graph->isBorder){
            if(n->graph->neighbours.size() == 1 || is_region_contained_bb(r,n)){
              r->addChild(n);
            }
          }
        }
      }
      complex->directSubRegions = new std::vector<ImageRegion>(m_data->graph->children.begin(),m_data->graph->children.end());
    }
    if(directOnly){
      return *complex->directSubRegions;
    }else{
      std::set<ImageRegionData*> all; 
      collect_subregions_recursive(all,m_data); 
      return *(complex->allSubRegions = new std::vector<ImageRegion>(all.begin(),all.end()));
    }
  }

  
  const ImageRegion &ImageRegion::getParentRegion() const throw (ICLException){
    ICLASSERT_THROW(m_data->graph, ICLException("ImageRegion::getParentRegion: no region graph information available"));
    ImageRegionData::ComplexInformation *complex = m_data->ensureComplex();
    
    if(complex->parent) return *complex->parent;
    
    const std::vector<ImageRegion> &nb = getNeighbours();
    for(unsigned int i=0;i<nb.size();++i){
      nb[i].getSubRegions();
    }

    return *(complex->parent = new ImageRegion(m_data->graph->parent));
  }

  void collect_parent_regions_recursive(std::vector<ImageRegion> &buf, const ImageRegion &r){
    const ImageRegion &p = r.getParentRegion();
    if(!p) return;
    buf.push_back(p);
    collect_parent_regions_recursive(buf,p);
  }
  
  const std::vector<ImageRegion> &ImageRegion::getParentTree() const throw (ICLException){
    ICLASSERT_THROW(m_data->graph, ICLException("ImageRegion::getParentTree: no region graph information available"));
    ImageRegionData::ComplexInformation *complex = m_data->ensureComplex();
    
    if(complex->parentTree) return *complex->parentTree;
    
    complex->parentTree = new std::vector<ImageRegion>;
    collect_parent_regions_recursive(*complex->parentTree,*this);
    
    return *complex->parentTree;
  }

  const std::vector<ImageRegion> &ImageRegion::getNeighbours() const throw (ICLException){
    ICLASSERT_THROW(m_data->graph, ICLException("ImageRegion::getNeighbours: no region graph information available"));
    ImageRegionData::ComplexInformation *complex = m_data->ensureComplex();
    if(complex->publicNeighbours) return *complex->publicNeighbours;

    std::set<ImageRegionData*> &nb = m_data->graph->neighbours;
    return *(complex->publicNeighbours = new std::vector<ImageRegion>(nb.begin(),nb.end()));
  }

  bool ImageRegion::isBorderRegion() const throw (ICLException){
    ICLASSERT_THROW(m_data->graph, ICLException("ImageRegion::isBorderRegion: no region graph information available"));
    return m_data->graph->isBorder;
  }

  bool ImageRegion::contains(const Point &p) const{
    const std::vector<LineSegment> &ls = m_data->segments;
    for(unsigned int i=0;i<ls.size();++i){
      const LineSegment &s = ls[i];
      if(p.y == s.y && p.x >= s.x && p.x < s.xend){
        return true;
      }
    }
    return false;
  }


  static void show_tree_recursive(const ImageRegion &r, int indent){
    for(int i=0;i<indent-1;++i) std::cout << "   ";
    if(indent)  std::cout << "`-";
    std::cout << r.getID()  << " --- neighbours:[";
    const std::vector<ImageRegion> &ns = r.getNeighbours();
    for(unsigned int i=0;i<ns.size();++i){
      std::cout << ns[i].getID() << (i<ns.size()-1 ? "," : "]"); 
    }
    std::cout << std::endl;
    
    const std::vector<ImageRegion> &cs = r.getSubRegions();
    for(unsigned int i=0;i<cs.size();++i){
      show_tree_recursive(cs[i],indent+1);
    }
  }

  void ImageRegion::showTree() const{
    ICLASSERT_THROW(m_data->graph, ICLException("ImageRegion::showTree: no region graph information available"));
    show_tree_recursive(*this,0);
  }

  void ImageRegion::setMetaData(const Any &any) const{
    m_data->meta = any;
  }
  
  const Any &ImageRegion::getMetaData() const{
    return m_data->meta;
  }

}
