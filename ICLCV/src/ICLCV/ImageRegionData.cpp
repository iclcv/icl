/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLCV/src/ICLCV/ImageRegionData.cpp                    **
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


#include <ICLCV/ImageRegionData.h>
#include <ICLCV/ImageRegionPart.h>

using namespace icl::utils;
using namespace icl::core;

namespace icl{
  namespace cv{
  
  
    struct TransformLinesegAndSetImageRegionData{
      ImageRegionData *d;
      TransformLinesegAndSetImageRegionData(ImageRegionData *d) : d(d){}
      const LineSegment &operator()(WorkingLineSegment *in){
        in->ird = d;
        return *in;
      }
    };
  
    static unsigned int collect(ImageRegionPart *r, LineSegment *s, ImageRegionData *ird){
      if(r->is_collected()) return 0;
      r->notify_collected();
      LineSegment *sSave = s;
      
      std::transform(r->segments.begin(),r->segments.end(),s,TransformLinesegAndSetImageRegionData(ird));
      
      s += r->segments.size();
      
      for(ImageRegionPart::children_container::iterator it = r->children.begin(); it != r->children.end(); ++it){
        s += collect(*it,s,ird);
      }
      
      return (unsigned int)(s - sSave);
    }
  
  
    static unsigned int count(ImageRegionPart *r){
      if(r->is_counted()) return 0;
      r->notify_counted();
      
      unsigned int n = r->segments.size();
      for(ImageRegionPart::children_container::iterator it = r->children.begin(); it != r->children.end(); ++it){
        n += count(*it);
      }
      return n;
    }
    
    ImageRegionData * ImageRegionData::createInstance(CornerDetectorCSS *css, ImageRegionPart *topRegionPart, int id, bool createGraphInfo, const ImgBase *image){
      ImageRegionData *data = new ImageRegionData(css,topRegionPart->val,id,count(topRegionPart), createGraphInfo, image);
      collect(topRegionPart, data->segments.data(),data);
      return data;
    }
  
  
  
    
    void ImageRegionData::showTree(int indent) const{
      ICLASSERT_RETURN(graph);
      for(int i=0;i<indent-1;++i) std::cout << "   ";
      if(indent)  std::cout << "|--";
      std::cout << id << std::endl;
      for(unsigned int i=0;i<graph->children.size();++i){
        graph->children.operator[](i)->showTree(indent+1);
      }
    }
    void ImageRegionData::showWithNeighbours() const{
      ICLASSERT_RETURN(graph);
      std::cout << "neighbours of region " << id << ':'<< std::endl;
      for(std::set<ImageRegionData*>::const_iterator it = graph->neighbours.begin(); it != graph->neighbours.end(); ++it){
        if(!*it) {
          std::cout << "\t" << "Border" << std::endl;
        }else{
          std::cout << "\t" << (*it)->id << std::endl;
        }
      }
    }
  } // namespace cv
}
