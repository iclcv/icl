/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2012 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : include/ICLMath/QuadTree.h                             **
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

#include <ICLMath/FixedVector.h>

namespace icl{

  namespace math{
    
    template<class Scalar, int CAPACITY=4, int ALLOC_CHUNK_SIZE=1024>
    class QuadTree{
      public:
      
      // 2D-point type, internally used
      typedef FixedColVector<Scalar,2> Pt;
      
      
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
          return  (fabs(center[0] - o.center[0]) < (halfSize[0] + o.halfSize[0])
                   && fabs(center[1] - o.center[1]) < (halfSize[1] + o.halfSize[1]));
        }

        
        Rect32f rect() const {
          return Rect32f(center[0] - halfSize[0], center[1] - halfSize[1],
                         halfSize[0]*2, halfSize[1]*2);
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

        /// empty default constructor (does nothing)
        Node(){}
        
        /// constructor from given AABB-boundary
        Node(const AABB &boundary){
          init(boundary);
        }
        /// initialization methods (with given boundary)
        /** sets next to points-begin and children to NULL */
        void init(const AABB &boundary){
          this->boundary = boundary;
          this->next = this->points;
          this->children = 0;
        }
    
        /// recursive getter function that queries all nodes within a given bounding box
        /** breaks the recursion if no children are present or if the nodes
            boundary does not intersect with the given boundary. Recursively fills
            the given 'found'-vector*/
        void query(const AABB &boundary, std::vector<Pt> &found) const{
          if (!this->boundary.intersectsAABB(bounary)){
            return;
          }
          for(const Pt *p=points; p<next ; ++p){
            if(bounary.containsPoint(*p)){
              found.push_back(*p);
            }
          }
          if(!children) return;
          
          children[0].queryRange(boundary,found);
          children[1].queryRange(boundary,found);
          children[2].queryRange(boundary,found);
          children[3].queryRange(boundary,found);
        }

        /// creates the children for this node
        /** children order is ul, ur, ll, lr. The children
            are created by the top-level QuadTree's allocator and passed
            to this function. 'split' initializes the four given children */
        void split(Node *children){
          const Size32f half = boundary.halfSize*0.5;
          const Pt &c = boundary.center;
          this->children = children;
          this->children[0].init(AABB(Pt(c[0]-half[0],c[1]-half[1]),half));
          this->children[1].init(AABB(Pt(c[0]+half[0],c[1]-half[1]),half));
          this->children[2].init(AABB(Pt(c[0]-half[0],c[1]+half[1]),half));
          this->children[3].init(AABB(Pt(c[0]+half[0],c[1]+half[1]),half));
        }
        
        void vis(VisualizationDescription &d) const{
          d.rect(boundary.rect());
          if(children){
            children[0].vis(d);
            children[1].vis(d);
            children[2].vis(d);
            children[3].vis(d);
          }
        }
      };
      
      struct Allocator{
        std::vector<Node*> allocated;
        int curr;

        Allocator(){ grow(); }
        
        void grow(){
          allocated.push_back(new Node[ALLOC_CHUNK_SIZE*4]);
          curr = 0;
        }
      
        Node *next(){
          if(curr == ALLOC_CHUNK_SIZE) grow();
          return allocated.back()+4*curr++;
        }
        
        ~Allocator(){
          for(size_t i=0;i<allocated.size();++i){
            delete [] allocated[i];
          }
        }
      };

      /// root node pointer
      Node *root;
      
      /// memory allocator for all children except for the root node
      Allocator alloc;
  

      public:
      /// creates a QuadTree for the given 2D rectangle
      QuadTree(const Scalar &minX, const Scalar &minY, const Scalar &width, const Scalar &height){
        this->root = new Node(AABB(Pt(minX-width/2, minY-height/2),
                                   Pt(width/2,height/2)));
      }
      
      /// creates a QuadTree for the given 2D Size (minX and minY are set to 0 here)
      QuadTree(const Scalar &width, const Scalar &height){
        this->root = new Node(AABB(Pt(size[0]/2,size[1]/2), 
                                   Pt(size[0]/2,size[1]/2)));
      }

      /// destructor
      /** Deletes the root node only, all other nodes are deleted by the allocator */
      ~QuadTree(){
        // the root node is not part of the allocator
        delete root;
      }
  
      /// finds the nearest neighbor to the given node
      /** The implementation of this method explicitly avoids recursion by using
          a run-time stack. This leads to a 4x speed factor in comparison to
          the recursive implementaiton of this function.
          
          As a 2nd accelleration heuristic, all CAPACITY nodes'
          distances are are first calculated and compared in a squared
          version, which can be computed without an expensive
          square-root operation. However, once the closest point
          within a single node is found, its real euclidian minimum
          distance is computed and stored for further bounding box checks.
          
          If no neighbour could be found, an exception is thown. This should
          actually only happen when nn is called on an empty QuadTree
      */
      const Pt &nn(const Pt &p) const throw (ICLException){
        std::vector<const Node*> stack;
        stack.reserve(16);
        stack.push_back(root);
        float currMinDist = Range32f::limits().maxVal/2;
        const Pt *currNN  = 0;
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
          for(const Pt *x=n->points; x < n->next; ++x){
            float dSqr = sqr(x->x-p[0]) + sqr(x->y-p[1]);
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
      
      /// inserts a node into the QuadTree
      /** This method is also implemented in an iterative fashion for 
          performance issues. 'insert' automatically uses the internal allocator
          if new nodes are needed. */ 
      void insert(const Pt &p){
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
      
      std::vector<Pt> query(const AABB &range) const{
        std::vector<Pt> found;
        root->query(range,found);
        return found;
      }

      
      VisualizationDescription vis() const{
        VisualizationDescription d;
        root->vis(d);
        return d;
      }
    };

  } // namespace math
} // namespace icl

