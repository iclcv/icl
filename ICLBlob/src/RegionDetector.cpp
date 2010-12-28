/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2010 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLBlob/src/RegionDetector.cpp                         **
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


#include <ICLBlob/RegionDetector.h>
#include <ICLBlob/RegionDetectorTools.h>
#include <ICLBlob/ImageRegionData.h>

#include <ICLBlob/LineSegment.h>
#include <ICLBlob/WorkingLineSegment.h>
#include <ICLBlob/RunLengthEncoder.h>
#include <ICLBlob/ImageRegionPart.h>

#include <ICLUtils/Range.h>
#include <ICLUtils/StringUtils.h>

#include <algorithm>

#include <ICLUtils/StackTimer.h>

namespace icl{

  namespace{
    struct JoinRegionsIf{
      ImageRegionPart *m_oldR;
      ImageRegionPart *m_newR;
      inline JoinRegionsIf(ImageRegionPart*oldR, ImageRegionPart*newR):
        m_oldR(oldR),m_newR(newR){
      }
      inline void operator()(WorkingLineSegment &sl) const { 
        if(sl.reg == m_oldR){
          m_newR->children.push_back(m_oldR->adopt());
          sl.reg = m_newR;
        }
      }
    };
  }
  
  using namespace region_detector_tools;
  
  struct RegionDetector::Data{
    const ImgBase *image;
    Point roiOffset;
    Rect roi;
    RunLengthEncoder rle;
    
    std::vector<ImageRegionPart> parts;
    int nUsedParts;
    
    std::vector<ImageRegion> regions;
    std::vector<ImageRegion> filteredRegions;

    std::vector<ImageRegionData*> regionData;
    
    
    Data():image(0){}

    ~Data(){
      for(unsigned int i=0;i<regionData.size();++i){
        delete regionData[i];
      }
    }
    CornerDetectorCSS css;
  };


  RegionDetector::RegionDetector(bool createRegionGraph, const std::string &configurableID):Configurable(configurableID){
    m_data = new Data;

    addProperty("minimum region size","range:spinbox","[0,100000]","0");
    addProperty("maximum region size","range:spinbox","[0,100000]","1000000");
    addProperty("minimum value","range:slider","[0,255]","0");
    addProperty("maximum value","range:slider","[0,255]","255");
    addProperty("create region graph","menu","off,on",createRegionGraph ? "on" : "off");

    addChildConfigurable(&m_data->css,"CSS");
  }

  RegionDetector::RegionDetector(int minSize, int maxSize, int minVal, int maxVal, bool createRegionGraph,
                                 const std::string &configurableID):Configurable(configurableID){
    m_data = new Data;

    addProperty("minimum region size","range:spinbox","[0,100000]",str(minSize));
    addProperty("maximum region size","range:spinbox","[0,100000]",str(maxSize));
    addProperty("minimum value","range:slider","[0,255]",str(minVal));
    addProperty("maximum value","range:slider","[0,255]",str(maxVal));
    addProperty("create region graph","menu","off,on",createRegionGraph ? "on" : "off");

    addChildConfigurable(&m_data->css,"CSS");
  }

  void RegionDetector::setCreateGraph(bool on){
    setPropertyValue("create region graph", on ? "on" : "off");
  }  

  RegionDetector::~RegionDetector(){
    delete m_data;
  }

  const ImageRegion RegionDetector::click(const Point &pos){
    std::vector<ImageRegion> &rs = m_data->regions;
    for(unsigned int i=0;i<rs.size();++i){
      if(rs[i].contains(pos)){
        return rs[i];
      }
    }
    return ImageRegion(0);
  }

  void RegionDetector::setConstraints(int minSize, int maxSize, int minVal, int maxVal){
    setPropertyValue("minimum region size",str(minSize));
    setPropertyValue("maximum region size",str(maxSize));
    setPropertyValue("minimum value",str(minVal));
    setPropertyValue("maximum value",str(maxVal));
  }
  
  void RegionDetector::setCSSParams(float angle_thresh,
                                    float rc_coeff, 
                                    float sigma, 
                                    float curvature_cutoff, 
                                    float straight_line_thresh){
    m_data->css.setAngleThreshold(angle_thresh);
    m_data->css.setRCCoeff(rc_coeff);
    m_data->css.setSigma(sigma);
    m_data->css.setCurvatureCutoff(curvature_cutoff);
    m_data->css.setStraightLineThreshold(straight_line_thresh);
  }                                             


  void RegionDetector::useImage(const ImgBase *image) throw (ICLException){
    ICLASSERT_THROW(image->getDepth() != depth32f && image->getDepth() != depth64f, 
                    ICLException("RegionDetector::prepareImage: depth32f and depth64f are not supported"));
    m_data->roi = image->getROI();
    m_data->image = image;
  }
  
  void RegionDetector::analyseRegions(){
    //BENCHMARK_THIS_FUNCTION;
    int W = m_data->roi.width, H = m_data->roi.height;
    if((int)m_data->parts.size() != W*H){
      m_data->parts.resize(W*H);
    }
    
    ImageRegionPart *nextReg = m_data->parts.data()-1;
    RunLengthEncoder &rle = m_data->rle;
    
    // start region-detection on scan-line basis
    // first line: each sl gets a new region
    for(WorkingLineSegment *s=rle.begin(0); s != rle.end(0); ++s){
      s->reg = (++nextReg)->init(s); 
    }

    for(int y=1;y<H;++y){
      WorkingLineSegment *l = rle.begin(y-1); 
      WorkingLineSegment *c = l+W;
      
      WorkingLineSegment *lEnd = rle.end(y-1);
      WorkingLineSegment *cEnd = rle.end(y);
      WorkingLineSegment *cStart = c;
      while(c < cEnd){
        do{
          if( (l->val == c->val) && (c->reg != l->reg) ){
            if(!c->reg){
              c->reg = l->reg; // just copy the label if only one is equal
              c->reg->segments.push_back(c);
            }else{
              JoinRegionsIf joiner(l->reg,c->reg);
              std::for_each(l,lEnd,joiner);
              std::for_each(cStart,c,joiner);
            }
          }
        }while(c->xend > l->xend && ++l);

        if(!c->reg){
          c->reg = (++nextReg)->init(c); 
        }
        if(c->xend == l->xend) ++l;
        ++c;
      }
    }
    ++nextReg;
    m_data->nUsedParts =  (int)(nextReg-m_data->parts.data());
  }

  void RegionDetector::joinRegions(){
    //BENCHMARK_THIS_FUNCTION;
    /// clear former data regions and their data
    m_data->regions.clear();
    m_data->filteredRegions.clear();
    for(unsigned int i=0;i<m_data->regionData.size();++i){
      delete m_data->regionData[i];
    }
    m_data->regionData.clear();
    
    bool crg = getPropertyValue("create region graph") == "on";
    
    int nextID = -1;
    ImageRegionPart *parts = m_data->parts.data();
    ImageRegionPart *partsEnd = parts + m_data->nUsedParts;
    for(ImageRegionPart *p=parts; p != partsEnd; ++p){
      if(p->is_top()){
        m_data->regionData.push_back(ImageRegionData::createInstance(&m_data->css,p,++nextID, crg, m_data->image));
        m_data->regions.push_back(ImageRegion(m_data->regionData.back()));
      }
    }
  }
  
  void RegionDetector::linkRegions(){
    //BENCHMARK_THIS_FUNCTION;
    RunLengthEncoder &rle = m_data->rle;
    const int H = m_data->roi.height;
    
    for(WorkingLineSegment *s=rle.begin(0)+1; s < rle.end(0); ++s){
      s->ird->link(s[-1].ird);
    }
    
    for(int y=1;y<H;++y){
      //    DEBUG_LOG("y:" << y << " num sl for y: " << (int)(slEnds[y] - (sldata + w*y) ));
      WorkingLineSegment *l = rle.begin(y-1);
      WorkingLineSegment *c = rle.begin(y);
      
      //    SL *lEnd = slEnds[y-1];
      WorkingLineSegment *cEnd = rle.end(y);
      // SL *cStart = c;
      
      while(c < cEnd){
        do{
          l->ird->link(c->ird);
          //std::cout << "linking " << *l << " <--> " << *c << std::endl;
        }while(c->xend > l->xend && ++l);
        
        if(c->xend == l->xend) ++l;
        
        if(++c != cEnd){
          //std::cout << "linking " << c[-1]  << " <--> " << *c << std::endl;
          (c-1)->ird->link(c->ird);
        }
      }
    }
  }




  void RegionDetector::setUpBorders(){
    //BENCHMARK_THIS_FUNCTION;
    RunLengthEncoder &rle = m_data->rle;
    const int H = m_data->roi.height;
    
      
    // first row
    for(WorkingLineSegment *s = rle.begin(0); s < rle.end(0); ++s){
      s->ird->graph->isBorder = true;
    }
    // last row
    for(WorkingLineSegment *s = rle.begin(H-1); s < rle.end(H-1); ++s){
      s->ird->graph->isBorder = true;
    }
    // first and last column
    for(int y=1;y<H-1;++y){
      rle.begin(y)->ird->graph->isBorder = true;
      (rle.end(y)-1)->ird->graph->isBorder = true;
    }
    
    // detection of children is no longer done here
    // children and surrounding regions are detected
    // on demand in the ImageRegion
  }
  
  struct SimpleRegionFilter{
    int minVal,maxVal,minSize,maxSize;
    inline SimpleRegionFilter(int minVal, int maxVal, int minSize, int maxSize):
      minVal(minVal),maxVal(maxVal),minSize(minSize),maxSize(maxSize){}
    inline bool operator()(const ImageRegion &r) const{
      int val = r.getVal();
      if(val > maxVal || val < minVal) return false;
      int size = r.getSize();
      if(size > maxSize || size < minSize) return false;
      return true;
    }
  };
  
  void RegionDetector::filterRegions(){
    //BENCHMARK_THIS_FUNCTION;
    m_data->filteredRegions.clear();
    copy_if(m_data->regions.begin(),m_data->regions.end(),
            std::back_inserter(m_data->filteredRegions),
            SimpleRegionFilter(parse<int>(getPropertyValue("minimum value")),
                               parse<int>(getPropertyValue("maximum value")),
                               parse<int>(getPropertyValue("minimum region size")),
                               parse<int>(getPropertyValue("maximum region size"))
                               ));
  }
  
  const std::vector<ImageRegion> &RegionDetector::detect(const ImgBase *image){
    //BENCHMARK_THIS_FUNCTION;
    useImage(image);

    // run length encoding
    m_data->rle.encode(image);

    // find all image region parts
    analyseRegions();

    // join parts and create image regions
    joinRegions();

    if(getPropertyValue("create region graph") == "on"){
      // create connectivity graph
      linkRegions();

      // which region contains which other region
      setUpBorders();
    }      

    // removed regions that do not match the given filter criteria
    filterRegions();
    
    return m_data->filteredRegions;
  }

}

