#include <ICLMarkers/FiducialDetectorPluginICL1.h>
#include <ICLMarkers/MarkerMetricsICL1.h>
#include <ICLUtils/Range.h>

#include <ICLQuick/Quick.h>

namespace icl{

  struct FiducialImplICL1 : public FiducialImpl{
    FiducialImplICL1(FiducialDetectorPlugin *parent,
                     Fiducial::FeatureSet supported,
                     Fiducial::FeatureSet computed,
                     int id, int index,
                     const Size32f &realSizeMM,
                     ImageRegion region,
                     const MarkerCodeICL1 &code):
      FiducialImpl(parent,supported,computed,id,index,realSizeMM),
      region(region),metrics(code,realSizeMM){}
    ImageRegion region;
    MarkerMetricsICL1 metrics;
  };
  
  
  struct FiducialDetectorPluginICL1::Data{
    std::vector<ImageRegion> regions;
    std::vector<FiducialImpl*> impls;
    std::bitset<1296> valid; // MarkerCodeICL1::P^4 = 1296
    std::bitset<1296> loaded; // MarkerCodeICL1::P^4 = 1296
    Size sizes[1296];
  };

  FiducialDetectorPluginICL1::FiducialDetectorPluginICL1():data(new Data){
    data->loaded.reset();
    data->valid.reset();
    const std::vector<MarkerCodeICL1> &existing = MarkerCodeICL1::generate();
    std::cout << "existing.size(): " << existing.size() << std::endl;
    for(unsigned int i=0;i<existing.size();++i){
      data->valid[existing[i].id] = true;
      std::cout << existing[i].id << "  ";
    }
    std::cout << "\n\n";
  }

  FiducialDetectorPluginICL1::~FiducialDetectorPluginICL1(){
    delete data;
  }
  
  
  void FiducialDetectorPluginICL1::getCorners2D(std::vector<Point32f> &dst, FiducialImpl &impl){
    const std::vector<Point> &boundary = ((FiducialImplICL1&)impl).region.getBoundary();
    dst.resize(boundary.size());
    std::copy(boundary.begin(),boundary.end(),dst.begin());
  }
  
  void FiducialDetectorPluginICL1::getRotation2D(float &dst, FiducialImpl &impl){
    Fiducial(&impl).getKeyPoints2D();
    dst = ((FiducialImplICL1&)impl).info2D->infoRotation;
  }

  static std::pair<int,int> get_outer_regions(const std::vector<ImageRegion> &rs){
    const Point32f cogs[4] = {rs[0].getCOG(), rs[1].getCOG(), rs[2].getCOG(), rs[3].getCOG()};
    Mat dist(0.0f);
    for(int i=0;i<4;++i){
      for(int j=0;j<i;++j){
        dist(i,j) = cogs[i].distanceTo(cogs[j]);
      }
    }
    int idx = (int)(std::max_element(dist.begin(),dist.end()) - dist.begin());
    return std::pair<int,int>(idx%4,idx/4);
  }

  static std::vector<ImageRegion> sort_subregions(const std::vector<ImageRegion> &rs, const std::pair<int,int> &outer){
    int min = outer.first, max=outer.second;
    if( rs[min].getSubRegions().size() > rs[max].getSubRegions().size() ){
      std::swap(min,max); 
    }
    
    bool used[4] = {0}; 
    used[min] = true; 
    used[max] = true;
    
    int a=0, b=0;
    for(int i=0;i<4;++i){
      if(!used[i]){
        a = i;
        break;
      }
    }
    for(int i=a+1;i<4;++i){
      if(!used[i]){
        b = i;
        break;
      }
    }
    
    if(rs[min].getCOG().distanceTo(rs[a].getCOG()) > rs[min].getCOG().distanceTo(rs[b].getCOG()) ){
      std::swap(a,b);
    }
    
    std::vector<ImageRegion> ret(4);
    ret[0] = rs[min];
    ret[1] = rs[a];
    ret[2] = rs[b];
    ret[3] = rs[max];
    
    return ret;
  }

  // Idea: use special structure FourImageRegions -> fixed size array of four instances of type ImageRegion

  namespace{
    struct sprod_to_cmp{
      const Point32f &x;
      sprod_to_cmp(const Point32f &x):x(x){}
      inline float sprod(const Point32f &ab) const{
        return x.x*ab.x + x.y*ab.y; 
      }
      inline bool operator()(const Point32f &a, const Point32f &b) const{
        return sprod(a) < sprod(b);
      }
    };
  }

  static float normalize_angle(float a){
    if(a < 0) a += 2*M_PI;
    return a;
  }

  void FiducialDetectorPluginICL1::getKeyPoints2D(std::vector<Fiducial::KeyPoint> &dst, FiducialImpl &impl){
    FiducialImplICL1 &iicl = (FiducialImplICL1&)(impl);
    const MarkerMetricsICL1 &metrics = iicl.metrics;
    Point32f c = impl.info2D->infoCenter;
    std::vector<ImageRegion> srs = iicl.region.getSubRegions();
    std::vector<ImageRegion> sorted = sort_subregions(srs, get_outer_regions(srs));
    
    Point32f vy = sorted[0].getCOG()-sorted[3].getCOG();
    impl.ensure2D()->infoRotation = normalize_angle(atan2(vy.y,vy.x));
    Point32f vx (-vy.y, vy.x);
    
    // associate points
    for(int i=0;i<4;++i){
      const std::vector<ImageRegion> s=sorted[i].getSubRegions();
      int n = (int)s.size();
      std::vector<Point32f> cs(n);
      std::transform(s.begin(),s.end(),cs.begin(), std::mem_fun_ref(&ImageRegion::getCOG));
      std::sort(cs.begin(),cs.end(), sprod_to_cmp( vx )); // face == TPFrontFace ? x : Point32f(-x.x,-x.y)));

      for(int j=0;j<n;++j){
        dst.push_back(Fiducial::KeyPoint(s[j].getCOG(),metrics.crs[i].ccrs[j].center(),i+MarkerCodeICL1::P1*j));
      }
    }
    
  }
  

  void FiducialDetectorPluginICL1::getFeatures(Fiducial::FeatureSet &dst){
    static const Fiducial::FeatureSet all = Fiducial::AllFeatures;
    dst = all;
  }
  
  // Think about this
  // ------------------------------------
  // think about maybe changing the logic of the detect method
  // since currently the results are always push_backed twice ...
  // maybe we can simply ensure, that dst is not touched by the parent
  // FiducialDetector
  
  void FiducialDetectorPluginICL1::detect(std::vector<FiducialImpl*> &dst, const std::vector<ImageRegion> &regions){
    static const Fiducial::FeatureSet supported = Fiducial::AllFeatures;
    static const Fiducial::FeatureSet computed = 1<<Fiducial::Center2D;

    for(unsigned int i=0;i<data->impls.size();++i){
      delete data->impls[i];
    }
    data->impls.clear();
    
    for(unsigned int i=0;i<regions.size();++i){
      ImageRegion r = regions[i];
      if(!Range32f(1.3,4.5).contains(r.getFormFactor())) continue;
      
      const std::vector<ImageRegion> &srs = regions[i].getSubRegions();
      if(srs.size() != 4) continue;
      
      int ns[] = { 
        iclMin(MarkerCodeICL1::P,(int)srs[0].getSubRegions().size()),
        iclMin(MarkerCodeICL1::P,(int)srs[1].getSubRegions().size()),
        iclMin(MarkerCodeICL1::P,(int)srs[2].getSubRegions().size()),        
        iclMin(MarkerCodeICL1::P,(int)srs[3].getSubRegions().size())
      };
      
      std::sort(ns,ns+4);
      MarkerCodeICL1 code(ns);
      if(!Range32s(0,1296).contains(code.id) || !data->loaded[code.id]) continue;

      FiducialImpl *impl = new FiducialImplICL1(this,supported,computed,
                                                code.id,data->impls.size(),
                                                data->sizes[code.id],
                                                r,code);
      
      FiducialImpl::Info2D *info2D = impl->ensure2D();
      info2D->infoCenter = r.getCOG();
      
      data->impls.push_back(impl);
    }
    dst = data->impls;
  }
  
  void FiducialDetectorPluginICL1::addOrRemoveMarkers(bool add, const Any &which, const ParamList &params){
    Size s = params["size"];
    
    std::vector<int> ids = parse_list_str(which);

    for(unsigned int i=0;i<ids.size();++i){
      int x = ids[i];
      if(x<0 || x>=1296) continue;
      data->loaded[x] = add;
      if(add) data->sizes[x] = s;
    }
    
    data->loaded &= data->valid;
  }


  /// marker is 13x17 cells
  Img8u FiducialDetectorPluginICL1::createMarker(const Any &whichOne,const Size &size, const ParamList &params){
    Size size2 = size * 2;
    
    MarkerCodeICL1 code(whichOne.as<int>());
    MarkerMetricsICL1 metrics(code,size2); // we use mm-pixels here

    float dx = size2.width/13.0f;
    float dy = size2.height/17.0f;
    
    ImgQ image(size2,1);
    image.fill(255);
    color(0,0,0,0);

    fill(0,0,0,255);
    rect(image,round(dx),round(dy),round(11*dx),round(15*dy),round(dx/2));
    for(int i=0;i<4;++i){
      fill(255,255,255,255);
      rect(image,
           round(metrics.crs[i].x),
           round(metrics.crs[i].y),
           round(metrics.crs[i].width),
           round(metrics.crs[i].height),
           round(dx/2));
      fill(0,0,0,255);
      for(unsigned int j=0;j<metrics.crs[i].ccrs.size();++i){
        rect(image,
             round(metrics.crs[i].ccrs[j].x),
             round(metrics.crs[i].ccrs[j].y),
             round(metrics.crs[i].ccrs[j].width), 
             round(metrics.crs[i].ccrs[j].height),
             round(dx/2));
      }
    }
    
    image.scale(size,interpolateRA);
    return cvt8u(image);
    
  }

  
}
