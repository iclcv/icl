#include <ICLQuick.h>

int main(){

  //ImgQ i = scale(create("parrot"),0.4);
  //ImgQ j = copy(i);


  // show(filter(i,"laplace"));
  // show(filter(i,"gauss"));

  // show(i+100);
  // show(i*i/255);

  // show(cc(i,formatHLS));

  // show(levels(cc(i,formatGray),4));
  
  /**
      ImgQ a,b;
      for(int i=0;i<10;i++){
      a = ImgQ();
      for(int j=0;j<10;j++){
      a = (a,scale(pwc(),0.1));
      }
      b=(b%a);
      }

      show(b);
  */

  ImgQ a = create("parrot");
  a = scale(a,0.2);
  
  show(norm(exp(a/100)));
  print(norm(exp(a/100)));

  
  
  return 0;
}
