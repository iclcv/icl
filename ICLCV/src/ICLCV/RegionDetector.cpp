/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLCV/src/ICLCV/RegionDetector.cpp                     **
** Module : ICLCV                                                  **
** Authors: Christof Elbrechter, Erik Weitnauer                    **
**                                                                 **
**                                                                 **
** GNU LESSER GENERAL PUBLIC LICENSE                               **
** This file may be used under the terms of the GNU Lesser General **
** Public License version 3.0 as published by the                  **
**                                                                 **
** Free Software Foundation and appearing in the file LICENSE.LGPL **
** included in the packaging of this file.  Please review the      **
** following information to ensure the license requirements will   **
** be met: http://www.gnu.org/licenses/lgpl-3.0.txt                **
**                                                                 **
** The development of this software was supported by the           **
** Excellence Cluster EXC 277 Cognitive Interaction Technology.    **
** The Excellence Cluster EXC 277 is a grant of the Deutsche       **
** Forschungsgemeinschaft (DFG) in the context of the German       **
** Excellence Initiative.                                          **
**                                                                 **
********************************************************************/

#include <ICLCV/RegionDetector.h>
#include <ICLCV/RegionDetectorTools.h>
#include <ICLCV/ImageRegionData.h>

#include <ICLCV/LineSegment.h>
#include <ICLCV/WorkingLineSegment.h>
#include <ICLCV/RunLengthEncoder.h>
#include <ICLCV/ImageRegionPart.h>

#include <ICLUtils/Range.h>
#include <ICLUtils/StringUtils.h>

#include <algorithm>

#include <ICLUtils/StackTimer.h>

using namespace icl::utils;
using namespace icl::core;
namespace icl{
  namespace cv{
  
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
  
      addProperty("minimum region size","range:spinbox","[0,100000]","0",0,
                  "Minimum amount of pixels, detection regions must have.");
      addProperty("maximum region size","range:spinbox","[0,100000]","1000000",0,
                  "Maximum amount of pixels, detected regions must have.");
      addProperty("minimum value","range:slider","[0,255]","0",0,
                  "Minimum pixel value for detected regions.");
      addProperty("maximum value","range:slider","[0,255]","255",0,
                  "Maximum pixel value for detected regions.");
      addProperty("create region graph","menu","off,on",createRegionGraph ? "on" : "off", 0,
                  "If this property is set to 'on', a region\n"
                  "conectivity graph is created in the region\n"
                  "detection step. This graph is used to find\n"
                  "region neighbours, children (fully contained\n"
                  "regions) and parents.");
  
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
      addProperty("track times.on","flag","",false);
      addProperty("track times.rle","info","","-");
      addProperty("track times.analyse regions","info","","-");
      addProperty("track times.join regions","info","","-");
      addProperty("track times.create graph","info","","-");
      addProperty("track times.filter regions","info","","-");
      addProperty("track times.total","info","","-");
  
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
    
    const std::vector<ImageRegion> & RegionDetector::getLastDetectedRegions(){
    	return m_data->filteredRegions;
    }

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
    
    
    inline std::string msec_string_and_reset(Time &t){
      Time ct = Time::now();
      Time dt = ct - t;
      float msec = (float)dt.toMilliSecondsDouble();
      t =  ct;
      return str( 0.01*(int)(msec * 100) );
    }
    
    const std::vector<ImageRegion> &RegionDetector::detect(const ImgBase *image){
      bool trackTimes = getPropertyValue("track times.on");

      Time t,tTotal;
      
      if(trackTimes){
        tTotal = Time::now();
      }
       
      useImage(image);
  
      if(trackTimes){
        t = Time::now();
        tTotal = t;
      }
      // run length encoding
      m_data->rle.encode(image);

      if(trackTimes){
        setPropertyValue("track times.rle", msec_string_and_reset(t));
      }
  
      // find all image region parts
      analyseRegions();

      if(trackTimes){
        setPropertyValue("track times.analyse regions", msec_string_and_reset(t));
      }

  
      // join parts and create image regions
      joinRegions();

      if(trackTimes){
        setPropertyValue("track times.join regions", msec_string_and_reset(t));
      }

  
      if(getPropertyValue("create region graph") == "on"){
        // create connectivity graph
        linkRegions();
  
        // which region contains which other region
        setUpBorders();


        if(trackTimes){
          setPropertyValue("track times.create graph", msec_string_and_reset(t));
        }

      }      
  
      // removed regions that do not match the given filter criteria
      filterRegions();


      if(trackTimes){
        setPropertyValue("track times.filter regions", msec_string_and_reset(t));
        setPropertyValue("track times.total", msec_string_and_reset(tTotal));
      }
      
      
      return m_data->filteredRegions;
    }
  
    REGISTER_CONFIGURABLE_DEFAULT(RegionDetector)
  } // namespace cv
}

