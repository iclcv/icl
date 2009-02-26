struct ImgBase{
  ImgParams params;
  Time timestamp;
  depth imagedepth;
};

template<class T>
struct Img : public ImgBase{
  std::vector<T*> data;
};
