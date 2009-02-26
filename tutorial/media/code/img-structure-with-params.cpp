template<class T>
struct Img{
  ImgParams params;
  Time timestamp;
  depth imagedepth;
  std::vector<T*> data;
};
