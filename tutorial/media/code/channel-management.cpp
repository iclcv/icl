/// create a 320x240 YUV-image
Img32f A(Size::QVGA,formatYUV);

/// create an empty image
Img32f B;

/// appending the Y-channel from A
B.appendChannel(&a,0);

/// or even simplier
Img32f D = B.extractChannelImg(0);

//-------------------------------------------------------

/// creates a 320x240 gray image
Img8u C(Size(320,240),formatGray);

/// (try to) append a channel of an Img32f 
C.appendChannel(&a,1); // compile ERROR -> depth is wrong

//-------------------------------------------------------

/// create an Img32f of differenct size
Img32f D(Size::VGA,formatGray);

/// (try to) append a channel of different size
C.append(&a,1); // runtime ERROR -> incompatible sizes

//-------------------------------------------------------

/// create a short-image of size 1920x1080
Img16s E(Size::HD1080,formatRGB);

/// swap red- and blue- channel
E.swapChannels(0,2);

//-------------------------------------------------------

/// create a std::vector of VGA images
std::vector<Img8u> V(5,Img8u(Size::VGA,formatRGB));
/// (all images share their channels)

/// create a multi-channel image
Img8u accu;
for(unsigned int i=0;i<V.size();++i){
  V[i].detach();      // make V[i] independent
  accu.append(&V[i]); // appends all channels
}




