/********************************************************************
 **                Image Component Library (ICL)                    **
 **                                                                 **
 ** Copyright (C) 2006-2010 CITEC, University of Bielefeld          **
 **                         Neuroinformatics Group                  **
 ** Website: www.iclcv.org and                                      **
 **          http://opensource.cit-ec.de/projects/icl               **
 **                                                                 **
 ** File   : include/ICLUtils/KDTree.h                              **
 ** Module : ICLUtils                                               **
 ** Authors: Christian Groszewski                                   **
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
#ifndef ICL_KDTREE_H_
#define ICL_KDTREE_H_

#include <ICLUtils/DynMatrix.h>
#include <ICLUtils/Macros.h>
#include <ICLUtils/BasicTypes.h>
#include <vector>
namespace icl{

/**
 This class implements a simple kd-tree. You can create an object of this class
 with or without pointdata. After creating a kd-tree without data you can use
 the buildTree function to insert datapoints or to rebuild the kd-tree with new
 datapoints. Note: deleting the pointdata outside of this class means kd-tree is
 referencing deleted pointers, so the tree is invalid an should be deleted.
 */
class KDTree {
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
		Node():left(0),right(0),point(0),median(0.0){}

		///Destructor
		~Node(){
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
	void buildTree(std::vector<DynMatrix<icl64f> > &list,unsigned int depth, Node *node);
	///internal call to fill the KDTree with data
	void buildTree(std::vector<DynMatrix<icl64f> *> &list,unsigned int depth, Node *node);
	///internal call to sort list by dimension of the vector (unused, instead std::sort)
	void sortList(std::vector<DynMatrix<icl64f> > &list, unsigned int dim);
	///internal call to sort list by dimension of the vector (unused, instead std::sort)
	void sortList(std::vector<DynMatrix<icl64f>* > &list, unsigned int dim);
	///internal call to release data from KDTree
	void releaseTree();

public :
	///Constructor
	/** Creates a new KDTree object, with data from list.
	 * @param list list of points for  the kd-tree
	 */
	KDTree(std::vector<DynMatrix<icl64f> > &list);
	///Constructor
	///Constructor
	/** Creates a new KDTree object, with data from list.
	 * @param list list of points for  the kd-tree
	 */
	KDTree(std::vector<DynMatrix<icl64f>* > &list);
	///Constructor
	///Constructor
	/** Creates a new KDTree object, with data from list.
	 * @param list list of points for  the kd-tree
	 */
	KDTree(){}
	///Destructor
	~KDTree();

	///builds a kd-tree
	/** Fills empty kd-tree or the current one with new data.
	 * @param list list of points for the kd-tree
	 */
	void buildTree(std::vector<DynMatrix<icl64f> *> &list){
		releaseTree();
		buildTree(list,0,&root);
	}

	///builds a kd-tree
	/** Fills empty KDTree object or the current one with new data.
	 * @param list list of points for the kd-tree
	 */
	void buildTree(std::vector<DynMatrix<icl64f> > &list){
		releaseTree();
		buildTree(list,0,&root);
	}

	///Prints the tree on standard output.
	void print();

	///Returns pointer to nearest neighbour to passed point.
	/**@param point the point to search nearest neighbor for
	 * @return the pointer to nearest neighbour*/
	DynMatrix<icl64f>* nearestNeighbour(const DynMatrix<icl64f> &point);

	///Returns pointer to nearest neighbour to passed point.
	/**@param point the point to search nearest neighbor for
	 * @return the pointer to nearest neighbour*/
	DynMatrix<icl64f>* nearestNeighbour(const DynMatrix<icl64f> *point);

};
}
#endif /* ICL_KDTREE_H_ */

