ImgBase *A = new Img8u(Size::VGA,formatLAB);

// depth adaption:
ImgBase *B = A.convert(depth32f);
delete A;
A = B;
