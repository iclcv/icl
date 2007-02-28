#include <ICLQuick.h>

#include <QFont>
#include <QFontMetrics>
#include <QApplication>

int main(int nargs, char **ppc){

  ImgQ A = create("flowers");
  A = scale(A,0.2);
  A = gray(A);
  A = levels(A,5);

  show(norm(A*A*A));
  

  /*
      
      ImgQ im = scale(create("parrot"),0.4);  
      
      for(int i=0;i<10;i++){
      for(int j=0;j<10;j++){
      fontsize(3*i+j);
      color(255-20*i,10+20*i,255-20*j,100+5*i+5*j);
      char buf[100];
      sprintf(buf,"[%d]",i+j);
      text(im,20*i,20*j,buf);
      }
      }
      show(im);
      */
      
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

  //ImgQ a = create("parrot");
  //a = scale(a,0.2);
  //
  //show(norm(exp(a/100)));
  //print(norm(exp(a/100)));

  
  /*
      for(int i=0;i<100;i++){
      show(pwc());
      usleep(1000*100);
      system("killall xv");
      }
  */
  
  return 0;
}
