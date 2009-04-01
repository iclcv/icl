void threshold(ImgBase *image, double t){
  switch(image->getDepth()){
#define ICL_INSTANTIATE_DEPTH(D) \
    case depth##D: threshold_t(*image->asImg<icl##D>(),(icl##D)t);
    ICL_INSTANTIATE_ALL_PEPTHS
#undef ICL_INSTANTIATE_DEPTH
    default: ICL_INVALID_DEPTH;
  }
}


