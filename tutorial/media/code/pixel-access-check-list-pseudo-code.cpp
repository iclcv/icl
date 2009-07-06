Decision what_to_use(Programmer &U,Function &F,IPP &I){
  if(I.hasFunctionFor(F) && U.wantsToUse("IPP")){
    return U.use("IPP-function");
  }
  
  if(U.hasExternalFunctionFor(F) && U.wantsToUse("it")){
    return U.use("raw data access for external function");
  }
    
  if(U.need("random (x,y)-access")){
    if(F.is("time-critical")){
      return U.use("Image channel");
    }else{
      if(F.needs("full pixel access")) {
        return U.use("Pixel Ref");
      }else{
        return U.use("(x,y,channel)-operator");
      }
    }
  }else{ // linear access only
    switch(U.likes("templates")){
      case aLot:   
        return U.use("forEach etc..");
      case aLittleBit:
        if(F.needs("ROI-Support")){
          return U.use("ROI-Iterators");
        }else{
          return U.use("Iterators");
        }
      case notAtAll:
        if(U.likes("Raw-Data access")){
          return U.use("raw data access");
        }else{
          return U.use("liner image channel");
        }
    }
  }
}
