#include <ICLSkinOp.h>
#include <ICLImg.h>
#include <ICLFileReader.h>
#include <ICLFileWriter.h>

using namespace icl;

int main(){
  // Variable initilization
  const ImgBase* refImg, *img;
  ImgBase *skinImg = imgNew();
  FileReader r("demoImages/skinRefImg.ppm");
  FileReader r2("demoImages/skinImg.ppm");
  FileWriter w("skinMask.pgm");
  std::vector<float> vecparameter;
  SkinOp skinFilter, skinFilter2;
  
  // --------------------------------------------------------------
  // ---- Train skin color filter 
  // --------------------------------------------------------------
  // Load skin reference image
  refImg = r.grab();
  
  // Apply skin parameter
  skinFilter.train(refImg);
  
  // --------------------------------------------------------------
  // ---- Detect skin color 
  // --------------------------------------------------------------
  // Load skin test image
  img = r2.grab();
  
  // Set parabola parameter
  vecparameter.push_back(101);
  vecparameter.push_back(65);
  vecparameter.push_back(0.1);
  vecparameter.push_back(103);
  vecparameter.push_back(88);
  vecparameter.push_back(-0.09);
  skinFilter2.setParameter(vecparameter);
  
  // Detect skin color 
  skinFilter2.apply(img, &skinImg);
  
  //Write skin mask
  w.write(skinImg);
  
  // Destroy objects and return
  return 0;
}
