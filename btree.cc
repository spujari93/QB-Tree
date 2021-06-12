// Project: B*-trees floorplanning
// Advisor: Yao-Wen Chang  <ywchang@cis.nctu.edu.tw>
// Authors: Jer-Ming Hsu   <barz@cis.nctu.edu.tw>
// 	    Hsun-Cheng Lee <gis88526@cis.nctu.edu.tw>
// Sponsor: Arcadia Inc.
// Date:    7/19/2000 ~

//---------------------------------------------------------------------------
#include <stack>
#include <algorithm>
#include <cmath>
#include <iostream>
#include <climits>
#include <string.h>
#include "btree.h"

using namespace std;
//---------------------------------------------------------------------------
float rotate_rate = 0.3;
float swap_rate = 0.5;

//---------------------------------------------------------------------------
//   Initialization
//---------------------------------------------------------------------------

void B_Tree::clear() {
    // initial contour value
    FPlan::clear();
    contour.clear();
}

//Initialize B*tree
void B_Tree::init() {
    TotalArea = 0;
    variants.clear();
    variants.reserve(vector_max_size);
    // initialize b*tree by complete binary tree
    vector<int> inds;
    for (int i = 0; i < modules_N; i++) {
        inds.push_back(i);
        TotalArea += modules[i].area;
    }
    initWithNodeIndices(inds);
    best_sol.clear();
    last_sol.clear();
    clear();
    //show_tree();
}

//Calculate total area of placement
void B_Tree::calcTotalArea()
{
    TotalArea = 0;
    auto nodes = allnodes();
    for (int i = 0; i < nodes.size(); i++)
    {
        TotalArea += modules[nodes[i]->id].area;
    }
}

//Get the total node size
int B_Tree::getNodesCount()
{
    return allnodes().size();
}

//Initialize B*tree without node indice
void B_Tree::initWithOutNode()
{
    contour.reserve(vector_max_size);
    contour.resize(modules.size());
    modules_info.resize(modules_N);
    variants.clear();
    variants.reserve(vector_max_size);
    TotalArea = 0;
    for (int i = 0; i < modules_N; i++) {
        TotalArea += modules[i].area;
    }
    best_sol.clear();
    last_sol.clear();
    clear();
    auto nodes = allnodes();
    nodes_N = nodes.size();

    packing();
}

//Initialize B*tree with node indice
void B_Tree::initWithNodeIndices(vector<int> indices)
{
    int j;
    contour.reserve(vector_max_size);
    contour.resize(modules.size());
    modules_info.resize(modules_N);

    TotalArea = 0;

    vector<Node*> nodes;

    // set every id of nodes
    for (int i = 0; i < indices.size(); i++)
    {
        Node* node = (Node*) new unsigned char[sizeof(Node)];
        memset(node, 0, sizeof(Node));
        nodes.push_back(node);
        node = nodes.back();
        node->id = indices[i];

        // insert variant constraint info to the node.
        if (!variants.empty()) {
            for (int t = 0; t < variants.size(); t++)
            {
                if (variants[t].mod == indices[i]) {
                    j = rand() % variants[t].ratios.size();
                    node->ratio = variants[t].ratios[j];
                }
            }
        }
        node->parent = nodes[(i + 1) / 2 - 1];
        TotalArea += modules[indices[i]].area;
    }

    // set left&right child
    for (int i = 0; i < indices.size(); i++)
    {
        nodes[i]->left = (2 * i + 1 < indices.size() ? nodes[2 * i + 1] : nullptr);
        nodes[i]->right = (2 * i + 2 < indices.size() ? nodes[2 * i + 2] : nullptr);
    }

    // set root node
    nodes_root = nodes[0];
    nodes[0]->parent = nullptr;
    nodes_N = nodes.size();

    // init
    best_sol.clear();
    last_sol.clear();
    clear();

    normalize_cost(10);
}

//---------------------------------------------------------------------------
//   Placement modules
//---------------------------------------------------------------------------

void B_Tree::packing() {
    stack<Node*> S;

    clear();
    Node* p = nodes_root;
    place_module(p, nullptr);
    Node* n = p;
    if (n->right != nullptr)      S.push(n->right);
    if (n->left != nullptr)      S.push(n->left);

    // inorder traverse
    while (!S.empty()) {
        p = S.top();
        S.pop();
        Node* n = p;

        assert(n->parent != nullptr);
        bool is_left = (n->parent->left == n);
        place_module(p, n->parent, is_left);
        if (n->right != nullptr)      S.push(n->right);
        if (n->left != nullptr)      S.push(n->left);
    }

    // compute Width, Height
    double max_x = -1, max_y = -1;
    /*for(Node* p = nodes_root; p!=nullptr; p = p->contour->front) {
      max_x = max(max_x,double(modules_info[p->id].rx));
      max_y = max(max_y,double(modules_info[p->id].ry));
    }*/
    auto nodes = allnodes();
    for (int i = 0; i < nodes.size(); i++) {
        max_x = max(max_x, double(modules_info[nodes[i]->id].rx));
        max_y = max(max_y, double(modules_info[nodes[i]->id].ry));
    }

    Width = max_x;
    Height = max_y;
    Area = Height * Width;

    FPlan::packing(); 	// for wirelength  
}

//********** Display B* tree **********//
void B_Tree::show_tree()
{
    auto nodes = allnodes();
    cout << "root : " << nodes[0]->id << endl;
    for (int i = 0; i < nodes.size(); i++) {
        cout << nodes[i]->id << ": ";
        cout << (nodes[i]->left == nullptr ? -1 : nodes[i]->left->id) << " ";
        cout << (nodes[i]->parent == nullptr ? -1 : nodes[i]->parent->id) << " ";
        cout << (nodes[i]->right == nullptr ? -1 : nodes[i]->right->id) << endl;
    }
}

// is_left: default is true
void B_Tree::place_module(Node* mod, Node* abut, bool is_left) {
    Module_Info& mod_mf = modules_info[mod->id];
    int min_flag = NIL;
    int dis;
    int r;
    mod_mf.rotate = mod->rotate;
    mod_mf.flip = mod->flip;

    int w = modules[mod->id].width;
    int h = modules[mod->id].height;

    if (!fixed_bndries.empty())
    {
        for (int i = 0; i < fixed_bndries.size(); i++)
        {
            if (mod->id == fixed_bndries[i].mod)
            {
                mod->rotate = fixed_bndries[i].rotate;
                break;
            }
        }
    }

    if (mod->rotate)
        swap(w, h);

    // Variant constraint 
    if (mod->ratio != 0)
    {
        h = w * mod->ratio;
    }

    // [1]. root node case
    if (abut == nullptr) {	// root node
        if (!min_seps.empty()) {
            for (int i = 0; i < min_seps.size(); i++) {
                if (mod->id == min_seps[i].mod) {
                    min_flag = 4;
                    dis = min_seps[i].dis;
                    break;
                }
            }
        }
        contour_root = mod->id;
        contour[mod->id].back = NIL;
        contour[mod->id].front = NIL;
        if (min_flag == 4)
        {
            mod_mf.x = mod_mf.y = dis;
            mod_mf.rx = dis + w, mod_mf.ry = dis + h;
        }
        else {
            mod_mf.x = mod_mf.y = 0;
            mod_mf.rx = w, mod_mf.ry = h;
        }
        return;
    }

    // minimum seperation constraint.
    if (!min_seps.empty()) {
        for (int i = 0; i < min_seps.size(); i++) {
            if (mod->id == min_seps[i].mod && abut != nullptr)
                min_flag = 0;
            else if (mod->parent->id == min_seps[i].mod && is_left)
                min_flag = 1;
            else if (mod->parent->id == min_seps[i].mod && !is_left)
                min_flag = 2;
            dis = min_seps[i].dis;
            break;
        }
    }

    // [2]. child node case
    int p;   // trace contour from p

    if (is_left) {	// left
        int abut_width = (abut->rotate ? modules[abut->id].height : modules[abut->id].width);

        if (min_flag == 0 || min_flag == 1) // left
        {
            mod_mf.x = modules_info[abut->id].x + abut_width + dis;
        }
        else
            mod_mf.x = modules_info[abut->id].x + abut_width;
        mod_mf.rx = mod_mf.x + w;
        //p = abut->left;// abut->contour->front;
        p = contour[abut->id].front;

        contour[abut->id].front = mod->id;
        contour[mod->id].back = abut->id;


        if (p == NIL) {  // no obstacle in X axis
            if (min_flag == 1)
            {
                mod_mf.y = dis;
                mod_mf.ry = mod_mf.y + h;
            }
            else {
                mod_mf.y = 0;
                mod_mf.ry = h;
            }
            contour[mod->id].front = NIL;
            return;
        }
    }
    else {	// upper
        if (min_flag == 0)
            mod_mf.x = modules_info[abut->id].x + dis;
        else
            mod_mf.x = modules_info[abut->id].x;
        mod_mf.rx = mod_mf.x + w;
        p = abut->id;

        int n = contour[abut->id].back;

        if (n == NIL) { // i.e, mod_mf.x==0
            contour_root = mod->id;
            contour[mod->id].back = NIL;
        }
        else {
            contour[n].front = mod->id;
            contour[mod->id].back = n;
        }
    }

    int min_y = INT_MIN;
    int bx, by;
    assert(p != NIL);

    for (; p != NIL; p = contour[p].front)
    {
        bx = modules_info[p].rx;
        by = modules_info[p].ry;
        min_y = max(min_y, by);

        if (bx >= mod_mf.rx) { 	// update contour
            mod_mf.y = min_y;
            mod_mf.ry = mod_mf.y + h;
            for (int i = 0; i < min_seps.size(); i++)
            {
                if (p == min_seps[i].mod)
                {
                    mod_mf.y = min_y + dis;
                    mod_mf.ry = mod_mf.y + h;
                    break;
                }
            }
            if (bx > mod_mf.rx) {
                contour[mod->id].front = p;
                contour[p].back = mod->id;
            }
            else { 			// bx==mod_mf.rx
                int n = contour[p].front;
                contour[mod->id].front = n;
                if (n != NIL)
                    contour[n].back = mod->id;
            }
            break;
        }
    }

    if (p == NIL) {
        mod_mf.y = (min_y == INT_MIN ? 0 : min_y);
        mod_mf.ry = mod_mf.y + h;
        contour[mod->id].front = NIL;
    }

    if ((min_flag == 2 || min_flag == 0 || min_flag == 1) && mod_mf.y < (modules_info[mod->parent->id].ry + dis))
    {
        mod_mf.y += dis;
        mod_mf.ry = mod_mf.y + h;
    }
    else if (min_flag != NIL)
    {
        mod_mf.y += dis;
        mod_mf.ry = mod_mf.y + h;
    }

}

//---------------------------------------------------------------------------
//   Manipulate B*Tree auxilary procedure
//---------------------------------------------------------------------------

void B_Tree::wire_nodes(Node* parent, Node* child, DIR edge) {
    assert(parent != nullptr);
    (edge == LEFT ? parent->left : parent->right) = child;
    if (child != nullptr) child->parent = parent;
}

Node* B_Tree::child(Node* node, DIR d) {
    assert(node != nullptr);
    return (d == LEFT ? node->left : node->right);
}


//---------------------------------------------------------------------------
//   Simulated Annealing Temporal Solution
//---------------------------------------------------------------------------

void B_Tree::get_solution(Solution& sol) {
    sol.nodes = vector<Node>();
    copyTree(sol.nodes);
    sol.nodes_root = &sol.nodes[0];
    sol.cost = getCost();
}

void B_Tree::keep_sol() {
    get_solution(last_sol);
}

void B_Tree::keep_best() {
    get_solution(best_sol);
}

void B_Tree::recover() {
    recover(last_sol);
}

void B_Tree::recover_best() {
    recover(best_sol);
}

void B_Tree::recover(Solution& sol) {
    nodes_root = sol.nodes_root;
    if (prev_tree)
        prev_tree->clear();
    vector<Node>* tree = new vector<Node>();

    copyTree(*tree);
    prev_tree = tree;
    nodes_root = &(tree->at(0));
}


//---------------------------------------------------------------------------
//   Simulated Annealing Permutation Operations
//---------------------------------------------------------------------------

void B_Tree::perturb() {
    auto nodes = allnodes();
    if (nodes.size() < 4)
        return;

    int p, n;
    n = rand() % nodes.size();  //modules_N;

  //  changed_nodes.clear();
  //  changed_root = NIL;


    if (rotate_rate > rand_01()) {
        //    changed_nodes.push_back(nodes[n]);
        nodes[n]->rotate = !nodes[n]->rotate;
        if (rand_bool()) nodes[n]->flip = !nodes[n]->flip;
    }
    else {

        if (swap_rate > rand_01()) {
            do {
                p = rand() % nodes.size(); //modules_N;
            } while (n == p || nodes[n]->parent == nodes[p] || nodes[p]->parent == nodes[n]);

            //      changed_nodes.push_back(nodes[p]);
            //      changed_nodes.push_back(nodes[n]);

            swap_node(nodes[p], nodes[n]);   // [TODO]. refer the swap in vector

        }
        else {
            do {
                p = rand() % nodes.size(); //modules_N;
            } while (n == p);

            //      changed_nodes.push_back(nodes[p]);
            //      changed_nodes.push_back(nodes[n]);

            delete_node(nodes[n]);           // [TODO]. refer the delete in vector
            insert_node(nodes[p], nodes[n]); // [TODO]. refer the insert in vector
        }
    }

}

void B_Tree::swap_node(Node* n1, Node* n2) {

    if (n1->left != nullptr) {
        //add_changed_nodes(n1.left);
        n1->left->parent = n2;
    }
    if (n1->right != nullptr) {
        //add_changed_nodes(n1.right);
        n1->right->parent = n2;
    }
    if (n2->left != nullptr) {
        //add_changed_nodes(n2.left);
        n2->left->parent = n1;
    }
    if (n2->right != nullptr) {
        //add_changed_nodes(n2.right);
        n2->right->parent = n1;
    }

    if (n1->parent != nullptr) {
        //add_changed_nodes(n1.parent);
        if (n1->parent->left == n1)
            n1->parent->left = n2;
        else
            n1->parent->right = n2;
    }
    else {
        changed_root = n1;
        nodes_root = n2;
    }

    if (n2->parent != nullptr) {
        //add_changed_nodes(n2.parent);
        if (n2->parent->left == n2)
            n2->parent->left = n1;
        else
            n2->parent->right = n1;
    }
    else {
        //    changed_root = n2.id;
        nodes_root = n1;
    }

    swap(n1->left, n2->left);
    swap(n1->right, n2->right);
    swap(n1->parent, n2->parent);
}

// Returns a node with given id
Node* B_Tree::find_node_by_id(int moduleID)
{
    auto nodes = allnodes();
    for (int i = 0; i < nodes.size(); i++)
        if (nodes[i]->id == moduleID)
            return nodes[i];
    return nullptr;
}

// Returns a random node
Node* B_Tree::find_node_random()
{
    auto nodes = allnodes();
    if (nodes.empty() || nodes.size() == 0)
        return nullptr;
    int i = rand() % nodes.size();
    return nodes[i];
}

// To insert a node with given id
void B_Tree::insertNodeById(Node* parent, int moduleId)
{
    int j;
    // [0]. If there is no node,inserted node is root node.
    if (parent == nullptr) {
        vector<int> inds;
        inds.push_back(moduleId);
        initWithNodeIndices(inds);
        return;
    }
    // [1]. check if the module ID is exist already.
    auto nodes = allnodes();
    for (int i = 0; i < nodes.size(); i++)
        if (nodes[i]->id == moduleId)
            return;
    // [2]. make new node.
    Node* node = (Node*) new unsigned char[sizeof(Node)];
    memset(node, 0, sizeof(Node));
    node->id = moduleId;
    // insert variant constraint info to the node.
    if (!variants.empty()) {
        for (int t = 0; t < variants.size(); t++)
        {
            if (variants[t].mod == moduleId) {
                j = rand() % variants[t].ratios.size();
                node->ratio = variants[t].ratios[j];
                break;
            }
        }
    }
    // [3]. insert new node.
    insert_node(parent, node);
    nodes_N = allnodes().size();
}

// To swap two nodes with given ids
void B_Tree::swap_node_by_id(int m1, int m2)
{
    Node* n1 = find_node_by_id(m1);
    Node* n2 = find_node_by_id(m2);
    Node* t;
    if (n1 == nullptr || n2 == nullptr) {
        cout << "incorrect mod number." << endl;
    }

    if (n1->parent == n2 || n2->parent == n1)
    {
        swap(n1, n2);

        if (n1->parent == nullptr) {
            nodes_root = n1;
        }
        else if (n2->parent == nullptr) {
            nodes_root = n2;
        }

    }
    else {
        swap_node(n1, n2);
    }

}
// To insert node
void B_Tree::insert_node(Node* parent, Node* node) {
    node->parent = parent;
    bool edge = rand_bool();

    if (edge) {
        //add_changed_nodes(parent.left);
        node->left = parent->left;
        node->right = nullptr;
        if (parent->left != nullptr)
            parent->left->parent = node;

        parent->left = node;

    }
    else {
        //add_changed_nodes(parent.right);
        node->left = nullptr;
        node->right = parent->right;
        if (parent->right != nullptr)
            parent->right->parent = node;

        parent->right = node;
    }
}

// To delete node with given id
void B_Tree::delete_node_by_id(int moduleID)
{
    auto nodes = allnodes();
    for (int i = 0; i < nodes.size(); i++)
        if (nodes[i]->id == moduleID)
        {
            auto node = nodes[i];
            delete_node(nodes[i]);
            SAFE_DELETE(node);

            break;
        }
    nodes_N = allnodes().size();
}

int B_Tree::take_node_random()
{
    auto nodes = allnodes();
    int i = rand() % nodes.size();
    int ModuleId = nodes[i]->id;
    auto node = nodes[i];
    delete_node(nodes[i]);
    SAFE_DELETE(node);
    nodes_N = allnodes().size();
    return ModuleId;
}

bool B_Tree::take_node(int mod_id)
{
    auto nodes = allnodes();
    for (int i = 0; i < nodes.size(); i++)
    {
        if (nodes[i]->id == mod_id) {
            auto node = nodes[i];
            delete_node(nodes[i]);
            SAFE_DELETE(node);
            nodes_N = allnodes().size();
            return true;
        }
    }

    return false;
}

void B_Tree::delete_node(Node* node) {
    Node* child = nullptr;	// pull which child
    Node* subchild = nullptr;   // child's subtree
    Node* subparent = nullptr;

    if (!node->isleaf()) {
        bool left = rand_bool();			// choose a child to pull up
        if (node->left == nullptr) left = false;
        if (node->right == nullptr) left = true;

        //add_changed_nodes(node.left);
        //add_changed_nodes(node.right);

        if (left) {
            child = node->left;			// child will never be NIL
            if (node->right != nullptr)
            {
                subchild = child->right;
                subparent = node->right;
                node->right->parent = child;
                child->right = node->right;	// abut with node's another child
            }
        }
        else {
            child = node->right;
            if (node->left != nullptr)
            {
                subchild = child->left;
                subparent = node->left;
                node->left->parent = child;
                child->left = node->left;
            }
        }
        //add_changed_nodes(subchild);
        child->parent = node->parent;
    }

    if (node->parent == nullptr) {			// root
  //    changed_root = nodes_root;
        nodes_root = child;
    }
    else {					// let parent connect to child
     //add_changed_nodes(node.parent);
        if (node == node->parent->left)
            node->parent->left = child;
        else
            node->parent->right = child;
    }

    // place subtree
    if (subchild != nullptr) {
        Node* sc = subchild;
        assert(subparent != nullptr);

        while (1) {
            Node* p = subparent;

            if (p->left == nullptr || p->right == nullptr) {
                //add_changed_nodes(p.id);

                sc->parent = p;
                if (p->left == nullptr) p->left = sc;
                else p->right = sc;
                break;
            }
            else {
                subparent = (rand_bool() ? p->left : p->right);
            }
        }
    }
}


bool B_Tree::delete_node2(Node* node, DIR pull) {
    DIR npull = !pull;

    Node* p = node->parent;
    Node* n = node;
    Node* c = child(n, pull);
    Node* cn = child(n, npull);

    assert(n != nodes_root); // not root;

    DIR p2c = (p->left == n ? LEFT : RIGHT);

    if (c == nullptr) {
        wire_nodes(p, cn, p2c);
        return (cn != nullptr);   // folding
    }
    else {
        wire_nodes(p, c, p2c);
    }

    while (c != nullptr) {
        Node* k = child(c, npull);
        wire_nodes(c, cn, npull);
        cn = k;
        n = c;
        c = child(c, pull);
    }

    if (cn != nullptr) {
        wire_nodes(n, cn, pull);
        return true;
    }
    else
        return false;
}

/*
   Insert node into parent's left or right subtree according by "edge".
   Push node into parent's subtree in  "push" direction.

   if "fold" is true, then fold the leaf.
   (for the boundary condition of "delete" operation)

   delete <==> insert are permutating operations that can be recoved.
*/

void B_Tree::insert_node2(Node* parent, Node* node,
    DIR edge, DIR push, bool fold) {
    DIR npush = !push;
    Node* p = parent;
    Node* n = node;
    Node* c = child(p, edge);

    wire_nodes(p, n, edge);
    wire_nodes(n, c, push);

    while (c != nullptr) {
        wire_nodes(n, child(c, npush), npush);
        n = c;
        c = child(c, push);
    }
    wire_nodes(n, nullptr, npush);

    if (fold) {
        wire_nodes(n->parent, nullptr, push);
        wire_nodes(n->parent, n, npush);
    }
}

void _copyTree(Node* parent, Node* node, bool is_left, vector<Node>& tree)
{
    if (node == nullptr)
    {
        (is_left ? parent->left : parent->right) = nullptr;
        return;
    }

    tree.push_back(*node);
    //cout<<node->id<<":"<<(node->left==nullptr ? -1 : node->left->id)<<" "<<(node->parent==nullptr ? -1 : node->parent->id)<<" "<<(node->right==nullptr ? -1 : node->right->id)<<endl;
    Node& currnode = tree.back();
    currnode.parent = parent;

    if (parent != nullptr)
        (is_left ? parent->left : parent->right) = &tree.back();

    _copyTree(&currnode, node->left, true, tree);
    _copyTree(&currnode, node->right, false, tree);

    //cout<<currnode.id<<":"<<(currnode.left==nullptr ? -1 : currnode.left->id)
    //        <<" "<<(currnode.parent==nullptr ? -1 : currnode.parent->id)<<" "<<(currnode.right==nullptr ? -1 : currnode.right->id)<<endl;
}
void B_Tree::copyTree(vector<Node>& tree)
{
    tree.reserve(vector_max_size);
    tree.clear();
    _copyTree(nullptr, nodes_root, true, tree);
}

void _traverse(Node* node, vector<Node*>& nodes)
{
    if (node == nullptr)
        return;

    nodes.push_back(node);
    _traverse(node->left, nodes);
    _traverse(node->right, nodes);
}

// To retrieve all nodes from a B* tree
vector<Node*> B_Tree::allnodes()
{
    vector<Node*> r;
    _traverse(nodes_root, r);
    return r;
}

// To delete all nodes from a B* tree
void B_Tree::destroy() {
    auto nodes = allnodes();
    for (int i = 0; i < nodes.size(); i++)
    {
        SAFE_DELETE(nodes[i]);
    }
    nodes.clear();
}

