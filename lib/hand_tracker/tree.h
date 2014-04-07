/*
 *  File:				tree.h
 *  Created by:			Steve Czarnuch
 *  Created:			January 2013
 *  Last Modified:		January 2013
 *  Last Modified by:	Steve Czarnuch
 */

#ifndef TREE_DEF_
#define TREE_DEF_

#include "defines.h"

enum node_type {
	UNKN,
	BRANCH,
	LEAF
};

struct split_cond{	// a splitting candidate and set of thresholds per feature
	cv::Point u;
	cv::Point v;
};


// The structure represents a node in a decision tree.
struct treeNode{

	treeNode* left;			// Pointer to the left child node.
	treeNode* right;		// Pointer to the right child node.
	treeNode* parent;		// Pointer to the parent node.

	cv::Point u;			// Offset u
	cv::Point v;			// Offset v
	int thresh;				// Feature threshold
	int depth;				// Depth of current node, where root = 0
	cv::Mat hist;			// Distribution of body part labels for leaf nodes
	node_type type;			// node or leaf
	int num;				// indexed number of the node;
	long int samples;		// number of samples assigned to this node

	// training data
	vector<imgSample> pixelsL, pixelsR;
	double gain;			// max gain

};

// structure used to read tree from file
struct treeNodeRead{
	int left;			// Pointer to the left child node.
	int right;		// Pointer to the right child node.
	int parent;		// Pointer to the parent node.

	cv::Point u;			// Offset u
	cv::Point v;			// Offset v
	int thresh;				// Feature threshold
	int depth;				// Depth of current node, where root = 0
	cv::Mat hist;			// Distribution of body part labels for leaf nodes
	node_type type;				// 1 = node or 2 = leaf
	int num;				// indexed number of the node;
	long int samples;		// number of samples assigned to this node
};

class DTree {
public:
	// constructor
	DTree();
	~DTree();

	treeNode* Query(cv::Point, cv::Mat rgb, cv::Mat depth);
	treeNode* insertIDDFS(split_cond cand, int thresh, cv::vector<imgSample> left, cv::vector<imgSample> right, node_type type, cv::Mat histogram, double gain, int maxDepth, long int cursamples);
	treeNode* searchIDDFS(int maxDepth);
	int saveIDDFS(double accum, int num_ims,  int num_nodes, vector<split_cond>& candidates, vector<int>& thresholds,float gain, int offset, int maxDepth, int splitThreshold, std::string projectPath, std::string dir);
	int loadTreeFromFile(std::string file);
	void destroy_tree();

	part parts;

private:

	treeNode* Query(cv::Point, treeNode* node, cv::Mat rgb, cv::Mat depth);
	treeNode* insertDLS(treeNode* node, int depth, split_cond cand, int thresh, vector<imgSample> left, vector<imgSample> right, node_type type, cv::Mat histogram, double gain, int maxDepth, long int cursamples);
	treeNode* searchDLS(treeNode* node, int depth);
	void saveDLS(treeNode* node, int depth);
	int loadTreeNode(treeNode* node, vector<treeNodeRead>& treeNodes);
	void destroy_tree(treeNode* leaf);

	treeNode* root;

};

#endif
