#include "header.h"
#include "structure.h"
using namespace std;

extern vector<BLOCK> nodelist;
extern vector<PIN> pinlist;
extern vector<NET> netlist;
extern int NumHardRectilinearBlocks;
extern int NumTerminals;
extern int NumNets;
extern int NumPins;
extern int Total_area;
extern double Outline;
extern double Outline_area;

int Rand(int n)
{
    return rand() % n ;
}

NODE* LeftMost(NODE* node){
    if (node->leftchild == nullptr){
        // cout << node->component.name << " left most" << endl;
        return node;
    }
    else {
        // cout << node->component.name << " find left most" << endl;
        node = LeftMost(node->leftchild);
        return node;
    }
}

NODE* RightMost(NODE* node){
    if (node->rightchild == nullptr){
        // cout << node->block.name << " right most" << endl;
        return node;
    }
    else{
        // cout << node->block.name << " find right most" << endl;
        node = RightMost(node->rightchild);
        return node;
    } 
}

NODE::NODE(BLOCK comp){
    block      = comp;
    parent     = nullptr;
    rightchild = nullptr;
    leftchild  = nullptr;
    rotate     = 0;
    coordinate.x = 0;
    coordinate.y = 0;
}

NODE::NODE(){
    parent     = nullptr;
    rightchild = nullptr;
    leftchild  = nullptr;
}

NODE::~NODE(){
    delete leftchild;
    delete rightchild;
    // cout << "Deleting " << this->block.name << endl;
}

BStarTree::~BStarTree(){
    delete root;
}

void BStarTree::PrintTree(){
    printTree(root);
    // cout << endl;
}

void BStarTree::printTree(NODE *current) {    // Pre-Order Traversal
    if (current){
        // cout << current->block.name << " ";
        cout << current->block.name <<" ("<< current->coordinate.x <<","<<current->coordinate.y<<")\n";
        printTree(current->leftchild);
        printTree(current->rightchild);
    }
    // else cout << "nullptr " ;
}
bool SortArea(BLOCK block1, BLOCK block2){
    return block1.area > block2.area;
}
BStarTree::BStarTree(vector<BLOCK> nodelist_copy){
// BStarTree::BStarTree(unorder_map<string,NODE*> nodelist){
    // sort(nodelist_copy.begin(), nodelist_copy.end(), SortArea);
    random_shuffle(nodelist_copy.begin(),nodelist_copy.end(),Rand);
    // for(const auto &item : nodelist){
    //     cout << item.second->name <<" ("<< item.second->coordinate.x <<","<<item.second->coordinate.y<<")\n";
    // }
    vector<NODE*> Exist_Node;
    NODE *insert;
    NODE *insertNode;
    int branch;
    // auto random_it = next(begin(nodelist), rand() % nodelist.size());
    // root = random_it->second;
    root = new NODE(nodelist_copy.at(0));
    Exist_Node.push_back(root);
    // nodelist.erase(random_it);
    for(int i = 1; i < NumHardRectilinearBlocks; i++){
        branch = rand() % 2;
        insert = Exist_Node.at(rand()%Exist_Node.size());
        insertNode = new NODE(nodelist_copy.at(i));
        if(rand()%2){
            insertNode->rotate = !insertNode->rotate;
        }
        if(branch==0){
            insert = LeftMost(insert);
            insert->leftchild = insertNode;
            insertNode->parent = insert;
        }
        else{
            insert = RightMost(insert);
            insert->rightchild = insertNode;
            insertNode->parent = insert;
        }
        Exist_Node.push_back(insertNode);
    }
    // stack<NODE*> node_stack;
    // node_stack.push(root);
    // while(!node_stack.empty()){
    //     NODE *temp = node_stack.top();
    //     tree_node.insert(pair<string,NODE*>(temp->block.name, temp));
    //     node_stack.pop();
    //     if(temp->rightchild) node_stack.push(temp->rightchild);
    //     if(temp->leftchild) node_stack.push(temp->leftchild);
    // }
}

void BStarTree::getTopLine(NODE* node) {
    int Max_Y = 0;
    // if(node->rotate){
    //    if(contour.size()< node->coordinate.x + node->block.height) 
    //     contour.resize(node->coordinate.x + node->block.height);
    //     for(int i=node->coordinate.x; i< node->coordinate.x + node->block.height; i++){
    //         Max_Y = (Max_Y<=contour.at(i))?contour.at(i): Max_Y;
    //     }
    //     for(int i=node->coordinate.x; i< node->coordinate.x + node->block.height; i++){
    //         contour.at(i) = Max_Y + node->block.width;
    //     }
    //     node->coordinate.y = Max_Y; 
    // }
    // else{
        if(contour.size()< node->coordinate.x + node->place_shape.x) 
        contour.resize(node->coordinate.x + node->place_shape.x);
        for(int i=node->coordinate.x; i< node->coordinate.x + node->place_shape.x; i++){
            Max_Y = (Max_Y<=contour.at(i))?contour.at(i): Max_Y;
        }
        for(int i=node->coordinate.x; i< node->coordinate.x + node->place_shape.x; i++){
            contour.at(i) = Max_Y + node->place_shape.y;
        }
        node->coordinate.y = Max_Y; 
    // }
    
}

int HPWL(vector<Vertex> wire){
    int max_x=0, max_y=0, min_x=9999, min_y=9999;
    for(const auto node: wire){
        max_x = (node.x > max_x)? node.x : max_x;
        max_y = (node.y > max_y)? node.y : max_y;
        min_x = (node.x > min_x)? min_x : node.x;
        min_y = (node.y > min_y)? min_y : node.y;
    }
    return (max_x-min_x)+(max_y-min_y);
}

void BStarTree::ComputeWire(){
    vector<Vertex> wire;
    Vertex temp;
    NODE *node;
    TotalWirelength = 0;
    for(const auto net : netlist){
        wire.clear();
        for(const auto pin : net.netarray_pin){
            temp = pin.coordinate;
            wire.push_back(temp);
        }
        for(const auto block : net.netarray_node){
            node = tree_node.at(block.name);
            temp.x = node->coordinate.x + node->place_shape.x/2;
            temp.y = node->coordinate.y + node->place_shape.y/2;
            wire.push_back(temp);
        }
        TotalWirelength += HPWL(wire);
    }
}

void BStarTree::Evaluate(){
    // vector<int> old_contour = contour;
    region.x = 0;
    region.y = 0;
    stack<NODE*> node_stack;
    node_stack.push(root);
    tree_node.clear();
    contour.clear();
    int penalty = 0;
    while(!node_stack.empty()){
        NODE *temp = node_stack.top();
        if(temp->rotate == 1){
            temp->place_shape.x = temp->block.height;
            temp->place_shape.y = temp->block.width;
        }
        else{
            temp->place_shape.x = temp->block.width;
            temp->place_shape.y = temp->block.height;
        }
        if(temp->parent){
            if(temp->parent->leftchild == temp){
                temp->coordinate.x = temp->parent->coordinate.x + temp->parent->place_shape.x;
                getTopLine(temp);
                // cout << temp->block.name <<" ("<< temp->coordinate.x <<","<<temp->coordinate.y<<")\n";
                //temp->coordinate.x + temp->place_shape.x - Outline +
                //temp->coordinate.y + temp->place_shape.y - Outline +
                if(Outline < temp->coordinate.x) penalty += 2;
                else if(Outline < temp->coordinate.x + temp->place_shape.x) penalty += 1;//temp->block.area/2;
                if(Outline < temp->coordinate.y) penalty += 2;
                else if(Outline < temp->coordinate.y + temp->place_shape.y) penalty += 1;//temp->block.area/2;
                // if((Outline < temp->coordinate.y + temp->place_shape.y) || 
                // (Outline < temp->coordinate.x + temp->place_shape.x)) penalty += temp->block.area/(Total_area/NumHardRectilinearBlocks);
            }
            else if (temp->parent->rightchild == temp){
                temp->coordinate.x = temp->parent->coordinate.x;
                getTopLine(temp);
                if(Outline < temp->coordinate.x) penalty += 2;
                else if(Outline < temp->coordinate.x + temp->place_shape.x) penalty += 1;//temp->block.area/2;
                if(Outline < temp->coordinate.y) penalty += 2;
                else if(Outline < temp->coordinate.y + temp->place_shape.y) penalty += 1;//temp->block.area/2;
                // cout << temp->block.name <<" ("<< temp->coordinate.x <<","<<temp->coordinate.y<<")\n";
                // if((Outline < temp->coordinate.y + temp->place_shape.y) || 
                // (Outline < temp->coordinate.x + temp->place_shape.x)) penalty += temp->block.area/(Total_area/NumHardRectilinearBlocks);
            }
        }
        else{
            temp->coordinate.x = 0;
            temp->coordinate.y = 0;
            contour.resize(temp->place_shape.x);
            for(int i = 0; i<temp->place_shape.x; i++){
                contour.at(i) = temp->place_shape.y;
            }
        }
        region.x = (region.x > temp->coordinate.x + temp->place_shape.x)? region.x : temp->coordinate.x +temp->place_shape.x;
        region.y = (region.y > temp->coordinate.y + temp->place_shape.y)? region.y : temp->coordinate.y + temp->place_shape.y;
        node_stack.pop();
        tree_node.insert(pair<string,NODE*>(temp->block.name, temp));
        if(temp->rightchild) {
            node_stack.push(temp->rightchild);
        }
        if(temp->leftchild){
            node_stack.push(temp->leftchild);
        } 
    }
    // if(region.x>Outline) penalty +=  region.x/Outline;
    // if(region.y>Outline) penalty +=  region.y/Outline;

    if(Outline-region.x>3) penalty += 2;
    else if(Outline-region.x>2) penalty += 1;
    if(Outline-region.y>3) penalty += 2;
    else if(Outline-region.y>2) penalty += 1;

    Area = region.x * region.y;
    Aspect_ratio = double(region.y / region.x);
    // cout<<tree_node.size()<<endl;
    this->ComputeWire();
    // fitness = penalty;
    fitness = alpha*Area/Outline_area + beta*TotalWirelength/(NumHardRectilinearBlocks*2000) + delta*(Aspect_ratio-1)*(Aspect_ratio-1) + quota*penalty;
}

BStarTree* BStarTree::CopyTree(BStarTree* tree){
    BStarTree* new_tree = new BStarTree;
    stack<NODE*> old_tree_stack;
    stack<NODE*> new_tree_stack;
    new_tree->fitness = tree->fitness;
    new_tree->region.x = tree->region.x;
    new_tree->region.y = tree->region.y;
    old_tree_stack.push(tree->root);
    new_tree_stack.push(new_tree->root);
    new_tree->tree_node.clear();
    while (old_tree_stack.empty() == false)
    {
        NODE* old_temp = old_tree_stack.top();
        NODE* new_temp = new_tree_stack.top();

        new_temp->block = old_temp->block;
        new_temp->rotate = old_temp->rotate;
        old_tree_stack.pop();
        new_tree_stack.pop();
        new_tree->tree_node.insert(pair<string,NODE*>(new_temp->block.name,new_temp));
        if (old_temp->rightchild)
        {
            new_temp->rightchild = new NODE;
            new_temp->rightchild->parent = new_temp;
            old_tree_stack.push(old_temp->rightchild);
            new_tree_stack.push(new_temp->rightchild);
        }
            
        if (old_temp->leftchild)
        {
            new_temp->leftchild = new NODE;
            new_temp->leftchild->parent = new_temp;
            old_tree_stack.push(old_temp->leftchild);
            new_tree_stack.push(new_temp->leftchild);
        }
    }
    return new_tree;
}

void BStarTree::Rotate(){
    BLOCK position = nodelist.at(rand() % nodelist.size());
    // int position = rand() % nodelist.size();
    tree_node.at(position.name)->rotate = ~tree_node.at(position.name)->rotate;
    // this->Evaluate();
}

void BStarTree::SwapNode(NODE* node1, NODE* node2){
    // swap two parent
    if(node1->parent){
        if(node1->parent->leftchild == node1){
            node1->parent->leftchild = node2;
        }
        else{
            node1->parent->rightchild = node2;
        }
    }
    if(node2->parent){
        if(node2->parent->leftchild == node2){
            node2->parent->leftchild = node1;
        }
        else{
            node2->parent->rightchild = node1;
        }
    }
    NODE *temp = node1->parent;
    node1->parent = node2->parent;
    node2->parent = temp;
    // swap two child
    temp = node1->leftchild;
    node1->leftchild = node2->leftchild;
    node2->leftchild = temp;

    temp = node1->rightchild;
    node1->rightchild = node2->rightchild;
    node2->rightchild = temp;

    if(node1->rightchild) node1->rightchild->parent = node1;
    if(node1->leftchild) node1->leftchild->parent = node1;
    if(node2->rightchild) node2->rightchild->parent = node2;
    if(node2->leftchild) node2->leftchild->parent = node2;
    // swap parent and child
    if(node1->parent == node1) node1->parent = node2;
    else if(node1->leftchild == node1) node1->leftchild = node2;
    else if(node1->rightchild == node1) node1->rightchild = node2; 
    
    if(node2->parent == node2) node2->parent = node1;
    else if(node2->leftchild == node2) node2->leftchild = node1;
    else if(node2->rightchild == node2) node2->rightchild = node1; 
    // root may change
    if(node1 == root) root = node2;
    else if(node2 == root) root = node1;
}

void BStarTree::DeleteNode(NODE* delete_node){
    if(delete_node->leftchild == nullptr && delete_node->rightchild == nullptr){
        // cout << "case 1" << endl;
        if(delete_node->parent){
            if(delete_node->parent->leftchild == delete_node){
                delete_node->parent->leftchild = nullptr; 
            } 
            else{
                delete_node->parent->rightchild = nullptr;
            }  
        }
    }
    else if(delete_node->leftchild == nullptr){
        // cout << "case 2" << endl;
        if(delete_node->parent){
            if(delete_node->parent->leftchild == delete_node){
                delete_node->parent->leftchild = delete_node->rightchild;
                delete_node->rightchild->parent = delete_node->parent;
            } 
            else{
                delete_node->parent->rightchild = delete_node->rightchild;
                delete_node->rightchild->parent = delete_node->parent;
            }  
        }
        else{
            root = delete_node->rightchild;
            delete_node->rightchild->parent = nullptr;
        }
    }
    else if(delete_node->rightchild == nullptr){
        // cout << "case 3" << endl;
        if(delete_node->parent){
            if(delete_node->parent->leftchild == delete_node){
                delete_node->parent->leftchild = delete_node->leftchild;
                delete_node->leftchild->parent = delete_node->parent;
            } 
            else{
                delete_node->parent->rightchild = delete_node->leftchild;
                delete_node->leftchild->parent = delete_node->parent;
            }  
        }
        else{
            root = delete_node->leftchild;
            delete_node->leftchild->parent = nullptr;
        }
    }
    else{
        // cout << "case 4" << endl;
        // cout << root->block.name << delete_node->block.name << endl;
        NODE* leaf = delete_node;
        while(leaf->leftchild != nullptr || leaf->rightchild != nullptr){
            bool move_left;
            if(leaf->leftchild != nullptr && leaf->rightchild != nullptr) move_left = rand()%2;
            else if(leaf->leftchild != nullptr) move_left = 1;
            else move_left = 0;

            if(move_left){
                SwapNode(leaf,leaf->leftchild);
            }
            else{
                SwapNode(leaf,leaf->rightchild);
            }
        }
        // cut the leaf
        if(delete_node->parent->leftchild == delete_node){
            delete_node->parent->leftchild = nullptr;
        }
        else {
            delete_node->parent->rightchild = nullptr;
        }
    }    
    // delete_node->parent = nullptr;
    // delete_node->leftchild = nullptr;
    // delete_node->rightchild = nullptr;
    // delete delete_node;
    
}

void BStarTree::InsertNode(NODE* insert_node,NODE* dest_node){
    int insert_type = rand() % 4;
    NODE* temp;
    // insert_node->parent = dest_node;
    switch (insert_type){
    case 0:
        temp = dest_node->leftchild;
        insert_node->leftchild = temp;
        insert_node->rightchild = nullptr;
        dest_node->leftchild = insert_node;
        break;
    case 1:
        temp = dest_node->rightchild;
        insert_node->leftchild = temp;
        insert_node->rightchild = nullptr;
        dest_node->rightchild = insert_node;
        break;
    case 2:
        temp = dest_node->leftchild;
        insert_node->leftchild = nullptr; 
        insert_node->rightchild = temp;
        dest_node->leftchild = insert_node;
        break;
    case 3:
        temp = dest_node->rightchild;
        insert_node->leftchild = nullptr;
        insert_node->rightchild = temp;
        dest_node->rightchild = insert_node;
    }
    insert_node->parent = dest_node;
    if(temp){
        // cout << temp->block.name << endl;
        temp->parent = insert_node;
    }
}

void BStarTree::Move(){
    BLOCK position_1 = nodelist.at(rand()% tree_node.size()) ;
    BLOCK position_2 = nodelist.at(rand()% tree_node.size()) ;
    while (position_1.name == position_2.name){
        position_1 = nodelist.at(rand()% tree_node.size()) ;
        position_2 = nodelist.at(rand()% tree_node.size()) ;
    }
    NODE* select_node = tree_node.at(position_1.name);
    NODE* dest_node = tree_node.at(position_2.name);
    // NODE* select_node_copy = new NODE;
    // select_node_copy->block = select_node->block;
    // cout << "Delete" <<select_node->block.name << dest_node->block.name << endl;
    this->DeleteNode(select_node);
    this->InsertNode(select_node, dest_node);
    // cout << "Insert" <<select_node->block.name << dest_node->block.name << endl;
    
    // this->PrintTree();
    // stack<NODE*> node_stack;
    // node_stack.push(root);
    // tree_node.clear();
    // while(!node_stack.empty()){
    //     NODE *temp = node_stack.top();
    //     tree_node.insert(pair<string,NODE*>(temp->block.name, temp));
    //     node_stack.pop();
    //     if(temp->rightchild) node_stack.push(temp->rightchild);
    //     if(temp->leftchild) node_stack.push(temp->leftchild);
    // }
    // cout<<"tree = " <<tree_node.size() <<endl;
    // this->Evaluate();
}

void BStarTree::Swap(){
    BLOCK position_1 = nodelist.at(rand()% tree_node.size()) ;
    BLOCK position_2 = nodelist.at(rand()% tree_node.size()) ;
    while (position_1.name == position_2.name){
        position_1 = nodelist.at(rand()% tree_node.size()) ;
        position_2 = nodelist.at(rand()% tree_node.size()) ;
    }
    NODE* select_node = tree_node.at(position_1.name);
    NODE* dest_node = tree_node.at(position_2.name);
    SwapNode(select_node,dest_node);
    // this->Evaluate();
}

void BStarTree::RotateBranch(){
    BLOCK position_1 = nodelist.at(rand()% tree_node.size()) ;
    NODE* select_node = tree_node.at(position_1.name);
    NODE* temp = select_node->leftchild;
    select_node->leftchild = select_node->rightchild;
    select_node->rightchild = temp;
    // this->Evaluate();
}

int BStarTree::CountLevel(NODE* node){
    int level = 0;
    NODE *temp = node;
    while (temp != root) {
        level++;
        temp = temp->parent;
    }
    return level;
}

bool BStarTree::CheckAncestor(NODE* node1, NODE* node2){
    bool check = 1;
    NODE *temp = node1;
    if(node1 == root || node2 == root) check = 0;
    while (temp->parent != root){
        if(temp == node2) check =  0;
        temp = temp->parent;
    }
    temp = node2;
    while (temp->parent != root){
        if(temp == node1) check = 0;
        temp = temp->parent;
    }
    if(CountLevel(node1)>5 && CountLevel(node1)>5) check = 1;
    else check = 0;
    return check;
}

void BStarTree::SwapBranch(){
    BLOCK position_1 = nodelist.at(rand()% tree_node.size()) ;
    BLOCK position_2 = nodelist.at(rand()% tree_node.size()) ;
    NODE* node1 = tree_node.at(position_1.name);
    NODE* node2 = tree_node.at(position_2.name);
    while (CheckAncestor(node1, node2)){
        position_1 = nodelist.at(rand()% tree_node.size()) ;
        position_2 = nodelist.at(rand()% tree_node.size()) ;
        node1 = tree_node.at(position_1.name);
        node2 = tree_node.at(position_2.name);
    }

    if(node1->parent->leftchild && node1->parent->leftchild == node1)
        node1->parent->leftchild = node2;
    else if(node1->parent->rightchild && node1->parent->rightchild == node1) 
        node1->parent->rightchild = node2;
    if(node2->parent->leftchild && node2->parent->leftchild == node2)
        node2->parent->leftchild = node1;
    else if(node2->parent->rightchild && node2->parent->rightchild == node2) 
        node2->parent->rightchild = node1;
    NODE* temp = node1->parent;
    node1->parent = node2->parent;
    node2->parent = temp;

    // this->Evaluate();

}

void BStarTree::Perturbation(){
    int operation = rand() % 4;
    // BStarTree* perturb_tree = CopyTree(this);
    switch (operation){
    case 0:
        this->Rotate();
        break;
    case 1:
        this->Move();
        break;
    case 2:
        this->Swap();
        break;
    case 3:
        this->RotateBranch();
        break;
    }
}

BStarTree *GetLeftSubtree(BStarTree* tree){
    BStarTree* new_tree = new BStarTree;
    stack<NODE*> old_tree_stack;
    stack<NODE*> new_tree_stack;
    new_tree->tree_node.clear();
    old_tree_stack.push(tree->root);
    new_tree_stack.push(new_tree->root);
    bool flag = 0;
    while (old_tree_stack.empty() == false)
    {
        NODE* old_temp = old_tree_stack.top();
        NODE* new_temp = new_tree_stack.top();

        new_temp->block = old_temp->block;
        new_temp->rotate = old_temp->rotate;
        old_tree_stack.pop();
        new_tree_stack.pop();
        new_tree->tree_node.insert(pair<string,NODE*>(new_temp->block.name, new_temp));
        if (old_temp->rightchild && flag){
            new_temp->rightchild = new NODE;
            new_temp->rightchild->parent = new_temp;
            old_tree_stack.push(old_temp->rightchild);
            new_tree_stack.push(new_temp->rightchild);
        }
        if (old_temp->leftchild){
            new_temp->leftchild = new NODE;
            new_temp->leftchild->parent = new_temp;
            old_tree_stack.push(old_temp->leftchild);
            new_tree_stack.push(new_temp->leftchild);
        }
        flag = 1;
    }
    return new_tree;
}

BStarTree *BStarTree::leftSubtreeCrossover(BStarTree *parent_2){
    
    BStarTree* offspring = GetLeftSubtree(this);
    BStarTree* right_subtree = CopyTree(parent_2);
    unordered_map<string, NODE*>::iterator find_node;
    int i = 0;
    // cout<< "after offspring = "<<offspring->tree_node.size() <<endl;
    // cout<< "after right_subtree = "<<right_subtree->tree_node.size() <<endl;
    for(auto &item : right_subtree->tree_node){
        // find_node = offspring->tree_node.find(item.second->block.name);
        if ( offspring->tree_node.find(item.second->block.name) != offspring->tree_node.end() ) {
            right_subtree->DeleteNode(item.second);
            i++;
            // cout<<"delete "<<i<<endl;
        }
    }

    //****************  stay   *****************//
    // cout<< "offspring = "<<offspring->tree_node.size() <<endl;
    offspring->root->rightchild = right_subtree->root;
    right_subtree->root->parent = offspring->root;
    BStarTree* offspringC =  CopyTree(offspring);
    // cout<< "offspring = "<<offspringC->tree_node.size() <<endl;
    // delete offspring;
    // delete right_subtree;
    //****************   stay   *****************//
    // cout << "test1" << endl;
    offspringC->Evaluate();
    // cout << "bug" << endl;
    
    return offspringC;
}
