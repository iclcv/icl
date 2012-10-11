#include <ICLQt/Common.h>
#include <ICLUtils/Random.h>


template<int CAPACITY=4, int ALLOC_CHUNK_SIZE=1024>
class QuadTree{
public:
  struct AABB{
    Point32f center;
    Size32f halfSize;
    AABB(){}
    AABB(const Rect &rect):
      center(rect.x + rect.width/2,
             rect.y + rect.height/2),
      halfSize(rect.width/2, rect.height/2){
    }
    AABB(const Point32f &center, const Size32f &halfSize):
      center(center),halfSize(halfSize){}
    
    bool containsPoint(const Point32f &p) const{
      return ( p.x >= center.x - halfSize.width
               && p.y >= center.y - halfSize.height
               && p.x <= center.x + halfSize.width
               && p.y <= center.y + halfSize.height);
    }
    
    bool intersectsAABB(const AABB &o) const{
      return  fabs(center.x - o.center.x) < (halfSize.width + o.halfSize.width)
      && fabs(center.y - o.center.y) < (halfSize.height + o.halfSize.height);
    }
    
    Rect32f getRect() const {
      return Rect32f(center.x - halfSize.width,
                     center.y - halfSize.height,
                     halfSize.width*2,
                     halfSize.height*2);
    }
  };

  struct Node{
    AABB boundary;
    Point32f points[CAPACITY];
    Point32f *next;
    Node *children;
    Node(){}
    Node(const AABB &boundary){
      init(boundary);
    }
    void init(const AABB &boundary){
      this->boundary = boundary;
      this->next = this->points;
      this->children = 0;
    }
    
    void queryRange(const AABB &range, std::vector<Point32f> &found) const{
      if (!boundary.intersectsAABB(range)){
        return;
      }
      for(const Point32f *p=points; p<next ; ++p){
      if(range.containsPoint(*p)){
        found.push_back(*p);
      }
      }
      if(!children) return;
      
      children[0].queryRange(range,found);
      children[1].queryRange(range,found);
      children[2].queryRange(range,found);
      children[3].queryRange(range,found);
    }

    void createChildren(Node *children){
      const Size32f half = boundary.halfSize*0.5;
      const Point32f &c = boundary.center;
      this->children = children;
      this->children[0].init(AABB(Point32f(c.x-half.width,c.y-half.height),half));
      this->children[1].init(AABB(Point32f(c.x+half.width,c.y-half.height),half));
      this->children[2].init(AABB(Point32f(c.x-half.width,c.y+half.height),half));
      this->children[3].init(AABB(Point32f(c.x+half.width,c.y+half.height),half));
    }

    void vis(PlotWidget &p) const{
      p.rect(boundary.getRect());
      if(children){
        children[0].vis(p);
        children[1].vis(p);
        children[2].vis(p);
        children[3].vis(p);
      }
    }
  };

  struct Allocator{
    Allocator(){
      grow();
    }
    std::vector<Node*> allocated;
    int curr;
    
    void grow(){
      allocated.push_back(new Node[ALLOC_CHUNK_SIZE*4]);
      curr = 0;
    }
      
    Node *alloc(){
      if(curr == ALLOC_CHUNK_SIZE) grow();
      return allocated.back()+4*curr++;
    }
    
    ~Allocator(){
      for(size_t i=0;i<allocated.size();++i){
        delete [] allocated[i];
      }
    }
  };

  Node *root;
  Allocator alloc;
  
  QuadTree(const AABB &boundary){
    this->root = new Node(boundary);
  }

  QuadTree(const Size &size){
    this->root = new Node(AABB(Point32f(size.width/2,size.height/2), 
                               Size32f(size.width/2,size.height/2)));
  }

  ~QuadTree(){
    // the root node is not part of the allocator
    delete root;
  }
  
  const Point32f &findNN(const Point32f &p) const{
    // 4times as fast as recursive version
    std::vector<const Node*> stack;
    stack.reserve(16);
    stack.push_back(root);
    float currMinDist = Range32f::limits().maxVal/2;
    const Point32f *currNN  = 0;
    while(stack.size()){
      const Node *n = stack.back();
      stack.pop_back();
      if(n->children){
        const AABB b(p, Size32f(currMinDist,currMinDist));
        if(b.intersectsAABB(n->children[0].boundary)) stack.push_back(n->children);
        if(b.intersectsAABB(n->children[1].boundary)) stack.push_back(n->children+1);
        if(b.intersectsAABB(n->children[2].boundary)) stack.push_back(n->children+2);
        if(b.intersectsAABB(n->children[3].boundary)) stack.push_back(n->children+3);
      }
      float sqrMinDist = sqr(currMinDist);
      for(const Point32f *x=n->points; x < n->next; ++x){
        float dSqr = sqr(x->x-p.x) + sqr(x->y-p.y);
        if(dSqr < sqrMinDist){
          sqrMinDist = dSqr;
          currNN = x; 
        }
      }
      currMinDist = sqrt(sqrMinDist);
    }
    if(!currNN) throw ICLException("no nn found for given point " + str(p));
    return *currNN;
  }

  void insert(const Point32f &p){
    Node *n = root;
    while(true){
      if(n->next != n->points+CAPACITY){
        *n->next++ = p;
        return;
      }
      if(!n->children) n->createChildren(alloc.alloc());
      n = (n->children + (p.x > n->boundary.center.x) + 2 * (p.y > n->boundary.center.y));
    }
  }


  std::vector<Point32f> queryRange(const AABB &range) const{
    std::vector<Point32f> found;
    root->queryRange(range,found);
    return found;
  }


  void vis(PlotWidget &p) const{
    root->vis(p);
  }
};

void init(){
  HBox gui;
  
  gui << Plot().handle("plot").minSize(64*0.7,48*0.7) << Show();
  
  PlotHandle plot = gui["plot"];
  
  //GRandClip rx(320,3*32, Range64f(0,640));
  //GRandClip ry(240,3*24, Range64f(0,480));
  URand rx(0,639), ry(0,479);
  QuadTree<32,1024> t(Size::VGA);
  
  std::vector<Point32f> ps(100*1000);
  for(size_t i=0;i<ps.size();++i){
    ps[i] = Point32f(rx,ry);
  }
  tic();
  for(size_t i=0;i<ps.size();++i){
    t.insert(ps[i]);
  }
  toc();
  for(size_t i=0;i<ps.size();++i){
    plot->circle(ps[i].x,ps[i].y,1);
  }
  
  Rect r(100,100,500,350);
  tic();
  for(int i=0;i<1000;++i){
    ps = t.queryRange(r);
  }
  toc();

  plot->color(0,255,0);
  plot->rect(r);
  plot->fill(0,255,0,100);
  for(size_t i=0;i<ps.size();++i){
    plot->circle(ps[i].x, ps[i].y, 1);
  }
  
  plot->setDataViewPort(Rect32f(0,0,640,480));
  plot->setPropertyValue("tics.x-distance",50);
  plot->setPropertyValue("tics.y-distance",50);

  plot->nofill();
  plot->color(0,100,255);
  t.vis(**plot);

  plot->color(255,100,0);
  
  ps.resize(1000);
  for(size_t i=0;i<ps.size();++i){
    ps[i] = Point32f(rx,ry);
  }
  std::vector<Point32f> nn(ps.size());

  tic();
  for(size_t i=0;i<ps.size();++i){
    nn[i] = t.findNN(ps[i]);
  }
  toc();

  for(size_t i=0;i<ps.size();++i){
    Point32f p = ps[i], n = nn[i];
    plot->circle(p.x,p.y,0.5);
    plot->line(p.x,p.y,n.x, n.y);
  }
}


int main(int n, char **ppc){
  return ICLApp(n,ppc,"",init).exec();
}
