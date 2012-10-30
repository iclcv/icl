/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2012 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : include/ICLMath/Octree.h                               **
** Module : ICLMath                                                **
** Authors: Christof Elbrechter                                    **
**                                                                 **
**                                                                 **
** Commercial License                                              **
** ICL can be used commercially, please refer to our website       **
** www.iclcv.org for more details.                                 **
**                                                                 **
** GNU General Public License Usage                                **
** Alternatively, this file may be used under the terms of the     **
** GNU General Public License version 3.0 as published by the      **
** Free Software Foundation and appearing in the file LICENSE.GPL  **
** included in the packaging of this file.  Please review the      **
** following information to ensure the GNU General Public License  **
** version 3.0 requirements will be met:                           **
** http://www.gnu.org/copyleft/gpl.html.                           **
**                                                                 **
** The development of this software was supported by the           **
** Excellence Cluster EXC 277 Cognitive Interaction Technology.    **
** The Excellence Cluster EXC 277 is a grant of the Deutsche       **
** Forschungsgemeinschaft (DFG) in the context of the German       **
** Excellence Initiative.                                          **
**                                                                 **
*********************************************************************/

#pragma once


#include <algorithm>
#include <set>

#include <ICLMath/FixedVector.h>
#include <ICLUtils/VisualizationDescription.h>
#include <ICLUtils/Rect32f.h>
#include <ICLUtils/Range.h>
#include <ICLUtils/StringUtils.h>

namespace icl{

  namespace math{
    

    /// Generic Octree Implementation
    /** The Octree implementation is a simple 3D-generalization of the 
        QuadTree class template.
        
        \section BENCH Benchmarks
        
        Even though, we do not have any reliable results, it might be
        possible, that the octree is much faster then the pcl-octree!
        
    **/
    template<class Scalar, int CAPACITY=16, int SF=1, class Pt=FixedColVector<Scalar,4>, int ALLOC_CHUNK_SIZE=1024>
    class Octree{
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
          return (    p[0] >= center[0] - halfSize[0]
                   && p[1] >= center[1] - halfSize[1]
                   && p[2] >= center[2] - halfSize[2]
                   && p[0] <= center[0] + halfSize[0]
                   && p[1] <= center[1] + halfSize[1]
                   && p[1] <= center[2] + halfSize[2]);
        }
        
        /// returns whether the AABB intersects with another AABB
        bool intersects(const AABB &o) const{
          return  (fabs(center[0] - o.center[0]) < (halfSize[0] + o.halfSize[0])
                   && fabs(center[1] - o.center[1]) < (halfSize[1] + o.halfSize[1])
                   && fabs(center[2] - o.center[2]) < (halfSize[2] + o.halfSize[2]));
                   
        }
      };
      
      public:
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
              found.push_back(Pt((*p)[0]/SF,(*p)[1]/SF, (*p)[2]/SF ));
            }
          }
          if(!children) return;
          
          for(int i=0;i<8;++i){
            children[i].query(boundary,found);
          }
        }

        /// creates the children for this node
        /** children order is ul, ur, ll, lr. The children
            are created by the top-level QuadTree's allocator and passed
            to this function. 'split' initializes the four given children */
        void split(Node *children){
          const Pt half = boundary.halfSize/2;//*0.5;

          const Pt &c = boundary.center;
          this->children = children;
          this->children[0].init(this,AABB(Pt(c[0]-half[0],c[1]-half[1], c[2]-half[2]),half));
          this->children[1].init(this,AABB(Pt(c[0]+half[0],c[1]-half[1], c[2]-half[2]),half));
          this->children[2].init(this,AABB(Pt(c[0]-half[0],c[1]+half[1], c[2]-half[2]),half));
          this->children[3].init(this,AABB(Pt(c[0]+half[0],c[1]+half[1], c[2]-half[2]),half));

          this->children[4].init(this,AABB(Pt(c[0]-half[0],c[1]-half[1], c[2]+half[2]),half));
          this->children[5].init(this,AABB(Pt(c[0]+half[0],c[1]-half[1], c[2]+half[2]),half));
          this->children[6].init(this,AABB(Pt(c[0]-half[0],c[1]+half[1], c[2]+half[2]),half));
          this->children[7].init(this,AABB(Pt(c[0]+half[0],c[1]+half[1], c[2]+half[2]),half));

        }
        
#if 0
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
#endif

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
          allocated.push_back(new Node[ALLOC_CHUNK_SIZE*8]);
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
          return allocated.back()+8*curr++;
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
                pts.push_back(Pt((*p)[0]/SF,(*p)[1]/SF,(*p)[2]/SF),1);
              }
            }
          }
          const Node *ns = allocated.back();
          for(int i=0;i<curr*4;++i){
            for(const Pt* p = ns[i].points; p != ns[i].next;++p){
              pts.push_back(Pt((*p)[0]/SF,(*p)[1]/SF,(*p)[2]/SF,1));
            }
          }
          return pts;
        }
      };

      protected:

      /// root node pointer
      Node *root;
      
      /// memory allocator for all children except for the root node
      Allocator alloc;

      /// internal counter for the number of contained points
      int num;

      public:
      /// creates a QuadTree for the given 2D rectangle
      Octree(const Scalar &minX, const Scalar &minY, const Scalar &minZ,
               const Scalar &width, const Scalar &height, const Scalar &depth):num(0){
        this->root = new Node(AABB(Pt(SF*minX+SF*width/2, SF*minY+SF*height/2, SF*minZ+SF*depth/2),
                                   Pt(SF*width/2,SF*height/2, SF*depth/2)));
      }

      /// creates a QuadTree for the given 2D rectangle
      Octree(const Scalar &min, const Scalar &len):num(0){
        this->root = new Node(AABB(Pt(SF*min+SF*len/2, SF*min+SF*len/2, SF*min+SF*len/2),
                                   Pt(SF*len/2,SF*len/2, SF*len/2)));
      }

      /// destructor
      /** Deletes the root node only, all other nodes are deleted by the allocator */
      ~Octree(){
        // the root node is not part of the allocator
        delete root;
      }
  
      protected:
      
      /// internal utility method that is used to find an approximated nearest neighbour
      const Pt &nn_approx_internal(const Pt &p, double &currMinDist, const Pt *&currNN) const throw (utils::ICLException){
        // 1st find cell, that continas p
        const Node *n = root;
        while(n->children){
          n = (n->children 
               + (p[0] > n->boundary.center[0]) 
               + 2 * (p[1] > n->boundary.center[1]) 
               + 4 * (p[2] > n->boundary.center[2]));
        }
        
        // this cell could be empty, in this case, the parent must contain good points
        if(n->next == n->points){
          n = n->parent;
          if(!n) throw utils::ICLException("no nn found for given point " + utils::str(p));
        }
        
        double sqrMinDist = utils::sqr(currMinDist);

        for(const Pt *x=n->points; x < n->next; ++x){
          Scalar dSqr = ( utils::sqr(x->operator[](0)-p[0]) 
                          + utils::sqr(x->operator[](1)-p[1])
                          + utils::sqr(x->operator[](2)-p[2]) );
          if(dSqr < sqrMinDist){
            sqrMinDist = dSqr;
            currNN = x; 
          }
        }
        currMinDist = sqrt(sqrMinDist);

        if(!currNN){
          throw utils::ICLException("no nn found for given point " + utils::str(p.transp()));
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
      Pt nn_approx(const Pt &p) const throw (utils::ICLException){
        double currMinDist = sqrt(utils::Range<Scalar>::limits().maxVal-1);
        const Pt *currNN  = 0;
        nn_approx_internal(Pt(SF*p[0],SF*p[1],SF*p[2]),currMinDist,currNN);
        return Pt((*currNN)[0]/SF,(*currNN)[1]/SF,(*currNN)[2]/SF,1) ;
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
      Pt nn(const Pt &pIn) const throw (utils::ICLException){
        const Pt p(SF*pIn[0],SF*pIn[1],SF*pIn[2]);
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
            const AABB b(p, Pt(currMinDist,currMinDist,currMinDist));
            for(int i=0;i<8;++i){
              if(b.intersects(n->children[i].boundary)) stack.push_back(n->children+i);
            }
          }
          Scalar sqrMinDist = utils::sqr(currMinDist);

          for(const Pt *x=n->points; x < n->next; ++x){
            Scalar dSqr = (utils::sqr(x->operator[](0)-p[0]) + 
                           utils::sqr(x->operator[](1)-p[1]) +
                           utils::sqr(x->operator[](2)-p[2]));
            if(dSqr < sqrMinDist){
              sqrMinDist = dSqr;
              currNN = x; 
            }
          }
          currMinDist = sqrt(sqrMinDist);
        }
        return Pt((*currNN)[0]/SF, (*currNN)[1]/SF, (*currNN)[2]/SF, 1 );
      }

      /// inserts a node into the QuadTree
      /** This method is also implemented in an iterative fashion for 
          performance issues. 'insert' automatically uses the internal allocator
          if new nodes are needed. */ 
      template<class OtherVectorType>
      void insert(const OtherVectorType &pIn){
        ++num;
        const Pt p(SF*pIn[0],SF*pIn[1],SF*pIn[2]);
        Node *n = root;
        while(true){
          if(n->next != n->points+CAPACITY){
            *n->next++ = p;
            return;
          }
          if(!n->children) n->split(alloc.next());
          n = (n->children 
               + (p[0] > n->boundary.center[0]) 
               + 2 * (p[1] > n->boundary.center[1])
               + 4 * (p[2] > n->boundary.center[2]));
        }
      }
      
      /// returns all contained points within the given rectangle
      std::vector<Pt> query(const Scalar &minX, const Scalar &minY, const Scalar &minZ, 
                            const Scalar &width, const Scalar &height, const Scalar &depth) const{
        AABB range(Pt(SF*minX+SF*width/2, SF*minY+SF*height/2, SF*minZ+SF*depth/2),
                   Pt(SF*width/2,SF*height/2, SF*depth/2));
        std::vector<Pt> found;
        root->query(range,found);
        return found;
      }

      /// returns all contained points
      std::vector<Pt> queryAll() const {
        return alloc.all();
      }

      /*
      /// returns a visualization description for QuadTree structure (not for the contained points)
      utils::VisualizationDescription vis() const{
        utils::VisualizationDescription d;
        d.color(0,100,255,255);
        d.fill(0,0,0,0);
        root->vis(d);
        return d;
      }
      */
      
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

      /*
          /// prints the quad-tree structure hierachically (for debug purpose)
          void printStructure(){
          std::cout << "QuadTree{" << std::endl;
          root->printStructure(1);
          std::cout << "}" << std::endl;
          }
      */
    };

  } // namespace math
} // namespace icl

