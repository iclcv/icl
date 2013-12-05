#include <ICLCore/Img.h>
#include <ICLQt/Quick.h>

int main(){
   // create an empty image with byte pixels
   Img8u byteImage;

   // create float image with VGA size with 3-channel RGB-format
   Img32f floatImage(Size(640,480),formatRGB);

   // set all pixels of channel 0 to 255
   std::fill(floatImage.begin(0),floatImage.end(0),255);

   // assign a demo image to 'floatImage' (the parrot-image is too
   // large for now, so we scale it's size by factor 0.3)
   floatImage = scale(create("parrot"),0.3);

   // convert the float image into the former byteImage
   // the byte image is adapted automatically
   floatImage.convert(&byteImage);
   
   // create a shallow copy of 'byteImage'
   Img8u byteImage2 = byteImage;

   // byteImage2 and byteImage now share their image data, so
   // changes on byteImage2s pixel-data will also effect the pixel-data
   // of byteImage. The image can be made independent by doing this:
   byteImage.detach();
   
   // now lets access the image pixels for a simple thresholding application
   // note: the image pixel data is organized channel-wise as independend 
   // row-major ordered data blocks 
   for(int c=0;c<byteImage.getChannels();++c){
      for(int x=0;x<byteImage.getWidth();++x){
         for(int y=0;y<byteImage.getHeight();++y){
            byteImage(x,y,c) = 255 * (byteImage(x,y,c)>128);
         } 
      }
   }

   // finally, we want to see the result
   show(byteImage);
}
