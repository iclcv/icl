// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christian Groszewski, Christof Elbrechter

#pragma once

#include <ICLUtils/CompatMacros.h>
#include <ICLMath/DynMatrix.h>
#include <ICLUtils/Macros.h>
#include <ICLUtils/BasicTypes.h>
#include <ICLUtils/Uncopyable.h>
#include <vector>

namespace icl::math {
    /// Simple KD-Tree implementation
    /** This class implements a simple kd-tree. You can create an object of this class
        with or without given point data. After creating a kd-tree without data you can use
        the buildTree method to insert data points or to rebuild the kd-tree with new
        data points.
        <b>Note:</b> If point data was passed to the KD-Tree constructor, it only references
        by the KD-Tree instance. Therefore, that tree instance will only stay valid as long as
        the referenced data does
    */
    class ICLMath_API KDTree {
      public:
      KDTree(const KDTree&) = delete;
      KDTree& operator=(const KDTree&) = delete;

      private:
      ///Keeps data of node
      struct Node{
        ///left node
        Node *left;
        ///right node
        Node *right;
        ///point in leafnode, else null
        DynMatrix<icl64f> *point;
        ///median of dimension
        double median;

        ///Constructor
        inline Node():left(0),right(0),point(0),median(0.0){}

        ///Destructor
        inline ~Node(){
          if(right != 0){
            delete right;
            right = 0;
          }
          if(left != 0){
            delete left;
            left = 0;
          }

        }
      };

      ///the root node of the tree
      Node root;

      ///internal print call
      void print(Node *node);
      ///internal call to fill the KDTree with data
      void buildTree(std::vector<math::DynMatrix<icl64f> > &list,unsigned int depth, Node *node);
      ///internal call to fill the KDTree with data
      void buildTree(std::vector<math::DynMatrix<icl64f> *> &list,unsigned int depth, Node *node);
      ///internal call to sort list by dimension of the vector (unused, instead std::sort)
      void sortList(std::vector<math::DynMatrix<icl64f> > &list, unsigned int dim);
      ///internal call to sort list by dimension of the vector (unused, instead std::sort)
      void sortList(std::vector<math::DynMatrix<icl64f>* > &list, unsigned int dim);
      ///internal call to release data from KDTree
      void releaseTree();

      public :
      ///Constructor
      /** Creates a new KDTree object, with data from list.
       * @param list list of points for  the kd-tree
       */
      KDTree(std::vector<math::DynMatrix<icl64f> > &list);

      ///Constructor
      /** Creates a new KDTree object, with data from list.
       * @param list list of points for  the kd-tree
       */
      KDTree(std::vector<math::DynMatrix<icl64f>* > &list);

      ///Constructor
      /** Creates a new KDTree object. Default constructor
       */
      KDTree(){}

      ///Destructor
      ~KDTree();

      ///builds a kd-tree
      /** Fills empty kd-tree or the current one with new data.
       * @param list list of points for the kd-tree
       */
      inline void buildTree(std::vector<math::DynMatrix<icl64f> *> &list){
        releaseTree();
        buildTree(list,0,&root);
      }

      ///builds a kd-tree
      /** Fills empty KDTree object or the current one with new data.
       * @param list list of points for the kd-tree
       */
      inline void buildTree(std::vector<math::DynMatrix<icl64f> > &list){
        releaseTree();
        buildTree(list,0,&root);
      }

      ///Prints the tree on standard output.
      void print();

      ///Returns pointer to nearest neighbour to passed point.
      /** @param point the point to search nearest neighbor for
       *  @return the pointer to nearest neighbour */
      math::DynMatrix<icl64f>* nearestNeighbour(const math::DynMatrix<icl64f> &point);

      ///Returns pointer to nearest neighbour to passed point.
      /** @param point the point to search nearest neighbor for
       *  @return the pointer to nearest neighbour */
      math::DynMatrix<icl64f>* nearestNeighbour(const math::DynMatrix<icl64f> *point);

    };
  } // namespace utils