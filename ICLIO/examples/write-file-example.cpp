#include <ICLIO/FileWriter.h>
#include <ICLQuick/Quick.h>

int main(int n, char **ppc){
  std::string s = (n>1) ? ppc[1] : "ppm";
  
  ImgQ image = gray(scale(create("parrot"),0.4));
  
  FileWriter(std::string("image.")+s).write(&image);
  
  std::cout << "wrote file image." << s << std::endl;
}
