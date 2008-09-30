#include <iclFileWriter.h>
#include <iclQuick.h>
#include <string>

int main(int n, char **ppc){
  std::string s = (n>1) ? ppc[1] : "ppm";
  
  ImgQ image = gray(scale(create("parrot"),0.4));
  
  FileWriter(std::string("image.")+s).write(&image);
}
