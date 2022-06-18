#ifndef STRUCTURE_H
#define STRUCTURE_H

//********************** Vertex and Line ***************//
typedef struct Vertex {
    int x;
    int y;
}Vertex;
//******************** BLOCK ****************************//
class BLOCK{
public:
    string name;
    double width;
    double height;
    double area;
};
//******************** NODE *****************************//
class NODE
{
private:
    /* data */
public:
    BLOCK block;
    bool rotate;    
    Vertex coordinate;
    Vertex place_shape;
    NODE *parent;
    NODE *rightchild;
    NODE *leftchild;
    NODE(BLOCK);
    NODE();
    ~NODE();
    // friend class BStarTree;
};
//*********************** PIN ************************//
class PIN
{   
    public:
        string name;
        Vertex coordinate;
        PIN(){};
};
//*********************** NETS **********************//
class NET
{
    public:
        vector<PIN> netarray_pin;
        vector<BLOCK> netarray_node;

};
//******************** Contour **************************//
//******************** BStarTree ************************//
class BStarTree
{
private:
    /* data */
    void printTree(NODE *current);
    //********** Three operator of perturbing B*-tree ***********//
    void Rotate();
    void Move();
    void Swap();
    void SwapBranch();
    void RotateBranch();
    void getTopLine(NODE*);
    void ComputeWire();
    int CountLevel(NODE*);
    bool CheckAncestor(NODE*,NODE*);
public:
    NODE *root;
    unordered_map<string,NODE*>tree_node;
    vector<int>contour;
    Vertex region;
    int Area;
    double Aspect_ratio;
    double fitness;
    int TotalWirelength;
    // BStarTree(map<string,NODE*>);
    BStarTree(vector<BLOCK>);
    ~BStarTree();
    BStarTree():root(new NODE){};
    void PrintTree();
    void DeleteNode(NODE*);
    void SwapNode(NODE*, NODE*);
    void InsertNode(NODE*,NODE*);
    void Evaluate();
    void Perturbation();
    BStarTree *CopyTree(BStarTree*);
    BStarTree *leftSubtreeCrossover(BStarTree *);
};



#endif