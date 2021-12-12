/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLMath/src/ICLMath/QuadTree.h                         **
** Module : ICLMath                                                **
** Authors: Christof Elbrechter                                    **
**                                                                 **
**                                                                 **
** GNU LESSER GENERAL PUBLIC LICENSE                               **
** This file may be used under the terms of the GNU Lesser General **
** Public License version 3.0 as published by the                  **
**                                                                 **
** Free Software Foundation and appearing in the file LICENSE.LGPL **
** included in the packaging of this file.  Please review the      **
** following information to ensure the license requirements will   **
** be met: http://www.gnu.org/licenses/lgpl-3.0.txt                **
**                                                                 **
** The development of this software was supported by the           **
** Excellence Cluster EXC 277 Cognitive Interaction Technology.    **
** The Excellence Cluster EXC 277 is a grant of the Deutsche       **
** Forschungsgemeinschaft (DFG) in the context of the German       **
** Excellence Initiative.                                          **
**                                                                 **
********************************************************************/

#pragma once


#include <ICLUtils/CompatMacros.h>
#include <ICLMath/FixedVector.h>
#include <ICLUtils/VisualizationDescription.h>
#include <ICLUtils/Rect32f.h>
#include <ICLUtils/Range.h>
#include <ICLUtils/StringUtils.h>

#include <algorithm>
#include <set>

namespace icl{

  namespace math{


    /// Generic QuadTree Implementation
    /** The implementation follows the pseudo code from Wikipedia. However, we added
        some performance related implementation enhancements.

        \section TMP Template Parameters

        * <b>Scalar</b> the internal data type, which must be either float or double
        * <b>CAPACITY</b> the node capacity. This defines the number of Points,
          that are stored in each node. Due to an internal optimization, best performace
          in terms insertion and nearest-neighbor search is given for CAPACITIES in the
          range of 32 to 128 entries.
        * <b>SF</b> the internal data scale factor. This can be used to reach a higher
          exact integer resolution when splitting regions. The scale factor should usually
          be a power of two. In our benchmarks, it turned out, that using a non-1 scalefactor
          does not affect the speed of the quadtree's methods. We recommend to use
          a scalefactor of 32, which ensures at least 5 quad-tree levels can be split
          perfectly, while still allowing a data range of [-67M,67M] (2^(31-5)).
        * <b>ALLOC_CHUNK_SIZE</b> This parameters does actually not alter the performance
          very much. It defines the size of memory blocks allocated by the internal block
          allocator

        \section PERF Performance

        As far as we can say, the implementation is quite fast. For now, we will only
        provide a few examples; a full evaluation will be given soon.

        System: 2.4GHz Core2Duo, Ubuntu 12.04 32Bit, 4 GB Ram, Optimized build
                (-O4 -march=native)

        Experiment base line:

        QuadTree<float,32,1,1024> with VGA bounds, containing 100K points uniformly distributed.
        for integers an upscaling factor of 32 is used:
        QuadTree<int,32,32,1024> with VGA bounds, containing 100K points uniformly distributed.
        Using no scale factor (SF = 1) does not lead to faster processing!

        Tasks are:

        * insertion
        * nearest neighbor search of 1000 random points
        * approximate nearest neighbor search of 1000 random points
        * query a huge region: Rect(100,100,500,350), containing 57% of all points

        \subsection E1 Experiment 1 (Base line results)

        (numbers in round braces refer to the integer quadtree performance)

        In particular the nearest neighbour search, that is dominated by
        comparing nodes and distances runs about 3 time faster for integers.

        * insertion: 5.8ms (5.4ms)
        * nn-search: 3.6ms (1.2ms)
        * approx. nn: 0.19ms  (0.15ms)
        * query: 1.7ms (1.7ms)

        \subsection E2 Experiment 2 (Using smaller Nodes of CAPACITY 4)

        Smaller nodes implicate more structure and a deeper node hierarchy. This makes
        all parts significantly slower. In particular the nn-search is affecter. Here,
        the list of points in each node can be compared without using the square-root function,
        which is very time consuming.

        The nearest neighbor search performance for integer processing is still about two times faster

        * insertion: 9.6ms (9.6ms)
        * nn-search: 1.7ms (0.9ms)
        * approx. nn: 0.16ms (0.12ms)
        * query: 2.3ms (2.3ms)

        \subsection E2b Experiment 2b (Using larger Nodes of CAPACITY 128)

        Here, again, we can see that larger nodes speed up the insertion part, while
        the nn-search optimization is already saturated here.

        * insertion: 4.5ms (4.6ms)
        * nn-search: 6.5ms (2.5ms)
        * approx nn: 0.31ms (0.31ms)
        * query: 1.73ms (1.72ms)


        \subsection E3 Experiment 3 (Using 10K Points only)

        With less points, the whole system gets significantly faster. In particular insertion
        and query is more then 10-times as fast which can be explained by better caching properties.
        The nearest neighbour search has a logarithmic complexity and is sped up least.

        * insertion: 0.4ms (0.34ms)
        * nn-search: 2.4ms (1.07ms)
        * approx. nn: 0.16ms (0.287ms)
        * query: 0.16ms (0.17ms)

        \subsection E4 Experiment 4 (Using 1000K Points)

        Obviously, we face caching issues here: While 10K and even 100K points could easily be cached,
        the whole structure cannot be cached with 1000K points. Therefore, insertion gets
        significantly slower. The logarithmic complexity of the nn-search stays valid and make
        this part not become that much slower. The approximate nn-search is not affected so strongly
        because it majorly depends on the node capacity.

        * insertion: 130ms (123ms)
        * nn-search: 8ms (1.5ms)
        * approx. nn: 0.33ms (0.18ms)
        * query: 26ms (27ms)

        \subsection E5 Experiment 5 (Using 1000K Points, but with Node CAPACITY 128)

        Here, the insertion time gets smaller, because less nodes have to be created.
        On the other hand, the nn-search takes slightly longer

        * insertion: 87ms (85ms)
        * nn-search: 9.8ms (3ms)
        * approx. nn: 0.3ms (0.23ms)
        * query: 23ms (24ms)

        \subsection E6 Experiment 6 (Using 1000K Points, but with Node CAPACITY 1024)

        Same effect as before, but much stronger. The approximate nn-search becomes
        alot slower, because all CAPACITY points in the best matching cell must be checked.
        However, the approximate results are usually more accurate here

        * insertion: 55ms (54ms)
        * nn-search: 41ms (17ms)
        * approx. nn: 2.7ms (2.7ms)
        * query: 22.8ms (22.8ms)
    */
    template<class Scalar, int CAPACITY=4, int SF=1, int ALLOC_CHUNK_SIZE=1024,
             class VECTOR_TYPE=FixedColVector<Scalar,2> >
    class QuadTree{
      public:

      // 2D-point type, internally used
      typedef VECTOR_TYPE Pt;

      protected:

      /// internally used axis-aligned bounding box
      /** An AABB is defined by its center point and it half bounds
          into x- and y-direction */
      struct AABB{
        Pt center;   //!< center point
        Pt halfSize; //!< half dimension

        /// default constructor (does nothing)
        AABB(){}

        /// constructor from given center and half size
        AABB(const Pt &center, const Pt &halfSize):
          center(center),halfSize(halfSize){}

        /// returns whether a given 2D point is contained
        bool contains(const Pt &p) const{
          return ( p[0] >= center[0] - halfSize[0]
                   && p[1] >= center[1] - halfSize[1]
                   && p[0] <= center[0] + halfSize[0]

                   && p[1] <= center[1] + halfSize[1]);
        }

        /// returns whether the AABB intersects with another AABB
        bool intersects(const AABB &o) const{
#ifdef WIN32
          return  (fabs((float)center[0] - o.center[0]) < (halfSize[0] + o.halfSize[0])
                   && fabs((float)center[1] - o.center[1]) < (halfSize[1] + o.halfSize[1]));
#else
          return  (fabs(center[0] - o.center[0]) < (halfSize[0] + o.halfSize[0])
            && fabs(center[1] - o.center[1]) < (halfSize[1] + o.halfSize[1]));
#endif
        }


        utils::Rect32f rect() const {
          return utils::Rect32f(double(center[0])/SF - double(halfSize[0])/SF, double(center[1])/SF - double(halfSize[1])/SF,
                                double(halfSize[0])/SF*2, double(halfSize[1])/SF*2);
        }
      };

      /// Internally used node structure
      /** Each node can contain up to CAPACITY elements. Further nodes
          are distributed to one of the four children */
      struct Node{
        AABB boundary;         //!< node boundary
        Pt points[CAPACITY];   //!< contained nodes
        Pt *next;              //!< next node to fill
        Node *children;        //!< pointer to four child-nodes
        Node *parent;

        /// empty default constructor (does nothing)
        Node(){}

        /// constructor from given AABB-boundary
        Node(const AABB &boundary){
          init(0,boundary);
        }
        /// initialization methods (with given boundary)
        /** sets next to points-begin and children to NULL */
        void init(Node *parent, const AABB &boundary){
          this->boundary = boundary;
          this->next = this->points;
          this->children = 0;
          this->parent = parent;
        }

        /// recursive getter function that queries all nodes within a given bounding box
        /** breaks the recursion if no children are present or if the nodes
            boundary does not intersect with the given boundary. Recursively fills
            the given 'found'-vector*/
        void query(const AABB &boundary, std::vector<Pt> &found) const{
          if (!this->boundary.intersects(boundary)){
            return;
          }
          for(const Pt *p=points; p<next ; ++p){
            if(boundary.contains(*p)){
              found.push_back((*p)/SF);//Pt((*p)[0]/SF,(*p)[1]/SF));
            }
          }
          if(!children) return;

          children[0].query(boundary,found);
          children[1].query(boundary,found);
          children[2].query(boundary,found);
          children[3].query(boundary,found);
        }

        /// creates the children for this node
        /** children order is ul, ur, ll, lr. The children
            are created by the top-level QuadTree's allocator and passed
            to this function. 'split' initializes the four given children */
        void split(Node *children){
          const Pt half = boundary.halfSize/2;//*0.5;

          const Pt &c = boundary.center;
          this->children = children;
          this->children[0].init(this,AABB(Pt(c[0]-half[0],c[1]-half[1]),half));
          this->children[1].init(this,AABB(Pt(c[0]+half[0],c[1]-half[1]),half));
          this->children[2].init(this,AABB(Pt(c[0]-half[0],c[1]+half[1]),half));
          this->children[3].init(this,AABB(Pt(c[0]+half[0],c[1]+half[1]),half));
        }

        /// recursively grabs visualizations commands
        void vis(utils::VisualizationDescription &d) const{
          d.rect(boundary.rect());
          if(children){
            children[0].vis(d);
            children[1].vis(d);
            children[2].vis(d);
            children[3].vis(d);
          }
        }

        void printStructure(int indent){
          for(int i=0;i<indent;++i) std::cout << "  ";
          if(children){
            std::cout << "Branch (";
          }else{
            std::cout << "Leaf (";
          }
          std::cout << "AABB=" << boundary.rect() << ", ";
          std::cout << "Content=" <<(int)(next-points) << "/" << CAPACITY;
          std::cout <<  ")";
          if(children){
            std::cout << " {" << std::endl;
            children[0].printStructure(indent+1);
            children[1].printStructure(indent+1);
            children[2].printStructure(indent+1);
            children[3].printStructure(indent+1);
            for(int i=0;i<indent;++i) std::cout << "  ";
            std::cout << "}" << std::endl;;
          }
          else std::cout << std::endl;

        }

      };

      /// Inernally used block allocator
      /** The allocator allocates ALLOC_CHUNK_SIZE*4 Node instances
          at once and automatically frees all data at destruction time */
      struct Allocator{

        /// allocated data
        std::vector<Node*> allocated;

        /// current data
        int curr;

        /// allocates the first data chunk
        Allocator(){ grow(); }

        /// allocates the next data chunk
        void grow(){
          allocated.push_back(new Node[ALLOC_CHUNK_SIZE*4]);
          curr = 0;
        }

        /// deletes all allocated data chunks (except for the first)
        void clear(){
          for(size_t i=1;i<allocated.size();++i){
            delete [] allocated[i];
          }
          allocated.resize(1);
          curr = 0;
        }

        /// returns the next four Node instances (allocates new data on demand)
        Node *next(){
          if(curr == ALLOC_CHUNK_SIZE) grow();
          return allocated.back()+4*curr++;
        }

        /// frees all allocated data
        ~Allocator(){
          for(size_t i=0;i<allocated.size();++i){
            delete [] allocated[i];
          }
        }

        /// returns all contained points
        std::vector<Pt> all() const{
          std::vector<Pt> pts;
          for(size_t i=0;i<allocated.size()-1;++i){
            const Node *ns = allocated[i];
            for(size_t j=0;j<allocated.size();++j){
              for(const Pt* p = ns[j].points; p != ns[j].next;++p){
                pts.push_back((*p)/SF);//Pt((*p)[0]/SF,(*p)[1]/SF));
              }
            }
          }
          const Node *ns = allocated.back();
          for(int i=0;i<curr*4;++i){
            for(const Pt* p = ns[i].points; p != ns[i].next;++p){
              pts.push_back((*p)/SF); //Pt((*p)[0]/SF,(*p)[1]/SF));
            }
          }
          return pts;
        }
      };

      /// root node pointer
      Node *root;

      /// memory allocator for all children except for the root node
      Allocator alloc;

      /// internal counter for the number of contained points
      int num;

      public:
      /// creates a QuadTree for the given 2D rectangle
      QuadTree(const Scalar &minX, const Scalar &minY, const Scalar &width, const Scalar &height):num(0){
        this->root = new Node(AABB(Pt(SF*minX+SF*width/2, SF*minY+SF*height/2),
                                   Pt(SF*width/2,SF*height/2)));
      }

      /// convenience constructor wrapper for given Rect32f bounds
      QuadTree(const utils::Rect32f &bounds):num(0){
        this->root = new Node(AABB(Pt(SF*bounds.x+SF*bounds.width/2, SF*bounds.y+SF*bounds.height/2),
                                   Pt(SF*bounds.width/2,SF*bounds.height/2)));
      }

      /// convenience constructor wrapper for given Rect32f bounds
      QuadTree(const utils::Rect &bounds):num(0){
        this->root = new Node(AABB(Pt(SF*bounds.x+SF*bounds.width/2, SF*bounds.y+SF*bounds.height/2),
                                   Pt(SF*bounds.width/2,SF*bounds.height/2)));
      }

      /// creates a QuadTree for the given 2D Size (minX and minY are set to 0 here)
      QuadTree(const Scalar &width, const Scalar &height):num(0){
        this->root = new Node(AABB(Pt(SF*width/2,SF*height/2),
                                   Pt(SF*width/2,SF*height/2)));
      }

      /// convenience wrapper for given Size32f bounds
      QuadTree(const utils::Size32f &size):num(0){
        this->root = new Node(AABB(Pt(SF*size.width/2,SF*size.height/2),
                                   Pt(SF*size.width/2,SF*size.height/2)));
      }

      /// convenience wrapper for given Size bounds
      QuadTree(const utils::Size &size):num(0){
        this->root = new Node(AABB(Pt(SF*size.width/2,SF*size.height/2),
                                   Pt(SF*size.width/2,SF*size.height/2)));
      }

      /// destructor
      /** Deletes the root node only, all other nodes are deleted by the allocator */
      ~QuadTree(){
        // the root node is not part of the allocator
        delete root;
      }

      protected:

      /// internal utility method that is used to find an approximated nearest neighbour
      const Pt &nn_approx_internal(const Pt &p, double &currMinDist, const Pt *&currNN) const{
        // 1st find cell, that continas p
        const Node *n = root;
        while(n->children){
          n = (n->children + (p[0] > n->boundary.center[0]) + 2 * (p[1] > n->boundary.center[1]));
        }

        // this cell could be empty, in this case, the parent must contain good points
        if(n->next == n->points){
          n = n->parent;
          if(!n) throw utils::ICLException("no nn found for given point " + utils::str(p));
        }

        double sqrMinDist = utils::sqr(currMinDist);

        for(const Pt *x=n->points; x < n->next; ++x){
          Scalar dSqr = utils::sqr(x->operator[](0)-p[0]) + utils::sqr(x->operator[](1)-p[1]);
          if(dSqr < sqrMinDist){
            sqrMinDist = dSqr;
            currNN = x;
          }
        }
        currMinDist = sqrt(sqrMinDist);

        if(!currNN){
          throw utils::ICLException("no nn found for given point " + utils::str(p));
        }
        return *currNN;
      }
      public:


      /// returns an approximated nearst neighbour
      /** While the real nearst neighbour must not neccessarily be
          in the cell that would theoretically contain p, The approximated
          one is always assumed to be in that bottom layer cell. If, by chance,
          the optimal leaf node does not contain any points (because it was just
          created as empty leaf), the leaf's parent node, which must actually contain
          CAPACITY points, is used instead. The approximate nearest neighbour search
          can easily be 5 times as fast as the real nearest neighbor search.
          The result quality depends on the number of contained points, and
          on the QuadTree's template parameters */
      Pt nn_approx(const Pt &p) const{
        double currMinDist = sqrt(utils::Range<Scalar>::limits().maxVal-1);
        const Pt *currNN  = 0;
        nn_approx_internal(p*SF /*Pt(SF*p[0],SF*p[1])*/,currMinDist,currNN);
        return (*currNN)/SF; //Pt((*currNN)[0]/SF,(*currNN)[1]/SF) ;
      }

      /// finds the nearest neighbor to the given node
      /** The implementation of this method explicitly avoids recursion by using
          a run-time stack. This leads to a 4x speed factor in comparison to
          the recursive implementaiton of this function.

          As an extra accelleration, the method initializes it's frist nearest
          neighbor guess using the nn_approx method, which gives an approximate
          speed up of factor two to four.

          As a 2nd accelleration heuristic, all CAPACITY nodes'
          distances are are first calculated and compared in a squared
          version, which can be computed without an expensive
          square-root operation. However, once the closest point
          within a single node is found, its real euclidian minimum
          distance is computed and stored for further bounding box checks.

          If no neighbour could be found, an exception is thown. This should
          actually only happen when nn is called on an empty QuadTree
      */
      Pt nn(const Pt &pIn) const{
        const Pt p = pIn*SF;//p(SF*pIn[0],SF*pIn[1]);
        std::vector<const Node*> stack;
        stack.reserve(128);
        stack.push_back(root);
        double currMinDist = sqrt(utils::Range<Scalar>::limits().maxVal-1);
        const Pt *currNN  = 0;

        nn_approx_internal(p,currMinDist,currNN);

        while(stack.size()){
          const Node *n = stack.back();
          stack.pop_back();
          if(n->children){
            const AABB b(p, Pt(currMinDist,currMinDist));
            if(b.intersects(n->children[0].boundary)) stack.push_back(n->children);
            if(b.intersects(n->children[1].boundary)) stack.push_back(n->children+1);
            if(b.intersects(n->children[2].boundary)) stack.push_back(n->children+2);
            if(b.intersects(n->children[3].boundary)) stack.push_back(n->children+3);
          }
          Scalar sqrMinDist = utils::sqr(currMinDist);

          for(const Pt *x=n->points; x < n->next; ++x){
            Scalar dSqr = utils::sqr(x->operator[](0)-p[0]) + utils::sqr(x->operator[](1)-p[1]);
            if(dSqr < sqrMinDist){
              sqrMinDist = dSqr;
              currNN = x;
            }
          }
          currMinDist = sqrt(sqrMinDist);
        }
        return (*currNN)/SF; //Pt((*currNN)[0]/SF, (*currNN)[1]/SF);
      }

      /// convenience wrapper for the Point32f type
      const utils::Point32f nn(const utils::Point32f &p) const{
        Pt n = nn(Pt(p.x,p.y));
        return utils::Point32f(n[0],n[1]);
      }

      /// convenience wrapper for the Point32f type
      const utils::Point nn(const utils::Point &p) const{
        Pt n = nn(Pt(p.x,p.y));
        return utils::Point(n[0],n[1]);
      }


      /// inserts a node into the QuadTree
      /** This method is also implemented in an iterative fashion for
          performance issues. 'insert' automatically uses the internal allocator
          if new nodes are needed. */
      void insert(const Pt &pIn){
        ++num;
        const Pt p = pIn*SF;//p(SF*pIn[0],SF*pIn[1]);
        Node *n = root;
        while(true){
          if(n->next != n->points+CAPACITY){
            *n->next++ = p;
            return;
          }
          if(!n->children) n->split(alloc.next());
          n = (n->children + (p[0] > n->boundary.center[0]) + 2 * (p[1] > n->boundary.center[1]));
        }
      }

      /// convenience wrapper for the Point32f instances
      void insert(const utils::Point32f &p){
        insert(Pt(p.x,p.y));
      }

      /// convenience wrapper for the Point instances
      void insert(const utils::Point &p){
        insert(Pt(p.x,p.y));
      }

      /// returns all contained points within the given rectangle
      std::vector<Pt> query(const Scalar &minX, const Scalar &minY,
                            const Scalar &width, const Scalar &height) const{
        AABB range(Pt(SF*minX+SF*width/2, SF*minY+SF*height/2),
                   Pt(SF*width/2,SF*height/2));
        std::vector<Pt> found;
        root->query(range,found);
        return found;
      }

      /// convenience wrapper for Rect class
      std::vector<Pt> query(const utils::Rect &r) const {  return query(r.x,r.y,r.width,r.height);  }

      /// convenience wrapper for Rect32f class
      std::vector<Pt> query(const utils::Rect32f &r) const {  return query(r.x,r.y,r.width,r.height);  }

      /// returns all contained points
      std::vector<Pt> queryAll() const {
        return alloc.all();
      }

      /// returns a visualization description for QuadTree structure (not for the contained points)
      utils::VisualizationDescription vis() const{
        utils::VisualizationDescription d;
        d.color(0,100,255,255);
        d.fill(0,0,0,0);
        root->vis(d);
        return d;
      }

      /// removes all contained points and nodes
      /** The allocator will free all memory except for the first CHUNK */
      void clear(){
        root->next = root->points;
        root->children = 0;
        alloc.clear();
        num = 0;
      }

      /// number of elments inserted
      int size() const {
        return num;
      }

      /// prints the quad-tree structure hierachically (for debug purpose)
      void printStructure(){
        std::cout << "QuadTree{" << std::endl;
        root->printStructure(1);
        std::cout << "}" << std::endl;
      }
    };

  } // namespace math
} // namespace icl
