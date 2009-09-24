// Simple vector implementation
template<class T>
class Vector{
  unsigned int dim; // vector dimension
  T *data; // data storage
  // utility function for save allocation
  static T *alloc(unsigned int dim){
    return dim?new T[dim]:0;
  }
  // utility function for save 'freeing'
  static void release(T *t){
    if(t) delete t;
  }
  // utility function for copying data
  static void cpy(const T *src, T *dst,unsigned int dim){
    for(unsigned int i=0;i<dim;++i){
      dst[i] = src[i];
    }
  }
public:
  
  // save default constructor 
  Vector(unsigned int dim=0):
    dim(dim),data(alloc(dim)){
  }
  // save destructor
  ~Vector(){
    release(data);
  }
  // copy constructor which is necessary because of
  // the dynamic data we use
  Vector(const Vector &v):
    dim(v.dim),data(alloc(dim)){
    cpy(v.data,data,dim);
  }
  
  // index operator (r/w-access)
  T &operator [](unsigned int idx){
    return data[idx];
  }

  // index operator (read-only -> in const case)
  const T &operator [](unsigned int idx) const{
    return data[idx];
  }
  
  // vector dimension
  unsigned int getDim() const { 
    return dim; 
  }
  
  // assignment operator (also mandatory because of
  // the dynamic data
  Vector &operator=(const Vector &v){
    if(this == &v) return *this; // important!
    if(dim != v.dim){
      release(data);
      data = alloc(v.dim);
      dim = v.dim;
    }
    cpy(v.data,data,dim);
    return *this;
  }
};

int main(){
  Vector<float> v(4);
  Vector<float> v2(6);

  v = v2;
}
