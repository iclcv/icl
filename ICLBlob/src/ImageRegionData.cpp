/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2010 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLBlob/src/ImageRegionData.cpp                        **
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


#include <ICLBlob/ImageRegionData.h>
#include <ICLBlob/ImageRegionPart.h>

namespace icl{


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
}
