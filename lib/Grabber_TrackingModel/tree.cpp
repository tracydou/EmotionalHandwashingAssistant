/*
 *  File:				tree.cpp
 *  Created by:			Steve Czarnuch
 *  Created:			January 2013
 *  Last Modified:		January 2013
 *  Last Modified by:	Steve Czarnuch
 */

#include <fstream>
#include "opencv2/opencv.hpp"

#include "tree.h"
#include "defines.h"

using namespace std;
using namespace cv;

int node_num = 0;
ofstream treeFile;
std::string get_date(void)
{
	time_t now;
	int maxDate = 20;
	char the_date[maxDate];

	the_date[0] = '\0';

	now = time(NULL);

	if (now != -1)
	{
		strftime(the_date, maxDate, "%y-%m-%d_%H-%M-%S", localtime(&now));
	}

	return std::string(the_date);
}
/******************************************************************************/

const string buildBuffer(treeNode* node){

	ostringstream buff;

	if(node->depth == 0){	// Construct header line for first node
		buff << "Node";
		buff << " " << "Type";
		buff << " " << "Depth";
		buff << " " << "Parent";
		buff << " " << "LChild";
		buff << " " << "RChild";
		buff << " " << "OffsetUx";
		buff << " " << "OffsetUy";
		buff << " " << "OffsetVx";
		buff << " " << "OffsetVy";
		buff << " " << "Threshold";
		buff << " " << "Gain";
		buff << " " << "Samples";
		buff << " " << "PDF(" << node->hist.total() << ")";
		buff << endl;
	}

	buff << node->num;
	buff << " " << node->type;
	buff << " " << node->depth;
	buff << " " << node->parent->num;
	if (node->left != NULL)
		buff << " " << node->left->num;
	else
		buff << " " << "-1";

	if (node->right != NULL)
		buff << " " << node->right->num;
	else
		buff << " " << "-1";

	buff << " " << node->u.x << " " << node->u.y;
	buff << " " << node->v.x << " " << node->v.y;
	buff << " " << node->thresh;
	buff << " " << node->gain;
	buff << " " << node->samples;

	for (unsigned int i=0;i<node->hist.total(); i++){
		buff << " " << node->hist.at<float>(0,i);
	}
	buff << endl;

	return buff.str();
}
/******************************************************************************/

int checkBG (Point pt, Mat fore){

	//int blue = fore.at<Vec3b>(pt.y,pt.x)[0];
	//int green = fore.at<Vec3b>(pt.y,pt.x)[1];
	//int red = fore.at<Vec3b>(pt.y,pt.x)[2];

	int dI = fore.at<uchar>(Point(pt.x,pt.y));

	//if (blue == 0 && green == 0 && red == 0)
	if(dI == 0)
		return(1);

	return(0);

}

/******************************************************************************/

// Calculate the depth feature for node splitting criteria according to
// "Real-Time human pose recognition in parts from single depth images" by Shotton et. all (2011)
int dFeature(Point pt, Point off_u, Point off_v, Mat fore, Mat depth){

	unsigned short dIu, dIv;
	Point u,v;

	float dI = depth.at<ushort>(Point(pt.x,pt.y))/1000.0;				// find depth of pixel (x,y) in mm and convert to meters

	// check if current pixel is invalid
	if (dI == 0)
		return INV_DEPTH;

	// calculate all offset points using the depth-normalized offsets of u,v
	u.x = int(pt.x + off_u.x/dI);
	u.y = int(pt.y + off_u.y/dI);
	v.x = int(pt.x + off_v.x/dI);
	v.y = int(pt.y + off_v.y/dI);

	// Check the foreground image to see if the point lies on the object of interest or if it's on the background.
	// if the offset point lies on the background or is outside the bounds of the image, the depth probe (dIu/dIv) is given a large positive value
	if( (u.x < 0) || (u.x > WIDTH-1) || (u.y < 0) || (u.y > HEIGHT - 1) || (checkBG(u, fore) == 1) )
		dIu = 20000;
	else
		dIu = depth.at<ushort>(Point(u.x,u.y));

	if( (v.x < 0) || (v.x > WIDTH-1) || (v.y < 0) || (v.y > HEIGHT - 1) || (checkBG(v, fore) == 1) )
		dIv = 20000;
	else
		dIv = depth.at<ushort>(Point(v.x,v.y));

	// if either of the offset pixels are invalid, return invalid, else return the feature
	if (dIu == 0 || dIv == 0)
		return INV_DEPTH;
	else
		return (dIu - dIv);
}
/******************************************************************************/

DTree::DTree()
{
	root = NULL;
	//pixels_count = 0;
}
/******************************************************************************/

// finds Probability Distribution Function in tree, or returns NULL if point is invalid
// given a point, binary foreimage and depth map
treeNode* DTree::Query(Point pix, Mat fore, Mat depth){

	if (this->root->left != NULL && this->root->right != NULL)
		return DTree::Query(pix, this->root, fore, depth);
	else
		return this->root;
}
/******************************************************************************/

treeNode* DTree::Query(Point pix, treeNode* node, Mat fore, Mat depth){

	int feature = dFeature( pix, node->u, node->v, fore, depth);

	if (feature != INV_DEPTH){
		if (feature < node->thresh){
			if(node->left->type == BRANCH)
				return DTree::Query(pix, node->left, fore, depth);
			else
				return node->left;
		}
		else{
			if(node->right->type == BRANCH)
				return DTree::Query(pix, node->right, fore, depth);
			else
				return node->right;
		}
	}

	return NULL;
}
/******************************************************************************/

// Iterative deepening depth-first search
// Searches for the first NULL node
//treeNode* DTree::insertIDDFS(split_cond cand, int thresh, cv::Mat left, cv::Mat right, node_type type, cv::Mat histogram, double gain)
treeNode* DTree::insertIDDFS(split_cond cand, int thresh, cv::vector<imgSample> left, cv::vector<imgSample> right, node_type type, cv::Mat histogram, double gain, int maxDepth, long int cursamples)
{
	int depth = 0;

	if(root==NULL)
	{
		root = new treeNode;
		root->left = NULL;
		root->right = NULL;
		root->parent = root;
		root->samples = cursamples;
		root->pixelsL.resize(left.size());
		std::copy(left.begin(),left.end(),root->pixelsL.begin());
		root->pixelsR.resize(right.size());
		std::copy(right.begin(),right.end(),root->pixelsR.begin());
		root->depth = 0;
		root->num = node_num;
		root->type = type;
		root->u.x = cand.u.x;
		root->u.y = cand.u.y;
		root->v.x = cand.v.x;
		root->v.y = cand.v.y;
		root->thresh = thresh;
		Scalar histTotal = sum(histogram);
		Mat test = Mat::ones(histogram.size(),CV_32FC1)*histTotal(0);
		Mat histNormal = histogram/test;		// Normalize the histogram
		root->hist = histNormal.clone();
		root->gain = gain;
		node_num++;
		return root;
	}

	while(depth < maxDepth - 1)
	{
		treeNode* result = insertDLS(root, depth, cand, thresh, left, right, type, histogram, gain, maxDepth, cursamples);
		if (result != NULL){ //result is a node)
			return result;
		}
		depth = depth + 1;
	}
	return NULL;
}
/******************************************************************************/

// Depth limited search and insert
treeNode* DTree::insertDLS(treeNode* node, int depth, split_cond cand, int thresh, vector<imgSample> left, vector<imgSample> right, node_type type, Mat histogram, double gain, int maxDepth, long int cursamples)
{
	//string nodetype;

	if (depth == 0 && node->left == NULL && node->type == BRANCH){			// Left child is the next NULL node of a split node
		node->left=new treeNode;
		node->left->left=NULL;    //Sets the left child of the child node to null
		node->left->right=NULL;   //Sets the right child of the child node to null
		node->left->parent=node;
		node->left->samples= cursamples;
		node->left->u.x = cand.u.x;
		node->left->u.y = cand.u.y;
		node->left->v.x = cand.v.x;
		node->left->v.y = cand.v.y;
		node->left->thresh = thresh;
		Scalar histTotal = sum(histogram);
		Mat test = Mat::ones(histogram.size(),CV_32FC1)*histTotal(0);
		Mat histNormal = histogram/test;		// Normalize the histogram
		node->left->hist = histNormal.clone();
		node->left->depth = node->depth+1;
		if (node->left->depth == maxDepth - 1)
			node->left->type=LEAF;
		else
			node->left->type=type;
		node->left->num = node_num;
		node->left->gain = gain;
		node_num ++;
		if (node->left->type == BRANCH){	// only store pixelsL and pixelsR if this node is a branch
			node->left->pixelsL.resize(left.size());
			std::copy(left.begin(),left.end(),node->left->pixelsL.begin());
			node->left->pixelsR.resize(right.size());
			std::copy(right.begin(),right.end(),node->left->pixelsR.begin());
		}
		return node;
	}
	else if (depth == 0 && node->right == NULL && node->type == BRANCH){			// Right child is the next NULL node of a split node
		node->right=new treeNode;
		node->right->left=NULL;    //Sets the left child of the child node to null
		node->right->right=NULL;   //Sets the right child of the child node to null
		node->right->parent=node;
		node->right->samples=cursamples;
		node->right->u.x = cand.u.x;
		node->right->u.y = cand.u.y;
		node->right->v.x = cand.v.x;
		node->right->v.y = cand.v.y;
		node->right->thresh = thresh;
		Scalar histTotal = sum(histogram);
		Mat test = Mat::ones(histogram.size(),CV_32FC1)*histTotal(0);
		Mat histNormal = histogram/test;		// Normalize the histogram
		node->right->hist = histNormal.clone();
		node->right->depth = node->depth+1;
		if (node->right->depth == maxDepth - 1)
			node->right->type=LEAF;
		else
			node->right->type=type;
		node->right->num = node_num;
		node->right->gain = gain;
		node_num++;
		if (node->right->type == BRANCH){ // only store pixelsL and pixelsR if this node is a branch
			node->right->pixelsL.resize(left.size());
			std::copy(left.begin(),left.end(),node->right->pixelsL.begin());
			node->right->pixelsR.resize(right.size());
			std::copy(right.begin(),right.end(),node->right->pixelsR.begin());
		}
		// Since the right node is always created after the left, if the right was created the parent no longer needs the left and right subsets.
		// So, we clear the memory of the left and right subsets
		node->pixelsL.clear();
		node->pixelsR.clear();

		return node;
	}
	else if (depth > 0){
		treeNode* result;

		// search through left node if it is a branch
		if (node->left->type == BRANCH){
			result = insertDLS(node->left, depth-1, cand, thresh, left, right, type, histogram, gain, maxDepth, cursamples);
			if(result !=NULL)
				return result;
		}
		// search through right node if it is a branch
		if (node->right->type == BRANCH){
			result = insertDLS(node->right, depth-1, cand, thresh, left, right, type, histogram, gain, maxDepth, cursamples);
			if(result != NULL)
				return result;
		}
	}

	return NULL;
}
/******************************************************************************/

// Iterative deepening depth-first search
// Searches for the first splitting node with at least one NULL child
treeNode* DTree::searchIDDFS(int maxDepth)
{
	int depth = 0;
	if(root==NULL)
	{
		cout << "Error! Root not initialized." << endl;
		return NULL;
	}

	while(depth < maxDepth - 1)
	{
		treeNode* result = searchDLS(root, depth);
		if (result != NULL){ //result is a solution)
			return result;
		}
		depth = depth + 1;
	}
	//cout << "Total nodes created: " << node_num << endl;
	return NULL;
}
/******************************************************************************/

// Depth limited search
treeNode* DTree::searchDLS(treeNode* node, int depth)
{
	if (depth == 0 && node->type==BRANCH && (node->left == NULL || node->right == NULL)){					// Child is the next NULL node of a split node
		return node;
	}
	else if (depth > 0 && node->type == BRANCH){
		//else if (depth > 0){
		treeNode* result;

		// search through left node if it isn't a leaf
		if (node->left->type == BRANCH){
			//cout << "Searching left node..." << endl;
			result = searchDLS(node->left, depth-1);
			if(result !=NULL){
				//cout << "Left node found, depth: " << node->left->depth << endl;
				return result;
			}
		}
		// search through right node if it isn't a leaf
		if (node->right->type == BRANCH){
			//cout << "Searching right node..." << endl;
			result = searchDLS(node->right, depth-1);
			if(result != NULL){
				//cout << "Right node found, depth: " << node->right->depth << endl;
				return result;
			}
		}
	}

	return NULL;
}
/******************************************************************************/

// TODO optimize this so it is more efficient like loadTree
// Iterative deepening depth-first search
// Searches through an existing tree and outputs tree to a file
int DTree::saveIDDFS( double accum, int num_ims, int num_nodes, vector<split_cond>& candidates, vector<int>& thresholds, float gain, int offset, int maxDepth, int splitThreshold, string projectPath, string dir)
{
	//ostringstream num_im, cands, threshs;
	ostringstream fileCreate;

	//num_im << num_ims;
	//cands << SPLIT_CANDIDATES;
	//threshs << NUM_THRESHOLDS;

	string trainDir = dir.substr(dir.find_last_of('/'), dir.length());

	//const string file_name = projectPath + "/trees/" + trainDir + "-" + get_date() + "_" + num_im.str() + "I_" + maxDepth + "D_" + gain + "G_" + cands.str() + "C_" + threshs.str() + "T.txt";
	fileCreate << projectPath << "/trees/" << trainDir
			<< "-" << get_date()
			<< "_" << num_ims << "I_"
			<< maxDepth << "D_"
			<< gain << "G_"
			<< candidates.size() << "C_"
			<< TRAIN_PIXELS << "S_"
			<< offset << "O_"
			<< thresholds.size() << "T.txt";

	string fileName = fileCreate.str();

	cout << "Tree: " << fileName << endl;

	treeFile.open(fileName.c_str());
	treeFile << "Training data:" << endl;
	treeFile << "SPLIT_CANDIDATES: " << candidates.size() << endl;
	treeFile << "THRESHOLDS: " << thresholds.size() << endl;
	treeFile << "NUM_PIXELS: " << TRAIN_PIXELS << endl;
	treeFile << "NUM_IMAGES: " << num_ims << endl;
	treeFile << "MAX_DEPTH: " << maxDepth << endl;
	treeFile << "MIN_GAIN: " << gain << endl;
	treeFile << "MAX_OFFSET: " << -offset << "," << offset << endl;
	treeFile << "MIN_THRESH, MAX_THRESH " << -splitThreshold << "," << splitThreshold << endl;
	treeFile << "BGCOLOUR " << this->parts.bgColour.x << " " << this->parts.bgColour.y << " " << this->parts.bgColour.z << endl;
	treeFile << "CLASSIFY_PARTS: " << parts.count << endl;
	for (int i=0; i < parts.count; i++){
		treeFile << this->parts.classify[i] << " ";
		treeFile << this->parts.colours[i].x << " " << this->parts.colours[i].y << " " << this->parts.colours[i].z;
		treeFile << this->parts.label[i] << endl;
	}
	treeFile << "Training time: " << accum << " seconds." << endl << endl;

	int depth = 0;
	if(root==NULL)
	{
		cout << "Error! Root not initialized. No tree to save." << endl;
		return 0;
	}

	treeFile << "NODES: " << num_nodes << endl;
	while(depth < maxDepth)
	{
		//treeNode* result = saveDLS(this->root, depth);
		saveDLS(this->root, depth);
		depth = depth + 1;
	}
	treeFile << endl;
	if (node_num != num_nodes)
		cout << "Total nodes in tree: " << node_num << ", reported by save: " << num_nodes << endl;

	treeFile << endl << "Split candidates (u.x u.y v.x v.y) " << candidates.size() << endl;
	for(unsigned int i=0; i < candidates.size(); i++){
		treeFile << candidates[i].u.x << " " << candidates[i].u.y << " " << candidates[i].v.x << " " << candidates[i].v.y;
		if (i != candidates.size()) treeFile << endl;
	}

	treeFile << endl << "Thresholds " << thresholds.size() << endl;
	for(unsigned int i=0; i < thresholds.size(); i++){
		treeFile << thresholds[i];
		if (i != thresholds.size()) treeFile << endl;
	}

	treeFile.close();
	return 1;
}
/******************************************************************************/

// Search through current depth and save node data to file
void DTree::saveDLS(treeNode* node, int depth)
{
	if (depth == 0){
		const string dataBuf = buildBuffer(node);
		treeFile << dataBuf;
	}
	else if (depth > 0){

		if (node->left != NULL)
			saveDLS(node->left, depth-1);

		if (node->right != NULL)
			saveDLS(node->right, depth-1);
	}
}
/******************************************************************************/

// Iterative deepening depth-first search
// Searches through an existing tree and outputs tree to a file
int DTree::loadTreeFromFile(string file)
{

	ifstream readFile;
	Point3_ <int> bgcolour = Point3_<int>(0,0,0);
	this->parts.bgColour = bgcolour; // initialize bg colour in case trees don't load

	readFile.open(file.data());

	int count=0;
	int numberNodes;										// number of nodes to read

	if (readFile.is_open())	{
		while ( readFile.good() )
		{
			string mainline;
			getline(readFile,mainline);				// read line from file
			istringstream nextline(mainline);		// string to input stream
			string head;						// header label of line
			nextline >> head;					// header label to string

			// put the data into the proper place in temp node
			if (head == "CLASSIFY_PARTS:"){
				nextline >> this->parts.count;
				bool c;
				Point3_<int> colour;
				string l;
				for (int i=0; i< this->parts.count; i++){
					string partline;
					getline(readFile,partline);
					istringstream vals(partline);

					if (!(vals >> c >> colour.x >> colour.y >> colour.z >> l))
					{
						std::cerr << "\tError in configuration for " << file << endl;
						return 0;
					}
					else{
						this->parts.classify.push_back(c);
						this->parts.colours.push_back(colour);
						this->parts.label.push_back(l);
					}
				}
			} // CLASSIFY PARTS
			else if (head == "BGCOLOUR"){
				if(!(nextline >> bgcolour.x >> bgcolour.y >> bgcolour.z))
				{
					cerr << "Error in " << file << " for background colour." << endl;
					return 0;
				}
				else{
					this->parts.bgColour = bgcolour;
				}
			}
			else if (head == "NODES:"){

				int type;
				float gain;												// dummy floats
				long int samples;										// dummy long ints

				nextline >> numberNodes;

				string rootline;
				getline(readFile, rootline); // read header line for nodes
				getline(readFile, rootline); // read first node (root)
				istringstream rootnode(rootline);

				vector<treeNodeRead> treeNodes;
				treeNodeRead currentNode;

				rootnode >> currentNode.num;
				if (currentNode.num !=0){
					cout << "\tRoot is not first node in " << file << ". Exiting." << endl;
					return 0;
				}
				else if(root !=NULL){
					cout << "\tRoot already initialized while loading " << file << "! How did this happen?" << endl;
					return 0;
				}
				else{

					// Load data for the root node
					rootnode >> type;
					if (type == 1) currentNode.type = BRANCH;
					else currentNode.type = LEAF;
					rootnode >> currentNode.depth;
					rootnode >> currentNode.parent;
					rootnode >> currentNode.left;
					rootnode >> currentNode.right;
					rootnode >> currentNode.u.x >> currentNode.u.y;
					rootnode >> currentNode.v.x >> currentNode.v.y;
					rootnode >> currentNode.thresh;
					rootnode >> gain;		// we read it but don't need it
					rootnode >> samples;	// we read it but don't need it
					currentNode.hist = cv::Mat (1,this->parts.count,CV_32FC1);
					for (int i=0;i<this->parts.count;i++){
						rootnode >>	currentNode.hist.at<float>(0,i);
					}

					// Initialize and set root node

					this->root = new treeNode;
					this->root->num = currentNode.num;
					this->root->type = currentNode.type;
					this->root->depth = currentNode.depth;
					this->root->parent = root;
					this->root->left = NULL;
					this->root->right = NULL;
					this->root->u.x = currentNode.u.x;
					this->root->u.y = currentNode.u.y;
					this->root->v.x = currentNode.v.x;
					this->root->v.y = currentNode.v.y;
					this->root->thresh = currentNode.thresh;
					this->root->hist = currentNode.hist.clone();

					treeNodes.push_back(currentNode);		// push root node

					float gain;

					// loop through and insert the remaining nodes
					for (int i = 0; i < numberNodes - 1; i++){
						string nextnode;
						getline(readFile, nextnode); // read next node data
						istringstream nodeline(nextnode);
						nodeline >> currentNode.num;
						nodeline >> type;
						if (type == 1) currentNode.type = BRANCH;
						else currentNode.type = LEAF;
						nodeline >> currentNode.depth;
						nodeline >> currentNode.parent;
						nodeline >> currentNode.left;
						nodeline >> currentNode.right;
						nodeline >> currentNode.u.x >> currentNode.u.y;
						nodeline >> currentNode.v.x >> currentNode.v.y;
						nodeline >> currentNode.thresh;
						nodeline >> gain;
						nodeline >> currentNode.samples;

						currentNode.hist = cv::Mat(1,this->parts.count,CV_32FC1);
						for (int j = 0; j < this->parts.count; j++)
							nodeline >> currentNode.hist.at<float>(0,j);

						treeNodes.push_back(currentNode);
					}
					count = 1 + loadTreeNode(this->root, treeNodes);
				}
			} // NODES
		} // end of file

		if(VERBOSE)
		cout << "\tLoaded " << count << " of " << numberNodes << " nodes from " << file << endl;

		readFile.close();
		return 1;
	}
	cout << "\tCouldn't open file " << file << ". Tree not loaded." << endl;
	return 0;
}
/******************************************************************************/

int DTree::loadTreeNode(treeNode* node, vector<treeNodeRead>& treeNodes)
{
	int insertedNodes = 0;

	if(treeNodes[node->num].left != -1){
		if (node->left == NULL){			// Need to insert left node
			node->left = new treeNode;
			node->left->type = treeNodes[treeNodes[node->num].left].type;
			node->left->left=NULL;    //Set the left child of the child node to null
			node->left->right=NULL;   //Set the right child of the child node to null
			node->left->parent=node;
			node->left->u.x = treeNodes[treeNodes[node->num].left].u.x;
			node->left->u.y = treeNodes[treeNodes[node->num].left].u.y;
			node->left->v.x = treeNodes[treeNodes[node->num].left].v.x;
			node->left->v.y = treeNodes[treeNodes[node->num].left].v.y;
			node->left->thresh = treeNodes[treeNodes[node->num].left].thresh;
			node->left->hist = cv::Mat(1,this->parts.count,CV_32FC1);
			node->left->hist = treeNodes[treeNodes[node->num].left].hist.clone();
			node->left->depth = treeNodes[treeNodes[node->num].left].depth;
			node->left->num = treeNodes[treeNodes[node->num].left].num;

			insertedNodes ++;
			insertedNodes = insertedNodes + loadTreeNode(node->left, treeNodes);

		}
		else{
			cout << "Left child already exists." << endl;
		}
	}
	if(treeNodes[node->num].right != -1){
		if (node->right == NULL){
			node->right = new treeNode;
			node->right->type = treeNodes[treeNodes[node->num].right].type;
			node->right->left=NULL;    //Set the left child of the child node to null
			node->right->right=NULL;   //Set the right child of the child node to null
			node->right->parent=node;
			node->right->u.x = treeNodes[treeNodes[node->num].right].u.x;
			node->right->u.y = treeNodes[treeNodes[node->num].right].u.y;
			node->right->v.x = treeNodes[treeNodes[node->num].right].v.x;
			node->right->v.y = treeNodes[treeNodes[node->num].right].v.y;
			node->right->thresh = treeNodes[treeNodes[node->num].right].thresh;
			node->right->hist = cv::Mat(1,this->parts.count,CV_32FC1);
			node->right->hist = treeNodes[treeNodes[node->num].right].hist.clone();
			node->right->depth = treeNodes[treeNodes[node->num].right].depth;
			node->right->num = treeNodes[treeNodes[node->num].right].num;

			insertedNodes ++;
			insertedNodes = insertedNodes + loadTreeNode(node->right, treeNodes);
		}
		else{
			cout << "Right child already exists." << endl;
		}
	}
	return insertedNodes;
}
/******************************************************************************/

void DTree::destroy_tree(treeNode* leaf){
	if(leaf!=NULL)
	{
		destroy_tree(leaf->left);
		destroy_tree(leaf->right);
		delete leaf;
	}
}
/******************************************************************************/

void DTree::destroy_tree()
{
	destroy_tree(root);
	root = NULL;
}
