#ifndef qbtreeH
#define qbtreeH

#include <vector>
#include <fstream>
#include <iostream>
#include <cstring>


#include "btree.h"

#include "sa.h"

#define  SAFE_DELETE(p) {delete p; p= NULL;}

using namespace std;

// Specifies the left, right, bottom and top of a module
struct RECT
{
    long left, right, bottom, top;
};

// Declaration of properties of a QB-tree node
struct QBTreeNode
{
    RECT boundRect;
    int parent,tl,tr,bl,br,ppm;
    B_Tree *btree;
    bool isleaf(){return tl == NIL && tr == NIL && bl == NIL && br == NIL && ppm == NIL && btree==nullptr && parent!=NIL;}    
};

//********** Structure Declaration for each constraint **********//

// Maximum separation constriant
struct MAXIMUM_SEPERATION
{
    int mod1;
    int mod2;
    long dis;
};

// Symmetry Constriant
struct SYMMETRY
{
    int mod1;
    int mod2;
};

// Range Constriant
struct RANGE
{
    int mod;
    char boundary[10];
    int range;
};

// Close to boundary Constriant
struct CLOSE_TO_BOUNDARY
{
    int mod;
    int dis;
};

// Defining all ten general geometrical constraint 
struct Constraint
{
    vector<MAXIMUM_SEPERATION>  max_sep;
    vector<MINIMUM_SEPERATION>  min_sep;
    vector<SYMMETRY>            symmetry;
    vector<int>                 proximity;
    vector<RANGE>               range;
    vector<CLOSE_TO_BOUNDARY>   clto_boundary;
    vector<int>                 boundary;
    vector<FIXED_BOUNDARY>      fixed_boundary;
    vector<VARIANT>             variant;
};

// To store the best solution
struct Solution
{
    vector<QBTreeNode>      qbnodes;
    double                  cost;
};

// QB-tree Class
class QBtree
{
private:
public:
  //  const int               NIL = -1;
    vector<RECT>            rects;
    vector<QBTreeNode>      qbnodes;
    vector<QBTreeNode*>     candidates;
    Modules_Info            modules_info;  
    vector<Module>          modules;
    int                     modules_N;
    vector<vector<int>>     connection;
    map<string,int>         net_table;
    Module                  root_module;
    double                  WireLength;
    double                  Area;
    Nets                    network;
    Constraint              constraints;
    int                     leaf_num;
    float                   alpha;
    char                    filename[40];
    int                     times;
    int                     local;
    float                   term_temp;
    vector<B_Tree*>         b_trees;
    char                    line[100],t1[40],t2[40];
    ifstream                fs;
    double                  TotalArea;
    Solution                bestSolution;
    Solution                lastSolution;
    double                  normal_cost,cost;

    void                    init(float alpha, char* filename, int times, int local, float term_temp);
    void                    read_module_info();
    void                    read_dimension(Module &mod);
    void                    read_IO_list(Module &mod,bool parent);
    void                    read_network();
    void                    create_network();
    void                    makeQBTreeRoot(const vector<RECT>& rects);
    void                    readPreplacedModules();
    void                    readConstraint(char* file);
    long                    getC(long y, const vector<RECT>& rects, int except_id);
    void                    qSplit(int parent, const vector<RECT>& rects);
    void                    showQBTree();
    void                    constructQBTree();
    void                    constructBTree(B_Tree &fp, vector<int> inds);
    void                    initBTree(B_Tree &fp);
    void                    perturb();
    QBTreeNode*             find_leaf_random();
    QBTreeNode*             find_big_leaf();
    QBTreeNode*             find_qbnode_with_btree(B_Tree* _btree);
    int                     find_btree(int mod);
    int                     find_leaf_with_module(int mod_id);
    int                     find_mod_id_with_module_name(char* module_name);
    void                    perturbation();
    void                    normalize_cost(int time);
    void                    move_node_to_quad_leaf();
    void                    move_node_to_b();
    void                    swap_node();
    void                    packing();
    double                  getCost();
    void                    show_module();
    int                     getModuleIDWithModuleName(char* moduleName);
    bool                    constraintChecking();
    bool                    maximum_seperation();
    bool                    minimum_seperation();
    bool                    range_cons();
    bool                    close_to_boundary();
    bool                    boundary();
    bool                    fixed_boundary();
    bool                    proximity();
    bool                    candidateGeneration(int constraint, int mod_id, int index);
    void                    Op3(int index, int op_index, int mod_id);
    void                    Op2(int op_mod_id, int mod_id);
    void                    Op1(int index, int op_index, int mod_id);
    double                  calcWireLength();
    void                    scaleIOPad();
    double                  calcNormalizeArea();
    double                  calcOutOfBoundArea();
    double                  calcViolationCost();
    char*                   tail(char *str);
    void                    keep_sol(Solution &sol);
    void                    recover(Solution &sol);
    void                    cost_evaluation();
    double                  SA_Floorplan(int k, int local, float term_T);
    double                  std_var(vector<double> &chain);
    void                    outPutResult(char *filepath);
    void                    copyTree(vector<Node> *tree_o, vector<Node> *tree);
    bool                    is_max_sep_module(int mid);
};

#endif