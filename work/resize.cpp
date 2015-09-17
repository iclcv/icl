#include <ICLQt/Common.h>

VBox gui;
GenericGrabber g;


inline icl8u pix(const Channel8u &c, float x, float y){
  int px = (int)x; // floor of x
  int py = (int)y; // floor of y

  icl8u p1 = c(px,py);
  icl8u p2 = c(px+1,py);
  icl8u p3 = c(px,py+1);
  icl8u p4 = c(px+1,py+1);
  
  // Calculate the weights for each pixel
  float fx = x - px;
  float fy = y - py;
  float fx1 = 1.0f - fx;
  float fy1 = 1.0f - fy;
  
  int w1 = fx1 * fy1 * 256.0f;
  int w2 = fx  * fy1 * 256.0f;
  int w3 = fx1 * fy  * 256.0f;
  int w4 = fx  * fy  * 256.0f;
 
  // Calculate the weighted sum of pixels (for each color channel)
  unsigned int out = p1 * w1 + p2 * w2 + p3 * w3 + p4 * w4;

  return (out  >> 8)& 0xff;
}


inline icl8u pixfix(const Channel8u &c, float x, float y){
  const unsigned int shift = 8; // shift can have values 8 to 16
  const unsigned int fixed = 1<<shift;
  unsigned int Fx = (unsigned int) (x * fixed); // convert to Fixed
  unsigned int Fy = (unsigned int) (y * fixed); // convert to Fixed
  unsigned int px = (Fx & -fixed)>>shift;
  unsigned int py = (Fy & -fixed)>>shift;

  icl8u p1 = c(px,py);
  icl8u p2 = c(px+1,py);
  icl8u p3 = c(px,py+1);
  icl8u p4 = c(px+1,py+1);
  
  
  unsigned int fx = Fx & (fixed-1);
  unsigned int fy = Fy & (fixed-1);
  unsigned int fx1 = fixed - fx;
  unsigned int fy1 = fixed - fy;

  unsigned int w1 = (fx1 * fy1) >> shift;
  unsigned int w2 = (fx * fy1) >> shift;
  unsigned int w3 = (fx1 * fy ) >> shift;
  unsigned int w4 = (fx * fy ) >> shift;
  
  // Calculate the weighted sum of pixels (for each color channel)
  unsigned int out = (p1 * w1 + p2 * w2 + p3 * w3 + p4 * w4) >> shift;

  return out & 0xff;
 }


const Img8u &scale(const Img8u &src, const Size &size){
  static Img8u dst(size,1);
  dst.setSize(size);
  const Channel8u s = src[0];
  Channel8u d = dst[0];
  const float fx = float(src.getWidth()-1)/float(size.width-1);
  const float fy = float(src.getHeight()-1)/float(size.height-1);
  for(int y=0;y<size.height-1;++y){
    for(int x=0;x<size.width-1;++x){
      d(x,y) = pix(s,x*fx,y*fy);
    }
  }
  return dst;
}

struct Pre{
  unsigned int p,f,f1;
};

const Img8u &scale_fix(const Img8u &src, const Size &size){
  static Img8u dst(size,1);
  dst.setSize(size);
  const Channel8u s = src[0];
  Channel8u d = dst[0];
  const float fx = float(src.getWidth()-1)/float(size.width-1);
  const float fy = float(src.getHeight()-1)/float(size.height-1);

  const unsigned int shift = 8; // shift can have values 8 to 16
  const unsigned int fixed = 1<<shift;
  std::vector<Pre> pre_x(size.width), pre_y(size.height);

  unsigned int F = 0;
  for(size_t x=0;x<size.width;++x){
    Pre &p = pre_x[x];
    F = (unsigned int)((x * fx) * fixed);
    p.p = (F & -fixed) >> shift;
    p.f = F & (fixed-1);
    p.f1 = fixed - p.f;
  }
  for(size_t y=0;y<size.height;++y){
    Pre &p = pre_y[y];
    F = (unsigned int)((y*fy) * fixed);
    p.p = (F & -fixed) >> shift;
    p.f = F & (fixed-1);
    p.f1 = fixed - p.f;
  }

  const icl8u *data = &s[0];
  
  for(int y=0;y<size.height-1;++y){
    const Pre &py = pre_y[y];
    //const icl8u *datarow = data + py.p * size.width;
    
    for(int x=0;x<size.width-1;++x){
      const Pre &px = pre_x[x];

      // ??
      //const icl8u *p = datarow + px.p;
      //icl8u p1 = p[0], p2 = p[1], p3 = p[size.width], p4 = p[size.width+1];
      icl8u p1 = s(px.p,py.p);
      icl8u p2 = s(px.p+1,py.p);
      icl8u p3 = s(px.p,py.p+1);
      icl8u p4 = s(px.p+1,py.p+1);
      
      unsigned int w1 = (px.f1 * py.f1) >> shift;
      unsigned int w2 = (px.f * py.f1) >> shift;
      unsigned int w3 = (px.f1 * py.f) >> shift;
      unsigned int w4 = (px.f * py.f) >> shift;

      /* sse: 
          A = load left colum,
          B = load right column
          C = A*B
          D = C >> shift
          E = load p1,p2,p3,p4
          D = <D.E>
          res = get D >> shift ...
       */
      
      //unsigned int out = (p1 * w1 + p2 * w2 + p3 * w3 + p4 * w4) >> shift;
      //d(x,y) = out & 0xff;
      d(x,y) = ( (p1 * w1 + p2 * w2 + p3 * w3 + p4 * w4) >> shift) & 0xff;
    }
  }
  return dst;
}




void init(){
  gui << Image().minSize(32,24).handle("image")
      << ( HBox().maxSize(99,2)
           << Combo("QVGA,VGA,!1280x960,1920x1080").handle("size")
           << Label("").handle("time")
           << Combo("scale,manual,!fixed").handle("mode")
           )
      << Show();
  g.init(pa("-i"));
  g.useDesired(depth8u);
  g.useDesired(Size(160,120));
  g.useDesired(formatGray);
}

void run(){
  const Img8u &image = *g.grab()->as8u();
  Size s = parse<Size>(gui["size"].as<std::string>());
  std::string mode = gui["mode"];
  if(mode == "scale"){
    static Img8u dst(s,1);
    dst.setSize(s);
    Time t = Time::now();
    image.scaledCopy(&dst,interpolateLIN);
    gui["time"] = str(t.age().toMilliSecondsDouble()) + " ms";
    gui["image"] = dst;
  }else if (mode == "manual"){
    Time t = Time::now();
    const Img8u &dst = scale(image,s);
    gui["time"] = str(t.age().toMilliSecondsDouble()) + " ms";
    gui["image"] = dst;
  }else{
    Time t = Time::now();
    const Img8u &dst = scale_fix(image,s);
    gui["time"] = str(t.age().toMilliSecondsDouble()) + " ms";
    gui["image"] = dst;
  }
}

int main(int n, char **ppc){
  return ICLApp(n,ppc,"-input|-i(2)",init,run).exec();
}
