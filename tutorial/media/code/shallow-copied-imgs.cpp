/// creates a rgb image of size 640x480
Img8u A(Size::VGA,formatRGB);

/// create another view on A's pixel data
Img8u B(A); // equal to Img8u B=A;

/// create a null-image
Img8u C;

/// let C reference A's data
C = A;  // assignment operator

/// make C independent (deep copy of data)
C.detach();

/// create a deep copy directly
Img8u *p = A.deepCopy();
