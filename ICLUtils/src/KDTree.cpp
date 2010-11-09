/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2010 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLUtils/src/KDTree.cpp                                **
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
#include <ICLUtils/KDTree.h>

namespace icl{

KDTree::KDTree(std::vector<DynMatrix<icl64f> > &list){
	if(list.size()>0){
		buildTree(list, 0, &root);
	}
}

KDTree::KDTree(std::vector<DynMatrix<icl64f> *> &list){
	if(list.size()>0){
		buildTree(list, 0, &root);
	}
}

KDTree::~KDTree(){}

struct sort_by_axis{
	unsigned int axis;
	sort_by_axis(unsigned int axis):axis(axis){}
	bool operator()(const DynMatrix<icl64f> &a, const DynMatrix<icl64f> &b) const{
		return a[axis] < b[axis];
	}
	bool operator()(const DynMatrix<icl64f> *a, const DynMatrix<icl64f> *b) const{
		return (*a)[axis] < (*b)[axis];
	}
};

void KDTree::buildTree(std::vector<DynMatrix<icl64f> > &list, unsigned int depth, Node *node){
	if(list.size() == 1){
		node->point = &(list.at(0));
		return;
	}
	unsigned int k = (list.at(0)).rows();
	unsigned int axis = depth % k;
	std::sort(list.begin(),list.end(),sort_by_axis(axis));
	//sortList(list, axis);

	unsigned int median = list.size()/2;

	node->left = new Node();
	node->median = double((list.at(median)).at(0,axis));
	std::vector<DynMatrix<icl64f> > sublist;
	for(unsigned int i=0;i<median;++i){
		sublist.push_back(list.at(i));
	}
	buildTree(sublist,depth+1,node->left);

	node->right = new Node();
	sublist.clear();
	for(unsigned int i=median;i<list.size();++i){
		sublist.push_back(list.at(i));
	}
	buildTree(sublist,depth+1,node->right);
}

void KDTree::buildTree(std::vector<DynMatrix<icl64f> *> &list,unsigned int depth, Node *node){
	if(list.size() <= 1){
		node->point = list.at(0);
		return;
	}
	unsigned int k = list.at(0)->rows();
	unsigned int axis = depth % k;
	std::sort(list.begin(),list.end(),sort_by_axis(axis));
	//sortList(list, axis);

	unsigned int median = list.size()/2;

	node->left = new Node();
	node->median = double((list.at(median))->at(0,axis));
	std::vector<DynMatrix<icl64f>* > sublist;
	for(unsigned int i=0;i<median;++i){
		sublist.push_back(list.at(i));
	}
	buildTree(sublist,depth+1,node->left);

	node->right = new Node();
	sublist.clear();
	for(unsigned int i=median;i<list.size();++i){
		sublist.push_back(list.at(i));
	}
	buildTree(sublist,depth+1,node->right);
}

void KDTree::sortList(std::vector<DynMatrix<icl64f> > &list, unsigned int dim){
	DynMatrix<icl64f> temp;
	unsigned int minIndex = 0;
	for(unsigned int i=0;i<list.size()-1;++i){
		minIndex = i;
		for(unsigned int j=(i+1);j<list.size();++j){
			if((list.at(minIndex)).at(0,dim) < (list.at(j)).at(0,dim)){
				minIndex = j;
			}
		}
		if(minIndex > i){
			temp = list.at(i);
			list.at(i) = list.at(minIndex);
			list.at(minIndex) = temp;
		}
	}
}

void KDTree::sortList(std::vector<DynMatrix<icl64f>* > &list, unsigned int dim){
	DynMatrix<icl64f> *temp=0;
	unsigned int minIndex = 0;
	for(unsigned int i=0;i<list.size()-1;++i){
		minIndex = i;
		for(unsigned int j=(i+1);j<list.size();++j){
			if(list.at(minIndex)->at(0,dim) < list.at(j)->at(0,dim)){
				minIndex = j;
			}
		}
		if(minIndex > i){
			temp = list.at(i);
			list.at(i) = list.at(minIndex);
			list.at(minIndex) = temp;
		}
	}
}

void KDTree::releaseTree(){
	if(root.right)
		delete root.right;
	if(root.left)
		delete root.left;
}

DynMatrix<icl64f>* KDTree::nearestNeighbour(const DynMatrix<icl64f> &point){
	unsigned int k = point.rows();
	unsigned int depth = 0;
	unsigned int axis = depth % k;
	Node *n = &root;
	while(1){
		if(point[axis]<n->median){
			if(n->left)
				n = n->left;
			else
				return n->point;
		} else {
			if(n->right)
				n = n->right;
			else
				return n->point;
		}
		depth += 1;
		axis = depth % k;
	}
}

DynMatrix<icl64f>* KDTree::nearestNeighbour(const DynMatrix<icl64f> *point){
	unsigned int k = point->rows();
	unsigned int depth = 0;
	unsigned int axis = depth % k;
	Node *n = &root;
	while(1){
		if(((*point)[axis]) < (n->median)){
			if(n->left != 0){
				n = n->left;
			}else{
				return n->point;
			}
		} else {
			if(n->right != 0){
				n = n->right;
			}else{
				return n->point;
			}
		}
		depth += 1;
		axis = depth % k;
	}
}

void KDTree::print(){
	print(&root);
}

void KDTree::print(Node *n){
	if(n->left){
		print(n->left);
	}
	if(n->right){
		print(n->right);
	}
	if(n->point){
		std::cout << (*(n->point))[0] << "  " << (*(n->point))[1] << "  " << (*(n->point))[2]  << std::endl;
		//SHOW(*(n->point));
	}
}
}

