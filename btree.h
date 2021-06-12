// Project: B*-trees floorplanning
// Advisor: Yao-Wen Chang  <ywchang@cis.nctu.edu.tw>
// Authors: Jer-Ming Hsu   <barz@cis.nctu.edu.tw>
// 	    Hsun-Cheng Lee <gis88526@cis.nctu.edu.tw>
// Sponsor: Arcadia Inc.
// Date:    7/19/2000 ~

//---------------------------------------------------------------------------
#ifndef btreeH
#define btreeH
//---------------------------------------------------------------------------
#include <vector>
#include <cassert>
#include "fplan.h"
#include <stdexcept>

#define SAFE_DELETE(p) { \
    if(p==NULL) { \
        std::string str="\ndelete on nullptr: "; \
        printf("%s",str.c_str()); \
        throw std::runtime_error("DELETE ON NULLPTR"); \
    } \
    p=NULL; \
}

//---------------------------------------------------------------------------
const int NIL = -1;
typedef bool DIR;
const bool LEFT=0, RIGHT=1;
const int vector_max_size = 10000;

//*********** Structure for constriant **********//
//*** Variant Constraint ***//
struct VARIANT
{
    int mod;
    vector<float> ratios;
};

//*** Minimum Separation Constraint ***//
struct MINIMUM_SEPERATION
{
    int mod;
    int dis;
};

//*** Fixed Boundary Constraint ***//
struct FIXED_BOUNDARY
{
    int mod;
    bool rotate;
};

struct Contour{
  int front,back;
};

class B_Tree : public FPlan{
  public:
    B_Tree(float calpha=1) :FPlan(calpha) { prev_tree = nullptr; }
    virtual void init();
    virtual void packing();
    virtual void perturb();
    virtual void keep_sol();
    virtual void keep_best();    
    virtual void recover();
    virtual void recover_best();

    void setNodes();
    vector<Node> getNodes();
    void initWithNodeIndices(vector<int> nodes);
    void initWithOutNode();
    void insertNodeById(Node* parent, int moduleId);
    int take_node_random();
    bool take_node(int mod_id);
    void delete_node_by_id(int moduleID);
    void swap_node_by_id (int m1, int m2);
    Node* find_node_by_id(int moduleID);
    Node* find_node_random();
    void show_tree();  
    int getNodesCount();
    void copyTree(vector<Node> &tree);
    vector<Node*> allnodes();
    void destroy();

    int contour_root;
    vector<Contour> contour;
    vector<VARIANT> variants;
    vector<MINIMUM_SEPERATION> min_seps;
    vector<FIXED_BOUNDARY>  fixed_bndries;

    // vector<Node> nodes;   

    // debuging

  protected:
    
    void place_module(Node* mod,Node* abut,bool is_left=true);
    void clear();
    
    // Auxilary function
    void wire_nodes(Node* parent,Node* child,DIR edge);
    Node* child(Node*,DIR d);
    void add_changed_nodes(int n);
  
    //*** Simulated Annealing permutating operation ***//
    void swap_node(Node *n1, Node *n2);

    void insert_node(Node *parent,Node *node);
    void delete_node(Node *node);
    
    bool delete_node2(Node *node,DIR pull);
	  void insert_node2(Node *parent,Node *node,DIR edge=LEFT,DIR push=LEFT,bool fold=false);

    void calcTotalArea();

  private:        
    struct Solution{
      Node* nodes_root;
      vector<Node> nodes;
      double cost;
      Solution() { cost = 1; }
      void clear() { cost = 1, nodes.clear(); }
    };
  
    void get_solution(Solution &sol);
    virtual void recover(Solution &sol);

    Solution best_sol, last_sol;
    // for partial recover
    vector<Node> changed_nodes;
    Node* changed_root;    
    vector<Node>* prev_tree;
    Node* prev_node;
};

//---------------------------------------------------------------------------
#endif
