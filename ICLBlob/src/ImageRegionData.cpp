#include <ImageRegionData.h>
#include <ImageRegionPart.h>

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
  
  ImageRegionData * ImageRegionData::createInstance(ImageRegionPart *topRegionPart, int id, bool createGraphInfo, const ImgBase *image){
    ImageRegionData *data = new ImageRegionData(topRegionPart->val,id,count(topRegionPart), createGraphInfo, image);
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
