ImgBase *ensureDepth(ImgBase **ppoImage, depth d){
  if(!ppoImage){
    return imgNew(d); // creates an appropriate Img<T>
  }
  if(!*ppoImage){
    // make the given instance point to a new image
    *ppoImage = imgNew(d);
  }
  else if((*ppoImage)->getDepth() != d){
    // create new instance with correct depth
    ImgBase *poNew = imgNew(d,(*ppoImage)->getParams());
    
    // delete former instance
    delete *ppoImage;
    
    // overwrite former instance
    *ppoImage = poNew;     
  }
  return *ppoImage;
}
