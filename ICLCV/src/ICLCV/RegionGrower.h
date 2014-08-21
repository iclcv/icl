/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLCV/src/ICLCV/RegionGrower.h                         **
** Module : ICLCV                                                  **
** Authors: Andre Ueckermann                                       **
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

#pragma once

#include <ICLMath/FixedVector.h>
#include <ICLCore/Img.h>
#include <ICLCore/DataSegment.h>
#include <ICLMath/HomogeneousMath.h>
#include <ICLUtils/Exception.h>

namespace icl{
  namespace cv{

    /// class for region growing on images and DataSegments (e.g. poincloud xyzh)
    /** The RegionGrower class is designed as template applying a growing criterion to given input data.
        A mask defines the points for processing (e.g. a region of interest).
    */

    class RegionGrower{
      	
  	  public:
        
        /// Applies the region growing on an input image with a growing criterion
        /** @param image the input image for region growing
            @param crit the region growing criterion
            @param initialMask the initial mask (e.g. ROI)
            @param minSize the minimum size of regions (smaller regions are removed)
            @param startID the start id for the result label image
            @return the result label image
        */      
        template<class Criterion>
        const core::Img32s &apply(const core::Img8u &image, Criterion crit, core::Img8u *initialMask = 0,
                            const unsigned int minSize=0, const unsigned int startID=1){
          this->result=core::Img32s(image.getParams());
          //this->mask=Img8u(image.getParams());
          core::Img8u &useMask = initialMask ? *initialMask : this->mask;
          if(image.getChannels() == 1){
            region_grow<core::Img8u,icl8u,1, Criterion>(image, useMask, this->result, crit, minSize, startID);
          }else{
            throw utils::ICLException("wrong number of image channels");
          }
          return this->result;
        }


        /// Applies the region growing on an input data segment with a growing criterion
        /** @param dataseg the input data segment for region growing
            @param crit the region growing criterion
            @param initialMask the initial mask (e.g. ROI)
            @param minSize the minimum size of regions (smaller regions are removed)
            @param startID the start id for the result label image
            @return the result label image
        */   
        template<class Criterion>
        const core::Img32s &apply(const core::DataSegment<float,4> &dataseg, Criterion crit, core::Img8u *initialMask = 0,
                            const unsigned int minSize=0, const unsigned int startID=1){
          core::Img8u &useMask = initialMask ? *initialMask : this->mask;
          this->result.setSize(dataseg.getSize());
          this->result.setChannels(1);
          region_grow<core::DataSegment<float,4>,float,4, Criterion>(dataseg, useMask, this->result, crit, minSize, startID);
          return this->result;
        }
  
  
        /// Applies the region growing on an input data segment with euclidean distance criterion
        /** @param dataseg the input data segment for region growing
            @param mask the initial mask (e.g. ROI)
            @param threshold the maximum euclidean distance
            @param minSize the minimum size of regions (smaller regions are removed)
            @param startID the start id for the result label image
            @return the result label image
        */   
        const core::Img32s &applyFloat4EuclideanDistance(const core::DataSegment<float,4> &dataseg, core::Img8u mask, 
                            const int threshold, const unsigned int minSize=0, const unsigned int startID=1){
          return apply(dataseg, Float4EuclideanDistance(threshold), &mask, minSize, startID);
        }
  
        
        /// Applies the region growing on an input image with value-equals-threshold criterion
        /** @param image the input image for region growing
            @param mask the initial mask (e.g. ROI)
            @param threshold the equals-to-value (growing criterion)
            @param minSize the minimum size of regions (smaller regions are removed)
            @param startID the start id for the result label image
            @return the result label image
        */     
        const core::Img32s &applyEqualThreshold(const core::Img8u &image, core::Img8u mask, const int threshold, 
                            const unsigned int minSize=0, const unsigned int startID=1){
          return apply(image, EqualThreshold(threshold), &mask, minSize, startID);
        }
  
  
        /// Returns a vector of regions containing the image IDs. This is an additional representation of the result.
        /** @return the vector of regions with the image IDs.
        */     
        std::vector<std::vector<int> > getRegions(){
          return regions;
        }
  
  
      private:

        core::Img8u mask;
        core::Img32s result;
        std::vector<std::vector<int> > regions;
      
        template<class T, class DataT, int DIM>
        struct RegionGrowerDataAccessor{
          RegionGrowerDataAccessor(const T &t){};
          int w() const { return 0; }
          int h() const { return 0; }
          math::FixedColVector<DataT, DIM> operator()(int x, int y) const { return math::FixedColVector<DataT,DIM>(); }
        };
        
        
        struct EqualThreshold{
          int t;
          EqualThreshold(int t):t(t){}
          bool operator()(icl8u a, icl8u b) const{
            return (int)b == t;
          }
        };

        
        struct Float4EuclideanDistance{
          float t;
          Float4EuclideanDistance(float t):t(t){}
          bool operator()(const math::FixedColVector<float,4> &a, const math::FixedColVector<float,4> &b) const{
            return math::dist3(a,b) < t;
          }
        };
        
  
        template<class T, class DataT, int DIM, class Criterion>
        static void flood_fill(const RegionGrowerDataAccessor<T,DataT,DIM> &a, int xStart, int yStart, 
                               core::Channel8u &processed, Criterion crit, std::vector<int> &result,  core::Channel32s &result2, int id);
                               
                               
        template<class T, class DataT, int DIM, class Criterion>
        void region_grow(const T &data, core::Img8u &mask, core::Img32s &result, Criterion crit, const unsigned int minSize, const unsigned int startID=1){
          RegionGrowerDataAccessor<T,DataT,DIM> a(data);
          
          core::Img8u processed = mask;
          core::Channel8u p = processed[0];
          std::vector<int> r;
          core::Channel32s res = result[0];
          result.fill(0);

          int nextID = startID;
          regions.clear();

          std::vector<std::vector<int> > clear;

          for(int y=0;y<a.h();++y){
            for(int x=0;x<a.w();++x){
              if(!p(x,y) && crit(a(x,y),a(x,y))){
                r.clear();
                flood_fill<T,DataT,DIM,Criterion>(a ,x ,y ,p, crit, r, res, nextID++);
                if(r.size()<minSize){
                  nextID--;
                  clear.push_back(r);//delete later
                }else{
                  regions.push_back(r);//add region
                }
              }
            }
          }

          //clear regions smaller minSize
          for(unsigned int i=0; i<clear.size(); i++){
            for(unsigned int j=0; j<clear.at(i).size(); j++){
              p[clear.at(i).at(j)]=false;
              res[clear.at(i).at(j)]=0;
            }
          }
        }
      
      };
     
     
      template<>
      struct RegionGrower::RegionGrowerDataAccessor<core::Img8u, icl8u, 1>{
        const core::Channel8u c;
        RegionGrowerDataAccessor(const core::Img8u &image):c(image[0]){}
        int w() const { return c.getWidth(); }
        int h() const { return c.getHeight(); }
        math::FixedColVector<icl8u, 1> operator()(int x, int y) const { return c(x,y); }
      };

      template<>
      struct RegionGrower::RegionGrowerDataAccessor<core::Img8u, icl8u, 3>{
        core::Channel8u c[3];
        RegionGrowerDataAccessor(const core::Img8u &image){
          for(int i=0;i<3;++i){
            c[i] = ((core::Img8u&)image)[i];
          }
        }
        int w() const { return c[0].getWidth(); }
        int h() const { return c[0].getHeight(); }
        math::FixedColVector<icl8u, 3> operator()(int x, int y) const { 
          return math::FixedColVector<icl8u,3>(c[0](x,y), c[1](x,y), c[2](x,y));
        }
      };

      template<>
      struct RegionGrower::RegionGrowerDataAccessor<core::DataSegment<float,4>, float, 4>{
        core::DataSegment<float,4> data;
        int ww,hh;
        RegionGrowerDataAccessor(const core::DataSegment<float,4> &data):data(data){
          ww = data.getSize().width;
          hh = data.getSize().height;
        }
        int w() const { return ww; }
        int h() const { return hh; }
        math::FixedColVector<float,4> operator()(int x, int y) const { return data(x,y); }
      };
        
      template<class T, class DataT, int DIM, class Criterion>
      void RegionGrower::flood_fill(const RegionGrowerDataAccessor<T,DataT,DIM> &a, int xStart, int yStart, 
                               core::Channel8u &processed, Criterion crit, std::vector<int> &result,  core::Channel32s &result2, int id){
        std::vector<utils::Point> stack(1,utils::Point(xStart,yStart));
        processed(xStart,yStart) = true;//update mask
        result2(xStart,yStart) = id;//update result image           
        result.push_back(xStart+yStart*a.w());//add to region vector
        unsigned int next = 0;
        while(next < stack.size()){
          const utils::Point p = stack[next];
          next++;
          for(int dy=-1;dy<=1;++dy){
            const int y = p.y+dy;
            if(y < 0 || y >=a.h()) continue;
            for(int dx=-1;dx<=1;++dx){
              const int x = p.x+dx;
              if(x < 0 || x >=a.w()) continue;
              if(dx==0 && dy==0) continue;

              if(crit(a(p.x,p.y),a(x,y)) && processed(x,y)==false){
                stack.push_back(utils::Point(x,y));
                processed(x,y) = true;
                result2(x,y) = id;           
                result.push_back(x+y*a.w());
              }
            }
          }      
        }
      }    
     
  } // namespace cv
}
