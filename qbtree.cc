#include <stack>
#include <algorithm>
#include "qbtree.h"
#include <iostream>
#include <climits>
#include <algorithm>    // std::min
#include <chrono>
#include <cmath>

using namespace std;

//********** Initialization of QB-tree **********//
void QBtree::init(float alpha, char* filename, int times, int local, float term_temp)
{
    QBtree::alpha = alpha;
    strcpy(QBtree::filename, filename);
    QBtree::times = times;
    QBtree::local = local;
    QBtree::term_temp = term_temp;

    char constraint_file[80];
    strcpy(constraint_file, filename);
    strcat(constraint_file, "_constraint");

    // Read Module info
    read_module_info();

    // Construct QB-tree.
    constructQBTree();

    // Read Constraint.
    readConstraint(constraint_file);

    // standard_cost.
    normalize_cost(10);
    //normalize_cost(1);
}

//********** Get the module dimension, IO list and create network **********//
void QBtree::read_module_info()
{
    fs.open(filename);
    if (fs.fail())
        error("unable to open file: %s", filename);

    bool final = false;
    Module dummy_mod;

    for (int i = 0; !fs.eof(); i++) {
        // modules
        modules.push_back(dummy_mod);	// new module
        Module& mod = modules.back();
        mod.id = i;
        mod.pins.clear();

        fs >> t1 >> t2;
        tail(t2);			// remove ";"
        strcpy(mod.name, t2);

        fs >> t1 >> t2;
        if (!strcmp(t2, "PARENT;"))
            final = true;

        // dimension
        read_dimension(mod);
        read_IO_list(mod, final);

        // network
        if (final) {
            read_network();
            break;
        }
    }

    root_module = modules.back();
    modules.pop_back();		// exclude the parent module
    modules_N = modules.size();
    modules_info.resize(modules_N);
    modules.resize(modules_N);

    create_network();
    fs.close();
}

//********** Read the dimension points of each module from input file **********//
void QBtree::read_dimension(Module& mod)
{
    fs >> t1;
    int min_x = INT_MAX, min_y = INT_MAX, max_x = INT_MIN, max_y = INT_MIN;
    int tx, ty;
    for (int i = 0; i < 4; i++) {
        fs >> tx >> ty;
        min_x = min(min_x, tx); max_x = max(max_x, tx);
        min_y = min(min_y, ty); max_y = max(max_y, ty);
    }

    mod.x = min_x;
    mod.y = min_y;
    mod.width = max_x - min_x;
    mod.height = max_y - min_y;
    mod.area = mod.width * mod.height;
    fs >> t1 >> t2;
}

//********** Read the pins for each module from input file**********//
void QBtree::read_IO_list(Module& mod, bool parent = false)
{
    // IO list
    while (!fs.eof()) {
        Pin p;
        fs.getline(line, 100);
        if (strlen(line) == 0) continue;
        sscanf(line, "%s %*s %d %d", t1, &p.x, &p.y);

        if (!strcmp(t1, "ENDIOLIST;"))
            break;

        if (parent) { // IO pad is network
            // make unique net id
            net_table.insert(make_pair(string(t1), net_table.size()));
            p.net = net_table[t1];
        }

        p.mod = mod.id;
        p.x -= mod.x;  p.y -= mod.y;	// shift to origin

        mod.pins.push_back(p);
    }
    fs.getline(line, 100);
}

//********** Create network **********//
void QBtree::create_network() {
    network.resize(net_table.size());

    for (int i = 0; i < modules_N; i++) {
        for (int j = 0; j < modules[i].pins.size(); j++) {
            Pin& p = modules[i].pins[j];
            network[p.net].push_back(&p);
        }
    }

    for (int j = 0; j < root_module.pins.size(); j++) {
        Pin& p = root_module.pins[j];
        network[p.net].push_back(&p);
    }

    connection.resize(modules_N + 1);
    for (int i = 0; i < modules_N + 1; i++) {
        connection[i].resize(modules_N + 1);
        fill(connection[i].begin(), connection[i].end(), 0);
    }

    for (int i = 0; i < network.size(); i++) {
        for (int j = 0; j < network[i].size() - 1; j++) {
            int p = network[i][j]->mod;
            for (int k = j + 1; k < network[i].size(); k++) {
                int q = network[i][k]->mod;
                connection[p][q]++;
                connection[q][p]++;
            }
        }
    }
}

//********** Retrieve Newtork Information from input file **********//
void QBtree::read_network() {
    while (!fs.eof()) {
        bool end = false;
        int n = 0;
        fs >> t1 >> t2;
        if (!strcmp(t1, "ENDNETWORK;"))
            break;
        // determine which module interconnection by name
        int m_id;
        for (m_id = 0; m_id < modules.size(); m_id++)
            if (!strcmp(modules[m_id].name, t2))
                break;
        if (m_id == modules.size())
            error("can't find suitable module name!");

        while (!fs.eof()) {
            fs >> t1;
            if (t1[strlen(t1) - 1] == ';') {
                tail(t1);
                end = true;
            }

            // make unique net id
            net_table.insert(make_pair(string(t1), net_table.size()));
            modules[m_id].pins[n++].net = net_table[t1];
            if (end) break;
        }
    }
}

//********** Retrieve constraint data from constraint file **********//
void QBtree::readConstraint(char* file)
{
    char* token;
    fs.open(file);
    if (fs.fail())
        error("unable to open file: %s", file);

    while (!fs.eof())
    {
        fs >> t1;
        if (strcmp(t1, "SYMMETRY") == 0)
            //SYMMETRY CONSTRAINT.
        {
            while (!fs.eof())
            {
                SYMMETRY sym;
                sym.mod1 = NIL;
                sym.mod2 = NIL;
                fs >> t1;
                if (strcmp(t1, "END") == 0)
                    break;
                if (t1[strlen(t1) - 1] == ';')
                {
                    tail(t1);
                }
                if (t1[0] == '[')
                {
                    token = strtok(t1, "[,]");
                    while (token != NULL)
                    {
                        if (sym.mod1 == NIL)
                        {
                            sym.mod1 = find_mod_id_with_module_name(token);
                        }
                        else
                        {
                            sym.mod2 = find_mod_id_with_module_name(token);
                            constraints.symmetry.push_back(sym);
                        }
                        token = strtok(NULL, "[,]");
                    }
                }
                else
                {
                    token = strtok(t1, ",");
                    while (token != NULL)
                    {
                        if (sym.mod1 == NIL)
                        {
                            sym.mod1 = find_mod_id_with_module_name(token);
                            sym.mod2 = NIL;
                            constraints.symmetry.push_back(sym);
                        }
                        token = strtok(NULL, ",");
                    }
                }
            }
            //cout<<"symmetry constraint"<<endl;
            // for(int i = 0; i < constraints.symmetry.size(); i++)
            // {
            // cout<<"mod1 : "<<constraints.symmetry[i].mod1;
            //  cout<<" mod2 : "<<constraints.symmetry[i].mod2<<endl;
            //  }
        }

        if (strcmp(t1, "PROXIMITY") == 0)
            // PROXIMITY CONSTRAINT
        {
            while (!fs.eof())
            {
                fs >> t1;
                if (strcmp(t1, "END") == 0)
                    break;
                token = strtok(t1, "[,];");
                while (token != NULL)
                {
                    constraints.proximity.push_back(find_mod_id_with_module_name(token));
                    token = strtok(NULL, "[,];");
                }
            }
            // cout<<"proximity constraint"<<endl;
            // for(int i = 0; i < constraints.proximity.size(); i++)
            // {
            //     cout<<"mod : "<<constraints.proximity[i]<<endl;
            // }
        }

        if (strcmp(t1, "MINIMUM_SEPARATION") == 0)
            // MINIMUM_SEPARATION CONSTRAINT
        {
            while (!fs.eof())
            {
                MINIMUM_SEPERATION min;
                min.mod = NIL;
                min.dis = NIL;
                fs >> t1;
                if (strcmp(t1, "END") == 0)
                    break;
                fs >> t2;
                tail(t2);
                min.mod = find_mod_id_with_module_name(t1);
                min.dis = atoi(t2);
                constraints.min_sep.push_back(min);
            }

            // cout<<"minimum seperation"<<endl;
            //  for(int i = 0; i < constraints.min_sep.size(); i++)
            //  {
            //     cout<<"mod : "<<constraints.min_sep[i].mod;
            //     cout<<" dis : "<<constraints.min_sep[i].dis<<endl;
            //  }
        }

        if (strcmp(t1, "MAXIMUM_SEPARATION") == 0)
            // MAXIMUM_SEPARATION CONSTRAINT
        {
            while (!fs.eof())
            {
                MAXIMUM_SEPERATION max;
                max.mod1 = NIL;
                max.mod2 = NIL;
                max.dis = NIL;
                fs >> t1;
                if (strcmp(t1, "END") == 0)
                    break;
                fs >> t2;
                tail(t2);
                token = strtok(t1, "[,] ");
                while (token != NULL)
                {
                    if (max.mod1 == NIL)
                    {
                        max.mod1 = find_mod_id_with_module_name(token);
                    }
                    else
                    {
                        max.mod2 = find_mod_id_with_module_name(token);
                    }
                    token = strtok(NULL, "[,]");
                }
                max.dis = atoi(t2);
                constraints.max_sep.push_back(max);
            }

            // cout<<"maximum seperation"<<endl;
            // for(int i = 0; i < constraints.max_sep.size(); i++)
            // {
            //     cout<<"mod1 : "<<constraints.max_sep[i].mod1;
            //     cout<<" mod2 : "<<constraints.max_sep[i].mod2;
            //     cout<<" dis : "<<constraints.max_sep[i].dis<<endl;
            // }
        }

        if (strcmp(t1, "RANGE") == 0)
            // RANGE CONSTRAINT
        {
            while (!fs.eof())
            {
                RANGE range;
                range.mod = NIL;
                range.range = NIL;
                fs >> t1;
                if (strcmp(t1, "END") == 0)
                    break;
                range.mod = find_mod_id_with_module_name(t1);
                fs >> t1;
                strcpy(range.boundary, t1);
                fs >> t1;
                tail(t1);
                range.range = atoi(t1);
                constraints.range.push_back(range);
            }

            // cout<<"range"<<endl;
            // for(int i = 0; i < constraints.range.size(); i++)
            // {
            //     cout<<"mod : "<<constraints.range[i].mod;
            //     cout<<" boundary : "<<constraints.range[i].boundary;
            //     cout<<" range : "<<constraints.range[i].range<<endl;
            // }
        }

        if (strcmp(t1, "CLOSE_TO_BOUNDARY") == 0)
            // CLOSE_TO_BOUNDARY CONSTRAINT
        {
            while (!fs.eof())
            {
                CLOSE_TO_BOUNDARY cl;
                cl.mod = NIL;
                cl.dis = NIL;
                fs >> t1;
                if (strcmp(t1, "END") == 0)
                    break;
                cl.mod = find_mod_id_with_module_name(t1);
                fs >> t1;
                cl.dis = atoi(t1);
                constraints.clto_boundary.push_back(cl);
            }

            // cout<<"CLOSE_TO_BOUNDARY"<<endl;
            // for(int i = 0; i < constraints.clto_boundary.size(); i++)
            // {
            //     cout<<"mod : "<<constraints.clto_boundary[i].mod;
            //     cout<<" dis : "<<constraints.clto_boundary[i].dis<<endl;
            // }
        }

        if (strcmp(t1, "BOUNDARY") == 0)
            // BOUNDARY CONSTRAINT
        {
            while (!fs.eof())
            {
                fs >> t1;
                if (strcmp(t1, "END") == 0)
                    break;
                tail(t1);
                constraints.boundary.push_back(find_mod_id_with_module_name(t1));
            }
            // cout<<"BOUNDARY"<<endl;
            // for(int i = 0; i < constraints.boundary.size(); i++)
            // {
            //     cout<<"mod : "<<constraints.boundary[i]<<endl;
            // }
        }

        if (strcmp(t1, "FIXED_BOUNDARY") == 0)
        {
            //FIXED_BOUDNARY CONSTRAINT
            while (!fs.eof())
            {
                FIXED_BOUNDARY fb;
                fs >> t1;
                if (strcmp(t1, "END") == 0)
                    break;
                fs >> t2;
                tail(t2);
                if (strcmp(t2, "1") == 0) // rotate
                    fb.rotate = true;
                else
                    fb.rotate = false;
                fb.mod = find_mod_id_with_module_name(t1);
                constraints.fixed_boundary.push_back(fb);
            }
        }

        if (strcmp(t1, "VARIANT") == 0)
        {
            //VARIANT CONSTRAINT
            while (!fs.eof())
            {
                fs >> t1;
                if (strcmp(t1, "END") == 0)
                    break;
                VARIANT v;
                v.mod = find_mod_id_with_module_name(t1);
                fs >> t2;
                token = strtok(t2, "[,];");
                while (token != NULL)
                {
                    v.ratios.push_back((float)atof(token));
                    token = strtok(NULL, "[,];");
                }
                constraints.variant.push_back(v);
            }
        }
    }
}

char* QBtree::tail(char* str)
{
    str[strlen(str) - 1] = 0;
    return str;
}

//********** Retrieves module with given id **********//
int QBtree::find_mod_id_with_module_name(char* module_name)
{
    for (int i = 0; i < modules.size(); i++)
    {
        if (strcmp(modules[i].name, module_name) == 0)
            return modules[i].id;
    }
    return NIL;
}

//********** Read Preplaced module **********//
void QBtree::readPreplacedModules()
{
    //RECT ppm_1;
    RECT ppm_2;
    int w, h;
    w = modules[0].width;
    h = modules[0].height;
    for (int i = 0; i < modules.size(); i++)
    {
        if (w < modules[i].width)
            w = modules[i].width;
        if (h < modules[i].height)
            h = modules[i].height;
    }
    // ppm_1's position is modified according to input file.
    //    ppm_1.left = root_module.width - 1832;
    //    ppm_1.right = root_module.width + 1832;
    //    ppm_1.bottom = root_module.height /2 + 1832;
    //    ppm_1.top = root_module.height + 1354;
    //rects.push_back(ppm_1);

    // ppm_2's position is left-bottom cornor.
    ppm_2.left = 0;
    ppm_2.right = w;
    ppm_2.bottom = 0;
    ppm_2.top = h;
    rects.push_back(ppm_2);

}

//********** Make root node of QB-tree **********//
void QBtree::makeQBTreeRoot(const vector<RECT>& rects)
{
    double average_length = ceil(sqrt(TotalArea));
    /*
    if (rects.empty())
        return;

    long top = rects[0].top;
    long bottom = rects[0].bottom;
    long left = rects[0].left;
    long right = rects[0].right;

    for (auto it = rects.begin(); it != rects.end(); it++)
    {
        if (top < it->top)
            top = it->top;
        if (bottom > it->bottom)
            bottom = it->bottom;
        if (left > it->left)
            left = it->left;
        if (right < it->right)
            right = it->right;
    }
    */

    //********** Root node of QB-tree **********//
    QBTreeNode qbnode;

    //qbnode.boundRect.left = left;
    //qbnode.boundRect.bottom = bottom;
    //qbnode.boundRect.right = right;
    //qbnode.boundRect.top = top;
    qbnode.boundRect.left = 0;
    qbnode.boundRect.bottom = 0;
    qbnode.boundRect.right = average_length * 3 / 2;
    qbnode.boundRect.top = average_length * 3 / 2;
    qbnode.parent = qbnode.tl = qbnode.tr = qbnode.bl = qbnode.br = qbnode.ppm = NIL;
    qbnode.btree = nullptr;
    qbnodes.push_back(qbnode);
}

//********** Calculate the cost of Partition line. c = alpha*N + beta*SUM(L) + gamma*D **********//
long QBtree::getC(long y, const vector<RECT>& rects, int except_id)
{
    const int alpha = 1;
    const int beta = 1;
    const int gamma = 1;

    long crossing = 0;
    long sumL = 0;
    long Dt_y = LONG_MAX;
    long Db_y = LONG_MIN;
    long D = 0;

    for (unsigned int i = 0; i < rects.size(); i++)
    {
        if (i == (unsigned int)except_id)
            continue;
        // N, L
        if (y > rects[i].bottom && y < rects[i].top)
        {
            crossing++;
            long lb = y - rects[i].bottom;
            long lt = rects[i].top - y;
            sumL += abs(lt - lb);
        }

        // D
        if (y < rects[i].bottom && Dt_y > rects[i].bottom)
            Dt_y = rects[i].bottom;
        if (y > rects[i].top && Db_y < rects[i].top)
            Db_y = rects[i].top;
    }

    if (Dt_y == LONG_MAX || Db_y == LONG_MIN)
        D = 0;
    else
        D = abs(Dt_y - Db_y);

    return alpha * crossing + beta * sumL + gamma * D;
}

//********** Construct Quad-tree **********//
void QBtree::qSplit(int parent, const vector<RECT>& rects)
{
    long cost = LONG_MAX;
    long y = 0, x = 0;
    RECT ppm;
    for (unsigned int i = 0; i < rects.size(); i++)
    {
        if (rects[i].bottom >= qbnodes[parent].boundRect.bottom &&
            rects[i].top <= qbnodes[parent].boundRect.top &&
            rects[i].left >= qbnodes[parent].boundRect.left &&
            rects[i].right <= qbnodes[parent].boundRect.right)
        {
            // if preplaced module is leaf node
            if (rects[i].bottom == qbnodes[parent].boundRect.bottom &&
                rects[i].top == qbnodes[parent].boundRect.top &&
                rects[i].left == qbnodes[parent].boundRect.left &&
                rects[i].right == qbnodes[parent].boundRect.right)
            {
                qbnodes[parent].ppm = i;
                qbnodes[parent].tl = qbnodes[parent].tr = qbnodes[parent].bl = qbnodes[parent].br = NIL;
                return;
            }
            // ith preplaced module's bottom line could be a partition line.
            else if (rects[i].top == qbnodes[parent].boundRect.top)
            {
                long c = getC(rects[i].bottom, rects, i);
                if (cost > c)
                {
                    cost = c;
                    y = rects[i].bottom;
                    if (rects[i].left != qbnodes[parent].boundRect.left)
                    {
                        x = rects[i].left;
                    }
                    else if (rects[i].right != qbnodes[parent].boundRect.right)
                    {
                        x = rects[i].right;
                    }
                    else
                    {
                        x = 0;
                    }
                }
            }
            // ith preplaced module's top line could be a partition line.
            else if (rects[i].bottom == qbnodes[parent].boundRect.bottom)
            {
                long c = getC(rects[i].top, rects, i);
                if (cost > c)
                {
                    cost = c;
                    y = rects[i].top;
                    if (rects[i].left != qbnodes[parent].boundRect.left)
                    {
                        x = rects[i].left;
                    }
                    else if (rects[i].right != qbnodes[parent].boundRect.right)
                    {
                        x = rects[i].right;
                    }
                    else
                    {
                        x = 0;
                    }
                }
            }
        }
        else
        {
            continue;
        }
    }

    if (y == 0 || x == 0)
    {

        leaf_num++;
        return;
    }

    // Generate new Quad-tree nodes
    QBTreeNode qbnode1;
    qbnode1.boundRect.left = qbnodes[parent].boundRect.left;
    qbnode1.boundRect.right = x;
    qbnode1.boundRect.top = qbnodes[parent].boundRect.top;
    qbnode1.boundRect.bottom = y;
    qbnode1.tl = qbnode1.tr = qbnode1.bl = qbnode1.br = qbnode1.ppm = NIL;
    qbnode1.parent = parent;
    qbnode1.btree = nullptr;
    qbnodes.push_back(qbnode1);
    qbnodes[parent].tl = qbnodes.size() - 1;

    //Recursive function
    qSplit(qbnodes[parent].tl, rects);

    QBTreeNode qbnode2;
    qbnode2.boundRect.left = x;
    qbnode2.boundRect.right = qbnodes[parent].boundRect.right;
    qbnode2.boundRect.top = qbnodes[parent].boundRect.top;
    qbnode2.boundRect.bottom = y;
    qbnode2.tl = qbnode2.tr = qbnode2.bl = qbnode2.br = qbnode2.ppm = NIL;
    qbnode2.parent = parent;
    qbnode2.btree = nullptr;
    qbnodes.push_back(qbnode2);
    qbnodes[parent].tr = qbnodes.size() - 1;

    qSplit(qbnodes[parent].tr, rects);

    QBTreeNode qbnode3;
    qbnode3.boundRect.left = qbnodes[parent].boundRect.left;
    qbnode3.boundRect.right = x;
    qbnode3.boundRect.top = y;
    qbnode3.boundRect.bottom = qbnodes[parent].boundRect.bottom;
    qbnode3.tl = qbnode3.tr = qbnode3.bl = qbnode3.br = qbnode3.ppm = NIL;
    qbnode3.parent = parent;
    qbnode3.btree = nullptr;
    qbnodes.push_back(qbnode3);
    qbnodes[parent].bl = qbnodes.size() - 1;
    qSplit(qbnodes[parent].bl, rects);

    QBTreeNode qbnode4;
    qbnode4.boundRect.left = x;
    qbnode4.boundRect.right = qbnodes[parent].boundRect.right;
    qbnode4.boundRect.top = y;
    qbnode4.boundRect.bottom = qbnodes[parent].boundRect.bottom;
    qbnode4.tl = qbnode4.tr = qbnode4.bl = qbnode4.br = qbnode4.ppm = NIL;
    qbnode4.parent = parent;
    qbnode4.btree = nullptr;
    qbnodes.push_back(qbnode4);
    qbnodes[parent].br = qbnodes.size() - 1;
    qSplit(qbnodes[parent].br, rects);
}

//********** Display QB-tree **********//
void QBtree::showQBTree()
{
    cout << "------ QBTree -------" << endl;
    for (int i = 0; i < qbnodes.size(); i++)
    {
        if (qbnodes[i].isleaf())
        {
            cout << "No " << i << " tl:" << qbnodes[i].tl
                << " tr:" << qbnodes[i].tr << " bl:" << qbnodes[i].bl
                << " br:" << qbnodes[i].br << " parent:" << qbnodes[i].parent << " leaf" << endl;
        }
        else if (qbnodes[i].ppm != NIL)
        {
            cout << "No " << i << " tl:" << qbnodes[i].tl
                << " tr:" << qbnodes[i].tr << " bl:" << qbnodes[i].bl
                << " br:" << qbnodes[i].br << " parent:" << qbnodes[i].parent << " leaf(ppm):" << qbnodes[i].ppm << endl;
        }
        else if (qbnodes[i].btree != nullptr)
        {
            cout << "No " << i << " tl:" << qbnodes[i].tl
                << " tr:" << qbnodes[i].tr << " bl:" << qbnodes[i].bl
                << " br:" << qbnodes[i].br << " parent:" << qbnodes[i].parent << " leaf(b*-tree):" << endl;
            qbnodes[i].btree->show_tree();
        }
        else
        {
            cout << "No " << i << " tl:" << qbnodes[i].tl
                << " tr:" << qbnodes[i].tr << " bl:" << qbnodes[i].bl
                << " br:" << qbnodes[i].br << " parent:" << qbnodes[i].parent << endl;
        }
    }
}

//********** Construct QB-tree **********//
void QBtree::constructQBTree()
{
    // Create B*-tree using input file.
    B_Tree* bt = new B_Tree(alpha);
    initBTree(*bt);
    bt->init();

    // Calculate Module's total area.
    TotalArea = 0;
    for (int i = 0; i < modules.size(); i++)
    {
        TotalArea += modules[i].area;
    }

    // Read pre-placed modules.
    readPreplacedModules();

    for (int i = 0; i < rects.size(); i++)
    {
        TotalArea += (rects[i].right - rects[i].left) * (rects[i].top - rects[i].bottom);
    }

    // Make QB-tree's root.
    makeQBTreeRoot(rects);

    // Split Quad-tree.
    qSplit(0, rects);

    // Make one B*-tree with all modules and add to Quad-tree leaf.
    QBTreeNode* qnode = find_big_leaf();
    qnode->btree = bt;
    b_trees.push_back(qnode->btree);
}

QBTreeNode* QBtree::find_big_leaf()
{
    int index = 0;
    double w, h, bw, bh;
    w = qbnodes[1].boundRect.right - qbnodes[1].boundRect.left;
    h = qbnodes[1].boundRect.top - qbnodes[1].boundRect.bottom;
    for (int i = 1; i < qbnodes.size(); i++)
    {
        if (!qbnodes[i].isleaf())
            continue;

        bw = qbnodes[i].boundRect.right - qbnodes[i].boundRect.left;
        bh = qbnodes[i].boundRect.top - qbnodes[i].boundRect.bottom;

        if ((w * h) < (bw * bh))
        {
            index = i;
        }
    }

    if (index == 0)
    {
        return find_leaf_random();
    }
    else
    {
        return &qbnodes[index];
    }
}

//********** Returns module's id from the module's name **********//
int QBtree::getModuleIDWithModuleName(char* moduleName)
{
    for (int i = 0; i < modules.size(); i++)
    {
        if (strcmp(modules[i].name, moduleName) == 0)
            return modules[i].id;
    }
    return NIL;
}

//********** Initialize B*-tree and set variant, minimum_separation and fixed_boundary constraint **********//
void QBtree::initBTree(B_Tree& fp)
{
    fp.setModules(modules);
    fp.setRootModule(root_module);
    fp.create_network(net_table.size());

    fp.variants = constraints.variant;
    fp.min_seps = constraints.min_sep;
    fp.fixed_bndries = constraints.fixed_boundary;
}

//********** Leads to function that constructs B*-tree **********//
void QBtree::constructBTree(B_Tree& fp, vector<int> inds)
{
    initBTree(fp);
    //fp.show_modules();
    if (inds.size() == 0)
    {
        fp.init();
    }
    else
    {
        fp.initWithNodeIndices(inds);
    }
}

//********** Retrieve a random leaf node **********//
QBTreeNode* QBtree::find_leaf_random()
{
    int i, c = NIL;

    for (i = 0; i < qbnodes.size(); i++)
    {
        if (qbnodes[i].isleaf())
        {
            c++;
            break;
        }
    }

    if (c == NIL)
        return nullptr;
    do
    {
        i = rand() % qbnodes.size();
    } while (!qbnodes[i].isleaf());

    return &qbnodes[i];
}

//********** Retrieves a node in B*-tree **********//
QBTreeNode* QBtree::find_qbnode_with_btree(B_Tree* _btree)
{
    for (int i = 0; i < qbnodes.size(); i++)
        if (qbnodes[i].btree == _btree)
            return &qbnodes[i];
    return nullptr;
}

//********** QB-tree Perturbation **********//
void QBtree::perturb()
{
    bool movetoleaf;
    bool movetob;
    bool swap;
    // Randomly perturb flags setting.
    do {
        movetoleaf = rand_bool();
        movetob = rand_bool();
        swap = rand_bool();
    } while (movetoleaf == false && movetob == false && swap == false);



    if (b_trees.size() == 1) {
        movetoleaf = true;
    }

    // Move one node to new B*-tree in Quad-leaf.
    if (movetoleaf == true)
    {
        move_node_to_quad_leaf();
    }
    // Move one node to another B*-tree.
    if (movetob == true)
    {
        move_node_to_b();
    }
    // Swap node between two B*-trees.
    if (swap == true)
    {
        swap_node();
    }
    packing();
}

void QBtree::normalize_cost(int time)
{
    double cost_min = INT_MAX;
    bestSolution.cost = NIL;
    lastSolution.cost = NIL;
    normal_cost = 0;
    for (int i = 0; i < time; i++)
    {
        perturb();
        if (cost < cost_min)
        {
            keep_sol(bestSolution);
            keep_sol(lastSolution);
            cost_min = cost;
        }
        normal_cost += cost;
    }

    normal_cost = normal_cost / time;
    cout << "normalize Cost : " << normal_cost << " , min_cost : " << cost_min << endl;
}

void QBtree::perturbation()
{
    // perturb
    perturb();
    // constraint checking & candidate generation.
    if (!constraintChecking())
    {
        // if candidate generation failed, perturb again.
        perturbation();
    }
}

//********** Checks Constraints **********//
bool QBtree::constraintChecking()
{
    if (!constraints.max_sep.empty())
        // check MAXIMUM SEPERATION CONSTRAINT.
    {
        if (!maximum_seperation())
            return false;
        //showQBTree();
    }

    /* if (!constraints.min_sep.empty())
    // [2]. MINIMUM SEPERATION CONSTRAINT
    {
        if (!minimum_seperation())
            return false;
        //showQBTree();
    }
    */

    if (!constraints.range.empty())
        // RANGE CONSTRAINT
    {
        if (!range_cons())
        {
            return false;
        }
        // showQBTree();
    }

    if (!constraints.clto_boundary.empty())
        // CLOSE TO BOUNDARY CONSTRAINT
    {

        if (!close_to_boundary())
        {
            return false;
        }
    }

    //PROXIMITY CONSTRAINT
    if (!constraints.proximity.empty())
    {
        if (!proximity())
        {
            return false;
        }
    }

    //BOUNDARY CONSTRAINT
    if (!constraints.boundary.empty())
    {
        if (!boundary())
        {
            return false;
        }
    }

    // FIXED BOUNDARY CONSTRAINT
    if (!constraints.fixed_boundary.empty())
    {
        if (!fixed_boundary())
            return false;
    }
    /*
        for(int i = 0; i < qbnodes.size(); i++)
        {
            if(qbnodes[i].btree == nullptr)
                continue;
            if(qbnodes[i].btree->getWidth() > (qbnodes[i].boundRect.right - qbnodes[i].boundRect.left))
                return false;
            if(qbnodes[i].btree->getHeight() > (qbnodes[i].boundRect.top - qbnodes[i].boundRect.bottom))
                return false;
        }
    */
    return true;
}

//********** Fixed Boundary Constraint Handling **********//
bool QBtree::fixed_boundary()
{
    int mod, index, x, y, rx, ry, left, right, top, bottom;
    for (int i = 0; i < constraints.boundary.size(); i++)
    {
        mod = constraints.boundary[i];
        index = find_leaf_with_module(mod);
        QBTreeNode* qbnode = &qbnodes[index];
        // determine the boundary of module.
        x = modules_info[mod].x;
        y = modules_info[mod].y;
        rx = modules_info[mod].rx;
        ry = modules_info[mod].ry;
        left = x > rx ? rx : x;
        right = x > rx ? x : rx;
        top = y > ry ? y : ry;
        bottom = y > ry ? ry : y;
        if (left != qbnode->boundRect.left && right != qbnode->boundRect.right &&
            top != qbnode->boundRect.top && bottom != qbnode->boundRect.bottom)
        {
            int j;
            for (j = 0; j < qbnode->btree->allnodes().size(); j++)
            {
                x = modules_info[qbnode->btree->allnodes()[j]->id].x;
                y = modules_info[qbnode->btree->allnodes()[j]->id].y;
                rx = modules_info[qbnode->btree->allnodes()[j]->id].rx;
                ry = modules_info[qbnode->btree->allnodes()[j]->id].ry;
                left = x > rx ? rx : x;
                right = x > rx ? x : rx;
                top = y > ry ? y : ry;
                bottom = y > ry ? ry : y;
                if (left == qbnode->boundRect.left || right == qbnode->boundRect.right ||
                    top == qbnode->boundRect.top || bottom == qbnode->boundRect.bottom)
                {
                    //SUCCESS CANDIDATE GENERATION
                    qbnode->btree->swap_node_by_id(mod, qbnode->btree->allnodes()[j]->id);

                    break;
                }
            }
            if (j == qbnode->btree->allnodes().size())
            {
                //FAILED CANDIDATE GENERATION
                cout << "NOT SATISFIED" << endl;
                return false;
            }
        }
    }
    return true;
}

//********** Proximity Constraint Handling **********//
bool QBtree::proximity()
{
    int x, y, rx, ry, left, right, top, bottom, min_x, min_y, max_rx, max_ry;
    double dx, dy, drx, dry, max_d1, max_d2, distance;
    int standard_mod;
    min_x = min_y = INT_MAX;
    max_rx = max_ry = INT_MIN;
    max_d1 = max_d2 = 0;
    //determine boundary of each module given for proximity constraint
    for (int i = 0; i < constraints.proximity.size(); i++)
    {
        x = modules_info[constraints.proximity[i]].x;
        y = modules_info[constraints.proximity[i]].y;
        rx = modules_info[constraints.proximity[i]].rx;
        ry = modules_info[constraints.proximity[i]].ry;
        left = x > rx ? rx : x;
        right = x > rx ? x : rx;
        top = y > ry ? y : ry;
        bottom = y > ry ? ry : y;

        max_d1 += (right - left); //Gives the horizontal distance of all the modules
        max_d2 += (top - bottom); //Gives the vertical height of all the modules

        if (max_rx < right)
            max_rx = right;
        if (min_x > left)
        {
            min_x = left;
            standard_mod = constraints.proximity[i]; //Stores the module at the bottom-left among all in the group
        }
        if (max_ry < top)
            max_ry = top;
        if (min_y > bottom)
            min_y = bottom;
    }

    if ((max_rx - min_x) > max_d1 || (max_ry - min_y) > max_d2)
    {
        for (int j = 0; j < constraints.proximity.size(); j++)
        {
            x = modules_info[constraints.proximity[j]].x;
            y = modules_info[constraints.proximity[j]].y;
            rx = modules_info[constraints.proximity[j]].rx;
            ry = modules_info[constraints.proximity[j]].ry;
            left = x > rx ? rx : x;
            right = x > rx ? x : rx;
            top = y > ry ? y : ry;
            bottom = y > ry ? ry : y;
            if ((right - min_x) > max_d1 || (top - min_y) > max_d2)
            {
                Op1(find_leaf_with_module(constraints.proximity[j]), find_leaf_with_module(standard_mod), constraints.proximity[j]);
                //cout<<"Proximity "<<j<<" : candidate generation is succeed."<<endl;
            }
        }
    }
    else
    {
        //cout<<"proximity constraints are satisfied."<<endl;
    }

    return true;
}

//********** Boundary Constraint Handling **********//
bool QBtree::boundary()
{
    int mod, index, x, y, rx, ry, left, right, top, bottom;
    for (int i = 0; i < constraints.boundary.size(); i++)
    {
        mod = constraints.boundary[i];
        index = find_leaf_with_module(mod);
        QBTreeNode* qbnode = &qbnodes[index];
        x = modules_info[mod].x;
        y = modules_info[mod].y;
        rx = modules_info[mod].rx;
        ry = modules_info[mod].ry;
        //determine the boundary of module
        left = x > rx ? rx : x;
        right = x > rx ? x : rx;
        top = y > ry ? y : ry;
        bottom = y > ry ? ry : y;
        if (left == qbnode->boundRect.left || right == qbnode->boundRect.right ||
            top == qbnode->boundRect.top || bottom == qbnode->boundRect.bottom)
        {
        }
        else
        {
            int j;
            for (j = 0; j < qbnode->btree->allnodes().size(); j++)
            {
                x = modules_info[qbnode->btree->allnodes()[j]->id].x;
                y = modules_info[qbnode->btree->allnodes()[j]->id].y;
                rx = modules_info[qbnode->btree->allnodes()[j]->id].rx;
                ry = modules_info[qbnode->btree->allnodes()[j]->id].ry;
                left = x > rx ? rx : x;
                right = x > rx ? x : rx;
                top = y > ry ? y : ry;
                bottom = y > ry ? ry : y;
                if (left == qbnode->boundRect.left || right == qbnode->boundRect.right ||
                    top == qbnode->boundRect.top || bottom == qbnode->boundRect.bottom)
                {
                    //SUCCESS CANDIDATE GENERATION
                    qbnode->btree->swap_node_by_id(mod, qbnode->btree->allnodes()[j]->id);
                    break;
                }
            }
            if (j == qbnode->btree->allnodes().size())
            {
                //FAILED CANDIDATE GENERATION
                return false;
            }
        }
    }
    return true;
}

//********** Close_to_Boundary Constraint Handling **********//
bool QBtree::close_to_boundary()
{
    int mod, dis, b, x, y, rx, ry, l, r, t, b_index, cn;
    long w, h;
    for (int i = 0; i < constraints.clto_boundary.size(); i++)
    {
        cn = 0;
        mod = constraints.clto_boundary[i].mod;
        dis = constraints.clto_boundary[i].dis;
        b_index = find_btree(mod);
        x = modules_info[mod].x;
        y = modules_info[mod].y;
        rx = modules_info[mod].rx;
        ry = modules_info[mod].ry;
        //determine the boudnary of module
        l = x > rx ? rx : x;
        r = x > rx ? x : rx;
        t = y > ry ? y : ry;
        b = y > ry ? ry : y;
        QBTreeNode* qnode = find_qbnode_with_btree(b_trees[b_index]);
        w = qnode->boundRect.right - qnode->boundRect.left;
        h = qnode->boundRect.top - qnode->boundRect.bottom;

        if (w > (dis * 2) || h > (dis * 2))
        {
            //IF CONSTRIANT IS NOT SATISFIED
            if (b < (qnode->boundRect.top - dis) || l < (qnode->boundRect.right - dis) || r >(qnode->boundRect.left + dis) || t >(qnode->boundRect.bottom + dis))
            {
                auto nodes = qnode->btree->allnodes();
                for (int i = 0; i < nodes.size(); i++)
                {
                    x = modules_info[nodes[i]->id].x;
                    y = modules_info[nodes[i]->id].y;
                    rx = modules_info[nodes[i]->id].rx;
                    ry = modules_info[nodes[i]->id].ry;
                    l = x > rx ? rx : x;
                    r = x > rx ? x : rx;
                    t = y > ry ? y : ry;
                    b = y > ry ? ry : y;

                    if (b > (qnode->boundRect.top - dis) || l > (qnode->boundRect.right - dis) || r < (qnode->boundRect.left + dis) || t < (qnode->boundRect.bottom + dis))
                    {
                        qnode->btree->swap_node_by_id(nodes[i]->id, mod);
                        //cout<<nodes[i]->id<<" : "<<mod<<endl;
                        //cout<<"Close to boundary "<<i+1<<" : "<<"candidate generation is succeed."<<endl;
                        return true;
                    }
                }
            }
            else
            {
                //cout<<"Close to boundary "<<i+1<<" : "<<"is satisfied."<<endl;
            }
        }
        else
        {
            //CANDIDATE GENERATION
            for (int j = 0; j < qbnodes.size(); j++)
            {
                w = qbnodes[j].boundRect.right - qbnodes[j].boundRect.left;
                h = qbnodes[j].boundRect.top - qbnodes[j].boundRect.bottom;
                if (w > (dis * 2) || h > (dis * 2))
                {
                    Op1(find_leaf_with_module(mod), j, mod);
                    //cout<<"Close to boundary "<<i+1<<" : "<<"candidate generation is succeed."<<endl;
                    break;
                }
            }
        }
    }
}

//********** Range Constraint Handling **********//
bool QBtree::range_cons()
{
    int mod, range, b_inx, x, y, rx, ry, left, right, top, bottom, cn;
    for (int i = 0; i < constraints.range.size(); i++)
    {
        //GET THE MODULE INFORMATION
        mod = constraints.range[i].mod;
        //GET THE RANGE VALUE
        range = constraints.range[i].range;
        //FIND B*-TREE WITH THE GIVEN MODULE
        b_inx = find_btree(mod);
        if (b_inx == NIL)
        {
            return false;
        }
        x = modules_info[mod].x;
        y = modules_info[mod].y;
        rx = modules_info[mod].rx;
        ry = modules_info[mod].ry;
        left = x > rx ? rx : x;
        right = x > rx ? x : rx;
        top = y > ry ? y : ry;
        bottom = y > ry ? ry : y;
        QBTreeNode* qnode = find_qbnode_with_btree(b_trees[b_inx]);
        //CHECK BOUNDARY
        if (strcmp(constraints.range[i].boundary, "TOP") == 0)
        {
            cn = 0;
            if ((qnode->boundRect.top - range) < top)
                //VIOLATING TOP RANGE CONSTRAINT
            {
                //CANDIDATE GENERATION
                for (int t = 0; t < b_trees.size(); t++)
                {
                    bool flag = false;
                    auto nodes = b_trees[t]->allnodes();
                    QBTreeNode* leaf = find_qbnode_with_btree(b_trees[t]);

                    if (leaf == qnode)
                        continue;
                    for (int j = 0; j < nodes.size(); j++)
                    {
                        x = modules_info[nodes[j]->id].x;
                        y = modules_info[nodes[j]->id].y;
                        rx = modules_info[nodes[j]->id].rx;
                        ry = modules_info[nodes[j]->id].ry;
                        left = x > rx ? rx : x;
                        right = x > rx ? x : rx;
                        top = y > ry ? y : ry;
                        bottom = y > ry ? ry : y;

                        // cout<<top<< " : "<<leaf->boundRect.top<<endl;

                        if (leaf->boundRect.top - range > top)
                            // find other module in other b*-tree satisfied range constraint.
                        {
                            cn++;
                            Op2(nodes[j]->id, mod);
                            //cout<<"Range "<<i+1<<" : "<<"candidate generation is succeed."<<endl;
                            flag = true;
                            break;
                        }
                    }

                    if (flag)
                        break;
                }
                if (cn == 0)
                {
                    // cout<<"Range "<<i+1<<" : "<<"candidate generation is failed."<<endl;
                    return false;
                }
            }
            else
            {
                // cout<<"Range "<<i+1<<" : "<<"is satisfied."<<endl;
            }
        }

        else if (strcmp(constraints.range[i].boundary, "BOTTOM") == 0)
        {
            cn = 0;
            if ((qnode->boundRect.bottom + range) > bottom)
                //VIOLATING BOTTOM RANGE CONSTRAINT
            {
                //CANDIDATE GENERATION
                for (int t = 0; t < b_trees.size(); t++)
                {
                    bool flag = false;
                    auto nodes = b_trees[t]->allnodes();
                    QBTreeNode* leaf = find_qbnode_with_btree(b_trees[t]);

                    if (leaf == qnode)
                        continue;
                    for (int j = 0; j < nodes.size(); j++)
                    {
                        x = modules_info[nodes[j]->id].x;
                        y = modules_info[nodes[j]->id].y;
                        rx = modules_info[nodes[j]->id].rx;
                        ry = modules_info[nodes[j]->id].ry;
                        left = x > rx ? rx : x;
                        right = x > rx ? x : rx;
                        top = y > ry ? y : ry;
                        bottom = y > ry ? ry : y;

                        // cout<<top<< " : "<<leaf->boundRect.top<<endl;

                        if (leaf->boundRect.bottom + range < bottom)
                            // find other module in other b*-tree satisfied range constraint.
                        {
                            cn++;
                            Op2(nodes[j]->id, mod);
                            // cout<<"Range "<<i+1<<" : "<<"candidate generation is succeed."<<endl;
                            flag = true;
                            break;
                        }
                    }

                    if (flag)
                        break;
                }
                if (cn == 0)
                {
                    // cout<<"Range "<<i+1<<" : "<<"candidate generation is failed."<<endl;
                    return false;
                }
            }
            else
            {
                // cout<<"Range "<<i+1<<" : "<<"is satisfied."<<endl;
            }
        }
    }
    return true;
}

//********** Minimum_Separation Constraint Handling **********//
bool QBtree::minimum_seperation()
{
    int left, right, top, bottom, l, r, t, b, ml, mr, mt, mb;
    int x, y, rx, ry, dis, cn, mod, index, cx, cy, d;
    for (int i = 0; i < constraints.min_sep.size(); i++)
    {
        //GET MINIMUM SEPARATION DISTANCE
        dis = constraints.min_sep[i].dis;
        x = modules_info[constraints.min_sep[i].mod].x;
        y = modules_info[constraints.min_sep[i].mod].y;
        rx = modules_info[constraints.min_sep[i].mod].rx;
        ry = modules_info[constraints.min_sep[i].mod].ry;
        //DETERMINT BOUNDARY OF MODULE
        left = x > rx ? rx : x;
        right = x > rx ? x : rx;
        top = y > ry ? y : ry;
        bottom = y > ry ? ry : y;
        cx = right - (right - left) / 2;
        cy = top - (top - bottom) / 2;
        ml = mr = mt = mb = INT_MIN;

        cn = 0;
        mod = NIL;
        for (int j = 0; j < modules_info.size(); j++)
        {
            if (constraints.min_sep[i].mod == j)
                continue;
            x = modules_info[j].x;
            y = modules_info[j].y;
            rx = modules_info[j].rx;
            ry = modules_info[j].ry;

            l = x > rx ? rx : x;
            r = x > rx ? x : rx;
            t = y > ry ? y : ry;
            b = y > ry ? ry : y;

            x = r - (r - l) / 2;
            y = t - (t - b) / 2;

            //d = sqrt(pow((cx - x),2) + pow((cy - y),2)); 

            if (r + dis > left || l - dis < right || b - dis < top || t + dis > bottom)
                //if(d < dis)
                // if this module is abut with constraint module by a distance (distance < costraint distance)
            {
                cn++;
                break;
            }
        }
        //CANDIDATE GENERATION
        if (cn > 0)
        {
            cn = 0;
            for (int j = 1; j < qbnodes.size(); j++)
            {
                if (qbnodes[j].isleaf())
                {
                    // cout<<"Minimun Seperation "<<i+1<<" : "<<"candidate generation is succeed."<<endl;
                    cn++;
                    index = find_leaf_with_module(constraints.min_sep[i].mod);
                    Op3(index, j, constraints.min_sep[i].mod);
                    //showQBTree();
                    break;
                }
            }
            if (cn == 0)
            {
                // if candidate generation failed. 
                // index = find_leaf_with_module(constraints.min_sep[i].mod);
                return false;
            }
        }
        else
        {
            return true;
        }
    }
    return true;
}

bool QBtree::maximum_seperation()
{
    int x1, y1, x2, y2, mid1, mid2, x, y, rx, ry, dis, left, right, top, bottom, dis_min;
    long d;
    for (int i = 0; i < 1; i++)
    {
        //GET MODULE 1 AND 2
        mid1 = constraints.max_sep[i].mod1;
        mid2 = constraints.max_sep[i].mod2;

        //GET MODULE INFORMATION OF MODULE 1
        x = modules_info[mid1].x;
        y = modules_info[mid1].y;
        rx = modules_info[mid1].rx;
        ry = modules_info[mid1].ry;
        dis = constraints.max_sep[i].dis;

        //DETERMINE BOUNDARY OF MODULE 1
        left = x > rx ? rx : x;
        right = x > rx ? x : rx;
        top = y > ry ? y : ry;
        bottom = y > ry ? ry : y;
        x1 = right - (right - left) / 2; //CENTER OF THE MODULE
        y1 = top - (top - bottom) / 2;

        //GET MODULE INFORMATION OF MODULE 2
        x = modules_info[mid2].x;
        y = modules_info[mid2].y;
        rx = modules_info[mid2].rx;
        ry = modules_info[mid2].ry;

        //DETERMINE BOUDNARY OF MODULE 2
        left = x > rx ? rx : x;
        right = x > rx ? x : rx;
        top = y > ry ? y : ry;
        bottom = y > ry ? ry : y;
        x2 = right - (right - left) / 2;
        y2 = top - (top - bottom) / 2;
        // cout<<"x1 y1 and x2 y2"<<x1<<" "<<y1<<" "<<x2<<" "<<y2<<endl;
        d = sqrt((x1 - x2) * (x1 - x2) + (y1 - y2) * (y1 - y2));

        //CHECK MAXIMUM SPEARATION
        if (d > dis)
        {
            //FAILED CANDIDATE GENERATION
            if (!candidateGeneration(1, mid2, i))
            {
                //cout<<"Maximun Seperation "<<i+1<<" : "<<"candidate generation is failed."<<endl;
                return false;
            }
            else
            {
                //cout<<"Maximun Seperation "<<i+1<<" : "<<"candidate generation is succeed."<<endl;
            }
        }
        else
        {
            //cout<<"Maximun Seperation "<<i+1<<" : "<<"is satisfied."<<endl;
        }
    }
    return true;
}

//RETRIEVES B*-TREE WITH GIVEN MODULE
int QBtree::find_btree(int mod)
{
    for (int j = 0; j < b_trees.size(); j++)
    {
        auto nodes = b_trees[j]->allnodes();
        for (int t = 0; t < nodes.size(); t++)
        {
            if (nodes[t]->id == mod)
                return j;
        }
    }
    return NIL;
}

//********** CANDIDATE GENERATION FOR MAXIMUM SEPARATION **********//
bool QBtree::candidateGeneration(int constraint, int mod_id, int index)
{
    int left, bottom, right, top, x, y, rx, ry, width, height;
    int mod1, mod2, c_index = NIL;
    long dis, dis_min;
    dis_min = INT_MIN;
    mod1 = constraints.max_sep[index].mod1;
    mod2 = constraints.max_sep[index].mod2;
    //GET MODULE INFORMATION
    x = modules_info[mod1].x;
    y = modules_info[mod1].y;
    rx = modules_info[mod1].rx;
    ry = modules_info[mod1].ry;
    dis = constraints.max_sep[index].dis;
    //DETERMINE MODULE BOUNDARY
    left = x > rx ? rx : x;
    right = x > rx ? x : rx;
    top = y > ry ? y : ry;
    bottom = y > ry ? ry : y;
    //DETERMINE MAXIMUM SEPARATION
    left = right - (right - left) / 2 - dis;
    right = left + 2 * dis;
    bottom = top - (top - bottom) / 2 - dis;
    top = bottom + 2 * dis;

    //GENERATE CANDIDATE
    for (int i = 0; i < qbnodes.size(); i++)
    {
        if (!qbnodes[i].isleaf())
            continue;
        //IF QUADLEAF OVERLAPS WITH THE MAXIMUM_SEPARATION REGION
        if (qbnodes[i].boundRect.left < right && qbnodes[i].boundRect.right > left && qbnodes[i].boundRect.top > bottom && qbnodes[i].boundRect.bottom < top)
        {
            width = qbnodes[i].boundRect.right - qbnodes[i].boundRect.right;
            height = qbnodes[i].boundRect.top - qbnodes[i].boundRect.bottom;

            //IF NODE BELONGS TO TOP LEFT;
            if (qbnodes[qbnodes[i].parent].tl == i)
            {

                dis = sqrt((top - qbnodes[i].boundRect.bottom) * (top - qbnodes[i].boundRect.bottom) + (left - qbnodes[i].boundRect.right) * (left - qbnodes[i].boundRect.right));
                if (dis_min < dis && qbnodes[i].btree == nullptr)
                {
                    dis_min = dis;
                    c_index = i;
                }
            }

            //IF NODE BELONGS TO TOP RIGHT;
            if (qbnodes[qbnodes[i].parent].tr == i)
            {

                dis = sqrt((top - qbnodes[i].boundRect.bottom) * (top - qbnodes[i].boundRect.bottom) + (right - qbnodes[i].boundRect.left) * (right - qbnodes[i].boundRect.left));

                if (dis_min < dis && qbnodes[i].btree == nullptr)
                {
                    dis_min = dis;
                    c_index = i;
                }
            }

            //IF NODE BELONGS TO BOTTOM LEFT;
            if (qbnodes[qbnodes[i].parent].bl == i)
            {

                dis = sqrt((bottom - qbnodes[i].boundRect.top) * (bottom - qbnodes[i].boundRect.top) + (left - qbnodes[i].boundRect.right) * (left - qbnodes[i].boundRect.right));

                if (dis_min < dis && qbnodes[i].btree == nullptr)
                {
                    dis_min = dis;
                    c_index = i;
                }
            }

            //IF NODE BELONGS TO BOTTOM RIGHT;
            if (qbnodes[qbnodes[i].parent].br == i)
            {

                dis = sqrt((bottom - qbnodes[i].boundRect.top) * (bottom - qbnodes[i].boundRect.top) + (right - qbnodes[i].boundRect.left) * (right - qbnodes[i].boundRect.left));

                if (dis_min < dis && qbnodes[i].btree == nullptr)
                {
                    dis_min = dis;
                    c_index = i;
                }
            }
        }
    }
    //FAILED CANDIDATE GENERATION
    if (dis_min == INT_MIN || c_index == NIL)
    {
        return false;
    }

    int t = find_leaf_with_module(mod2);
    Op3(t, c_index, mod2);

    return true;
}

//RETRIEVES QBTREE NODE'S ID USING MODULE ID
int QBtree::find_leaf_with_module(int mod_id)
{
    int b_index = NIL;
    for (int i = 0; i < b_trees.size(); i++)
    {
        auto nodes = b_trees[i]->allnodes();
        for (int j = 0; j < nodes.size(); j++)
        {
            if (nodes[j]->id == mod_id)
            {
                b_index = i;
            }
        }
    }
    for (int i = 0; i < qbnodes.size(); i++)
    {
        if (qbnodes[i].btree == b_trees[b_index])
            return i;
    }
    return NIL;
}

//OPERATION TO INSERT A NODE WITH GIVEN ID INTO B*-TREE
void QBtree::Op1(int index, int op_index, int mod_id)
{
    if (index == op_index)
        return;
    //cout<<"index : "<<index<<" : "<<op_index<<" , mod : "<<mod_id<<endl;
    QBTreeNode* qnode = &qbnodes[index];
    QBTreeNode* op_qnode = &qbnodes[op_index];
    auto nodes = qnode->btree->allnodes();
    int t = find_btree(mod_id);
    //delete node.
    if (nodes.size() == 1)
    {
        //delete btree
        qnode->btree = nullptr;
        SAFE_DELETE(b_trees[t]);
        b_trees.erase(b_trees.begin() + t);
    }
    else
    {
        b_trees[t]->take_node(mod_id);
    }
    op_qnode->btree->insertNodeById(op_qnode->btree->allnodes()[0], mod_id);
}

//OPEARTION TO SWAP NODES IN TWO B*-TREES 
void QBtree::Op2(int op_mod_id, int mod_id)
{
    int i, j;
    Node* p;
    if (b_trees.size() > 1)
    {

        //get B*trees.
        i = find_btree(mod_id);
        j = find_btree(op_mod_id);

        if (mod_id == op_mod_id || i == j)
        {
            return;
        }

        b_trees[i]->insertNodeById(b_trees[i]->find_node_random(), op_mod_id);
        b_trees[i]->delete_node_by_id(mod_id);
        b_trees[j]->insertNodeById(b_trees[j]->find_node_random(), mod_id);
        b_trees[j]->delete_node_by_id(op_mod_id);
    }
}

//OPERATION TO MOVE A NODE FROM ONE B*-TREE TO ANOTHER
void QBtree::Op3(int index, int op_index, int mod_id)
{
    // cout<<"Op3 : "<<mod_id<<" "<<index<<" "<<op_index<<endl;
    Node* node = nullptr;
    int t = find_btree(mod_id);
    // cout<<"index : "<<index<< " op_index : "<<op_index<<" mod_id : "<<mod_id<<endl;
    QBTreeNode* qnode = &qbnodes[index];
    QBTreeNode* op_qnode = &qbnodes[op_index];
    auto nodes = qnode->btree->allnodes();
    //delete node.
    if (nodes.size() == 1)
    {
        //delete btree
        qnode->btree = nullptr;
        SAFE_DELETE(b_trees[t]);
        b_trees.erase(b_trees.begin() + t);
    }
    else
    {
        b_trees[t]->take_node(mod_id);
    }

    //insert node.
    if (op_qnode->btree != nullptr)
    {
        return;
    }

    //make new B-Tree
    op_qnode->btree = new B_Tree(alpha);
    vector<int> inds;
    inds.push_back(mod_id);
    constructBTree(*op_qnode->btree, inds);

    //register root node
    b_trees.push_back(op_qnode->btree);
}

//MOVES NODE FROM ONE QUADTREE LEAF TO ANOTHER
void QBtree::move_node_to_quad_leaf()
{
    int i, mid;
    //find another Q-Tree leaf node
    QBTreeNode* qnode = find_leaf_random();
    if (qnode == nullptr)
        return;

    //delete from the first B-Tree

    i = rand() % b_trees.size();
    if (b_trees[i]->allnodes().size() == 1)
    {
        //find QB-tree leaf with btree.
        QBTreeNode* qnode = find_qbnode_with_btree(b_trees[i]);
        qnode->btree = nullptr;
        //delete btree
        mid = b_trees[i]->take_node_random();
        SAFE_DELETE(b_trees[i]);
        b_trees.erase(b_trees.begin() + i);
    }
    //else normal insert.
    else
    {
        mid = b_trees[i]->take_node_random();
    }
    //make new B-Tree
    qnode->btree = new B_Tree(alpha);
    vector<int> inds;
    inds.push_back(mid);
    constructBTree(*qnode->btree, inds);
    //register root node
    b_trees.push_back(qnode->btree);
}

//MOVE NODE FROM ONE B*-TREE TO ANOTHER B*-TREE
void QBtree::move_node_to_b()
{
    int i, j, mid1, mid2;
    if (b_trees.size() > 1)
    {
        //generate 2 different indices in [0 until b_tress.size()]
        do
        {
            i = rand() % b_trees.size();
            j = rand() % b_trees.size();
        } while (i == j);

        //delete from the first B-Tree
        //if number of nodes of the first B-Tree is 0 then set Q-Node as leaf, remove the B-Tree from the b_trees.
        if (b_trees[i]->allnodes().size() == 1)
        {
            //find QB-tree leaf with btree.
            QBTreeNode* qnode = find_qbnode_with_btree(b_trees[i]);
            qnode->btree = nullptr;
            // delete btree
            mid1 = b_trees[i]->take_node_random();
            SAFE_DELETE(b_trees[i]);
            b_trees.erase(b_trees.begin() + i);
        }
        // [2.2]. else normal insert.
        else
        {
            mid1 = b_trees[i]->take_node_random();
        }
        // [3]. insert to the second B-Tree
        b_trees[j]->insertNodeById(b_trees[j]->find_node_random(), mid1);
    }
}

//SWAP NODES IN TWO B*-TREES
void QBtree::swap_node()
{
    int i, j, mid1, mid2;
    QBTreeNode* qnode1, * qnode2;
    //Swap Node between two B*-tree
    if (b_trees.size() > 1)
    {
        //generate 2 different indices in [0 until b_tress.size()]
        do
        {
            i = rand() % b_trees.size();
            j = rand() % b_trees.size();
        } while (i == j);
        mid1 = b_trees[i]->find_node_random()->id;
        mid2 = b_trees[j]->find_node_random()->id;
        //swap_node
        Op2(mid1, mid2);
        //showQBTree();
    }
}

//QB-TREE PACKING
void QBtree::packing()
{
    int i, j, t, r;
    modules_info.clear();
    modules_info.resize(modules.size());

    //place module.
    for (i = 0; i < qbnodes.size(); i++)
    {

        if (qbnodes[i].btree == nullptr)
        {
            continue;
        }
        //B-TREE PACKING
        qbnodes[i].btree->packing();
        vector<Node*> r = qbnodes[i].btree->allnodes();

        //x,y,rx,ry adjustment.
        if (qbnodes[qbnodes[i].parent].tl == i)
        { //FOR TOP LEFT
            for (j = 0; j < r.size(); j++)
            {
                modules_info[r[j]->id].x = (int)qbnodes[i].boundRect.right - qbnodes[i].btree->getModuleInfo()[r[j]->id].x;
                modules_info[r[j]->id].y = (int)qbnodes[i].boundRect.bottom + qbnodes[i].btree->getModuleInfo()[r[j]->id].y;
                modules_info[r[j]->id].rx = (int)qbnodes[i].boundRect.right - qbnodes[i].btree->getModuleInfo()[r[j]->id].rx;
                modules_info[r[j]->id].ry = (int)qbnodes[i].boundRect.bottom + qbnodes[i].btree->getModuleInfo()[r[j]->id].ry;
                modules_info[r[j]->id].rotate = qbnodes[i].btree->getModuleInfo()[r[j]->id].rotate;
                modules_info[r[j]->id].flip = qbnodes[i].btree->getModuleInfo()[r[j]->id].flip;
            }
        }
        else if (qbnodes[qbnodes[i].parent].tr == i)
        { //FOR TOP RIGHT
            for (j = 0; j < r.size(); j++)
            {
                modules_info[r[j]->id].x = qbnodes[i].boundRect.left + qbnodes[i].btree->getModuleInfo()[r[j]->id].x;
                modules_info[r[j]->id].y = qbnodes[i].boundRect.bottom + qbnodes[i].btree->getModuleInfo()[r[j]->id].y;
                modules_info[r[j]->id].rx = qbnodes[i].boundRect.left + qbnodes[i].btree->getModuleInfo()[r[j]->id].rx;
                modules_info[r[j]->id].ry = qbnodes[i].boundRect.bottom + qbnodes[i].btree->getModuleInfo()[r[j]->id].ry;
                modules_info[r[j]->id].rotate = qbnodes[i].btree->getModuleInfo()[r[j]->id].rotate;
                modules_info[r[j]->id].flip = qbnodes[i].btree->getModuleInfo()[r[j]->id].flip;
            }
        }
        else if (qbnodes[qbnodes[i].parent].bl == i)
        { //FOR BOTTOM LEFT
            for (j = 0; j < r.size(); j++)
            {
                modules_info[r[j]->id].x = qbnodes[i].boundRect.right - qbnodes[i].btree->getModuleInfo()[r[j]->id].x;
                modules_info[r[j]->id].y = qbnodes[i].boundRect.top - qbnodes[i].btree->getModuleInfo()[r[j]->id].y;
                modules_info[r[j]->id].rx = qbnodes[i].boundRect.right - qbnodes[i].btree->getModuleInfo()[r[j]->id].rx;
                modules_info[r[j]->id].ry = qbnodes[i].boundRect.top - qbnodes[i].btree->getModuleInfo()[r[j]->id].ry;
                modules_info[r[j]->id].rotate = qbnodes[i].btree->getModuleInfo()[r[j]->id].rotate;
                modules_info[r[j]->id].flip = qbnodes[i].btree->getModuleInfo()[r[j]->id].flip;
            }
        }
        else if (qbnodes[qbnodes[i].parent].br == i)
        { //FOR BOTTOM RIGHT
            for (j = 0; j < r.size(); j++)
            {
                modules_info[r[j]->id].x = qbnodes[i].boundRect.left + qbnodes[i].btree->getModuleInfo()[r[j]->id].x;
                modules_info[r[j]->id].y = qbnodes[i].boundRect.top - qbnodes[i].btree->getModuleInfo()[r[j]->id].y;
                modules_info[r[j]->id].rx = qbnodes[i].boundRect.left + qbnodes[i].btree->getModuleInfo()[r[j]->id].rx;
                modules_info[r[j]->id].ry = qbnodes[i].boundRect.top - qbnodes[i].btree->getModuleInfo()[r[j]->id].ry;
                modules_info[r[j]->id].rotate = qbnodes[i].btree->getModuleInfo()[r[j]->id].rotate;
                modules_info[r[j]->id].flip = qbnodes[i].btree->getModuleInfo()[r[j]->id].flip;
            }
        }
    }
    //COST CALUCLATION
    cost_evaluation();
}

//DISPLAY MODULES OF B*-TREES
void QBtree::show_module()
{
    for (int i = 0; i < modules_info.size(); i++)
    {
        cout << "Module: " << b_trees[0]->getModule()[i].name << " : " << i << endl;
        cout << "  Width = " << b_trees[0]->getModule()[i].width;
        cout << "  Height= " << b_trees[0]->getModule()[i].height;
        cout << "  Area  = " << b_trees[0]->getModule()[i].area << endl;
        cout << " x : " << modules_info[i].x;
        cout << " y : " << modules_info[i].y;
        cout << " rx : " << modules_info[i].rx;
        cout << " ry : " << modules_info[i].ry << endl;
    }
}

void QBtree::scaleIOPad()
{
    float px = (qbnodes[0].boundRect.right - qbnodes[0].boundRect.left) / float(root_module.width);
    float py = (qbnodes[0].boundRect.top - qbnodes[0].boundRect.bottom) / float(root_module.height);

    for (int i = 0; i < root_module.pins.size(); i++)
    {
        Pin& p = root_module.pins[i];
        p.ax = int(px * p.x);
        p.ay = int(py * p.y);
    }
}

//********** CALCULATE WIRELENGTH *********//
double QBtree::calcWireLength()
{
    scaleIOPad();
    WireLength = 0;
    //compute absolute position
    for (int i = 0; i < modules.size(); i++)
    {
        int mx = modules_info[i].x;
        int my = modules_info[i].y;
        bool rotate = modules_info[i].rotate;
        int w = modules[i].width;

        for (int j = 0; j < modules[i].pins.size(); j++)
        {
            Pin& p = modules[i].pins[j];
            if (!rotate)
            {
                p.ax = p.x + mx;
                p.ay = p.y + my;
            }
            else
            { //Y' = W - X, X' = Y
                p.ax = p.y + mx;
                p.ay = (w - p.x) + my;
            }
        }
    }
    for (int i = 0; i < network.size(); i++)
    {
        int max_x = INT_MIN;
        int max_y = INT_MIN;
        int min_x = INT_MAX;
        int min_y = INT_MAX;
        assert(network[i].size() > 0);
        for (int j = 0; j < network[i].size(); j++)
        {
            Pin& p = *network[i][j];
            max_x = max(max_x, p.ax);
            max_y = max(max_y, p.ay);
            min_x = min(min_x, p.ax);
            min_y = min(min_y, p.ay);
        }
        WireLength += (max_x - min_x) + (max_y - min_y);
    }

    return WireLength;
}

//********** NORMALIZED AREA CALCULATION *********//
double QBtree::calcNormalizeArea()
{
    double Area = 0;
    double x, y, rx, ry, l, b, r, t;
    x = modules_info[0].x > modules_info[0].rx ? modules_info[0].rx : modules_info[0].x;
    y = modules_info[0].y > modules_info[0].ry ? modules_info[0].ry : modules_info[0].y;
    rx = modules_info[0].x > modules_info[0].rx ? modules_info[0].x : modules_info[0].rx;
    ry = modules_info[0].y > modules_info[0].ry ? modules_info[0].y : modules_info[0].ry;
    for (int i = 0; i < modules_info.size(); i++)
    {
        l = modules_info[i].x > modules_info[i].rx ? modules_info[i].rx : modules_info[i].x;
        r = modules_info[i].x > modules_info[i].rx ? modules_info[i].x : modules_info[i].rx;
        b = modules_info[i].y > modules_info[i].ry ? modules_info[i].ry : modules_info[i].y;
        t = modules_info[i].y > modules_info[i].ry ? modules_info[i].y : modules_info[i].ry;

        if ((r - rx) > 0) {
            rx = r;
            // cout<<"right - "<<rx<<endl;
        }
        if ((t - ry) > 0) {
            ry = t;
            // cout<<"top - "<<ry<<endl;
        }
        if ((x - l) > 0) {
            x = l;
            //cout<<"left - "<<x<<endl;
        }
        if ((y - b) > 0) {
            y = b;
            //cout<<"bottom - "<<y<<endl;
        }
    }
    for (int i = 0; i < rects.size(); i++)
    {
        if (rx < rects[i].right)
            rx = rects[i].right;
        if (ry < rects[i].top)
            ry = rects[i].top;
        if (x > rects[i].left)
            x = rects[i].left;
        if (y > rects[i].bottom)
            y = rects[i].bottom;
    }
    if (ry > qbnodes[0].boundRect.right || rx > qbnodes[0].boundRect.top
        || x < qbnodes[0].boundRect.bottom || y < qbnodes[0].boundRect.bottom)
    {
        Area = (ry - y) * (rx - x) * 100;
    }
    else
        Area = (ry - y) * (rx - x);
    return Area;
}

//********** OUT-OF-BOUND AREA CALCULATION *********//
double QBtree::calcOutOfBoundArea()
{
    double c_area = 0;
    double bw, bh;
    double w, h;

    for (int i = 0; i < qbnodes.size(); i++)
    {
        if (qbnodes[i].btree == nullptr || qbnodes[i].btree->allnodes().size() == 0)
            continue;

        w = qbnodes[i].btree->getWidth();
        h = qbnodes[i].btree->getHeight();
        bw = qbnodes[i].boundRect.right - qbnodes[i].boundRect.left;
        bh = qbnodes[i].boundRect.top - qbnodes[i].boundRect.bottom;
        if (w > bw || h > bh)
        {
            if (w > bw && h > bh)
            {
                c_area += ((w * h) - (bw * bh));
            }
            else
            {
                c_area += w > bw ? (w - bw) * h : (h - bh) * w;
            }
        }
    }

    return c_area < 0 ? 0 : c_area;
}

//********** VIOLATION COST CALCULATION *********//
double QBtree::calcViolationCost()
{
    int mid1, mid2;
    double x, y, rx, ry, R, left, right, top, bottom, x1, x2, y1, y2;
    double max_cost, cltobndry_cost, range_cost;
    max_cost = cltobndry_cost = range_cost = 0;
    //FOR MAXIMUM SEPARATION
    if (!constraints.max_sep.empty())
    {
        for (int i = 0; i < constraints.max_sep.size(); i++)
        {
            //get Module 1,2.
            mid1 = constraints.max_sep[i].mod1;
            mid2 = constraints.max_sep[i].mod2;

            //get module_info.
            x = modules_info[mid1].x;
            y = modules_info[mid1].y;
            rx = modules_info[mid1].rx;
            ry = modules_info[mid1].ry;
            R = constraints.max_sep[i].dis;
            //determine module's boundary.
            left = x > rx ? rx : x;
            right = x > rx ? x : rx;
            top = y > ry ? y : ry;
            bottom = y > ry ? ry : y;
            x1 = right - (right - left) / 2;
            y1 = top - (top - bottom) / 2;

            //get module_info.
            x = modules_info[mid2].x;
            y = modules_info[mid2].y;
            rx = modules_info[mid2].rx;
            ry = modules_info[mid2].ry;
            //determine module's boundary.
            left = x > rx ? rx : x;
            right = x > rx ? x : rx;
            top = y > ry ? y : ry;
            bottom = y > ry ? ry : y;
            x2 = right - (right - left) / 2;
            y2 = top - (top - bottom) / 2;
            R = sqrt(pow(x1 - x2, 2) + pow(y1 - y2, 2)) / R;
            if (R <= 1)
            {
                R = 0;
            }
            max_cost += R;
        }
    }

    //FOR RANGE CONSTRAINT
    if (!constraints.range.empty())
    {
        int index;
        double t;
        for (int i = 0; i < constraints.range.size(); i++)
        {
            mid1 = constraints.range[i].mod;
            //get module_info.
            x = modules_info[mid1].x;
            y = modules_info[mid1].y;
            rx = modules_info[mid1].rx;
            ry = modules_info[mid1].ry;
            //determine module's boundary.
            left = x > rx ? rx : x;
            right = x > rx ? x : rx;
            top = y > ry ? y : ry;
            bottom = y > ry ? ry : y;
            //find node from module ID.
            index = find_leaf_with_module(mid1);

            if (strcmp(constraints.range[i].boundary, "TOP") == 0)
            {
                if (top > (qbnodes[index].boundRect.top - constraints.range[i].range))
                {
                    t = top - (qbnodes[index].boundRect.top - constraints.range[i].range);

                    if (t > (top - bottom))
                        t = top - bottom;

                    range_cost += t / (top - bottom);
                }
            }
            if (strcmp(constraints.range[i].boundary, "BOTTOM") == 0)
            {
                if (bottom < (qbnodes[index].boundRect.bottom + constraints.range[i].range))
                {
                    t = abs((qbnodes[index].boundRect.bottom + constraints.range[i].range) - bottom);

                    if (t > (top - bottom))
                        t = top - bottom;

                    range_cost += t / (top - bottom);
                }
            }
            if (strcmp(constraints.range[i].boundary, "LEFT") == 0)
            {
                if (left < (qbnodes[index].boundRect.left + constraints.range[i].range))
                {
                    t = abs((qbnodes[index].boundRect.left + constraints.range[i].range) - left);

                    if (t > (right - left))
                        t = right - left;

                    range_cost += t / (right - left);
                }
            }
            if (strcmp(constraints.range[i].boundary, "RIGHT") == 0)
            {
                if (right > (qbnodes[index].boundRect.right - constraints.range[i].range))
                {
                    t = right - (qbnodes[index].boundRect.right - constraints.range[i].range);

                    if (t > (right - left))
                        t = right - left;

                    range_cost += t / (right - left);
                }
            }
        }
    }

    //FOR CLOSE_TO_BOUNDARY CONSTRAINT
    if (!constraints.clto_boundary.empty())
    {
        int index, l, r, t, b;
        for (int i = 0; i < constraints.clto_boundary.size(); i++)
        {
            //get Module
            mid1 = constraints.clto_boundary[i].mod;
            //get module_info.
            x = modules_info[mid1].x;
            y = modules_info[mid1].y;
            rx = modules_info[mid1].rx;
            ry = modules_info[mid1].ry;
            R = constraints.clto_boundary[i].dis;
            //determine module's boundary.
            left = x > rx ? rx : x;
            right = x > rx ? x : rx;
            top = y > ry ? y : ry;
            bottom = y > ry ? ry : y;

            index = find_leaf_with_module(mid1);
            l = qbnodes[index].boundRect.left;
            r = qbnodes[index].boundRect.right;
            t = qbnodes[index].boundRect.top;
            b = qbnodes[index].boundRect.bottom;

            if (left > (l + R) && right < (r - R) && top < (t - R) && bottom >(b + R))
            {
                cltobndry_cost += 1;
            }
            else
            {
                l = l + R;
                r = r - R;
                t = t - R;
                b = b + R;

                if (left < l && right > l && top > t && bottom < t)
                {
                    cltobndry_cost += abs(((t - bottom) * (right - l)) / ((top - bottom) * (right - left)));
                }
                else if (left >= l && right <= r && top > t && bottom < t)
                {
                    cltobndry_cost += abs((t - bottom) / (top - bottom));
                }
                else if (left < r && right > r && top > t && bottom < t)
                {
                    cltobndry_cost += abs(((r - left) * (t - bottom)) / ((top - bottom) * (right - left)));
                }
                else if (left < r && right > r && top <= t && bottom >= b)
                {
                    cltobndry_cost += abs((r - left) / (right - left));
                }
                else if (left < r && right > r && top > b && bottom < b)
                {
                    cltobndry_cost += abs(((r - left) * (top - b)) / ((top - bottom) * (right - left)));
                }
                else if (left >= l && right <= r && top > b && bottom < b)
                {
                    cltobndry_cost += abs((top - b) / (top - bottom));
                }
                else if (left < l && right > l && top > b && bottom < b)
                {
                    cltobndry_cost += abs(((right - r) * (top - b)) / ((top - bottom) * (right - left)));
                }
                else if (left < l && right > l && top <= t && bottom >= b)
                {
                    cltobndry_cost += abs((right - r) / (right - left));
                }
            }
        }
    }

    //cout<<range_cost<<endl;

    return max_cost + range_cost + cltobndry_cost;
}

//********** COST CALCULATION *********//
double QBtree::getCost()
{
    float alpha, beta, gamma, ramda;
    double O, V;
    //VALUES SET FOR ALPHA, BETA, GAMMA AND RAMDA
    alpha = 1e-3;
    beta = gamma = ramda = 0.25;
    //WIRE LENGHT
    WireLength = calcWireLength();
    //NORMALIZED AREA
    Area = calcNormalizeArea();
    //OUT OF BOUND AREA
    O = calcOutOfBoundArea();
    //VIOLATION COST
    V = calcViolationCost();

    cost = alpha * WireLength * 1e-3 + beta * O / TotalArea + gamma * Area / TotalArea + ramda * V;

    cout << "   WireLength : " << WireLength * 1e-3 << endl;
    cout << "   Out of bound area : " << O * 1e-6 << endl;
    cout << "   Area: " << Area * 1e-6 << endl;
    cout << "   Total Area : " << TotalArea * 1e-6 << endl;
    cout << "   Violation cost : " << V << endl;
    cout << "   Total Cost : " << cost << endl;
    cout << "                " << endl;
    return cost;
}

//********* COST EVALUTION **********//
void QBtree::cost_evaluation()
{
    float alpha, beta, gamma, ramda;
    double O, V;


    alpha = 1e-4;
    beta = 1;
    gamma = 0.25;
    ramda = 1;
    WireLength = calcWireLength();
    Area = calcNormalizeArea();
    O = calcOutOfBoundArea();
    V = calcViolationCost();
    cost = alpha * WireLength * 1e-4 + beta * O / TotalArea + gamma * Area / TotalArea + ramda * V;

}
//********** SAVES CURRENT SOLUTION IF BETTER THAN PREVIOUS BEST SOLUTION *********//
void QBtree::keep_sol(Solution& sol)
{
    if (sol.cost != NIL)
    {
        // delete prev
        for (int i = 0; i < sol.qbnodes.size(); i++)
            if (sol.qbnodes[i].btree)
            {
                vector<Node> t = *(vector<Node>*)sol.qbnodes[i].btree;
                t.clear();

            }
    }
    sol.qbnodes = qbnodes;
    // copy new
    for (int i = 0; i < qbnodes.size(); i++)
    {
        if (qbnodes[i].btree != nullptr)
        {
            auto btree_vector = new vector<Node>();
            qbnodes[i].btree->copyTree(*btree_vector);
            sol.qbnodes[i].btree = (B_Tree*)btree_vector;
        }
        else
        {
            sol.qbnodes[i].btree = nullptr;
        }
    }
    sol.cost = cost;
}

//********** RECOVERS THE BEST SOLUTION *********//
void QBtree::recover(Solution& sol)
{
    for (int i = 0; i < b_trees.size(); i++)
    {
        b_trees[i]->destroy();
        SAFE_DELETE(b_trees[i]);
    }
    b_trees.clear();

    qbnodes = sol.qbnodes;
    for (int i = 0; i < sol.qbnodes.size(); i++)
    {
        if (sol.qbnodes[i].btree)
        {
            qbnodes[i].btree = new B_Tree(alpha);
            vector<Node>* t = (vector<Node>*)sol.qbnodes[i].btree;
            auto tree = new vector<Node>();
            copyTree(t, tree);
            initBTree(*qbnodes[i].btree);
            qbnodes[i].btree->nodes_root = tree->data();
            qbnodes[i].btree->initWithOutNode();

            b_trees.push_back(qbnodes[i].btree);
        }
        else
        {
            qbnodes[i].btree = nullptr;
        }
    }
    cost = sol.cost;
}

//********** COPY THE QB-TREE **********//
void QBtree::copyTree(vector<Node>* tree_o, vector<Node>* tree)
{
    // copy tree vector.
    for (int i = 0; i < tree_o->size(); i++)
    {
        Node* node = (Node*) new unsigned char[sizeof(Node)];
        node->id = tree_o->at(i).id;
        node->flip = tree_o->at(i).flip;
        node->rotate = tree_o->at(i).rotate;
        node->ratio = tree_o->at(i).ratio;
        node->parent = node->left = node->right = nullptr;
        tree->push_back(*node);
    }

    for (int i = 0; i < tree_o->size(); i++)
    {
        for (int j = 0; j < tree_o->size(); j++)
        {
            // j is i's parent.
            if (&tree_o->at(j) == tree_o->at(i).parent)
            {
                tree->at(i).parent = &tree->at(j);
            }
            // j is i's left child
            if (&tree_o->at(j) == tree_o->at(i).left)
            {
                tree->at(i).left = &tree->at(j);
            }
            // j is i's right child
            if (&tree_o->at(j) == tree_o->at(i).right)
            {
                tree->at(i).right = &tree->at(j);
            }
        }
    }
}

//********** SIMULATED ANNEALING SCHEME **********//
double QBtree::SA_Floorplan(int k, int local, float term_T)
{
    int MT, uphill, reject;
    double pre_cost, best;
    float d_cost, reject_rate;

    int N = k * modules.size();
    float P = 0.9;
    float T, actual_T = 1;
    double avg = init_avg;
    float conv_rate = 1;
    double time = seconds();


    double estimate_avg = 0.08 / avg_ratio;
    cout << "Estimate Average Delta Cost = " << estimate_avg << endl;

    if (local == 0)
        avg = estimate_avg;

    T = avg / log(P);

    pre_cost = best = bestSolution.cost;
    int good_num = 0, bad_num = 0;
    double total_cost = 0;
    int count = 0;
    ofstream of("/tmp/btree_debug");

    do
    {
        double cn = 0;
        float p_time = 0;
        float pk_time = 0;
        count++;
        total_cost = 0;
        MT = uphill = reject = 0;
        printf("Iteration %d, T= %.2f\n", count, actual_T);

        vector<double> chain;
        for (; uphill < N && MT < 2 * N; MT++)
        {
            perturbation();
            packing();
            d_cost = cost - pre_cost;
            float p = exp(d_cost / T);
            chain.push_back(cost);

            if (d_cost <= 0 || rand_01() < p)
            {
                keep_sol(lastSolution);
                pre_cost = cost;
                if (d_cost > 0)
                {
                    uphill++, bad_num++;
                    of << d_cost << ": " << p << endl;
                }
                else if (d_cost < 0)
                    good_num++;
                // keep best solution
                if (cost < best)
                {
                    keep_sol(bestSolution);
                    best = cost;
                    printf("   ==>  Cost= %f, Area= %.6f, ", best, Area * 1e-6);
                    printf("Wire= %.3f\n", WireLength * 1e-3);
                    assert(calcNormalizeArea() >= TotalArea);
                    time = seconds();
                }
            }
            else
            {
                reject++;
                recover(lastSolution);
            }
        }
        double sv = std_var(chain);
        float r_t = exp(lamda * T / sv);
        T = r_t * T;

        // After apply local-search, start to use normal SA
        if (count == local)
        {
            T = estimate_avg / log(P);
            T *= pow(0.9, local); // smoothing the annealing schedule
            actual_T = exp(estimate_avg / T);
        }
        if (count > local)
        {
            actual_T = exp(estimate_avg / T);
            conv_rate = 0.87;
        }

        reject_rate = float(reject) / MT;
        printf("  T= %.2f, r= %.2f, reject= %.2f\n", actual_T, r_t, reject_rate);
    } while (reject_rate < conv_rate && actual_T > term_T);

    if (reject_rate >= conv_rate)
        cout << "\n  Convergent!\n";
    else if (actual_T <= term_T)
        cout << "\n Cooling Enough!\n";

    printf("\n good = %d, bad=%d, rejected=%d\n\n", good_num, bad_num, reject);

    recover(bestSolution);
    packing();
    return time;
}

double QBtree::std_var(vector<double>& chain)
{
    double m;
    double sum = 0;
    double var;
    int N = chain.size();
    for (int i = 0; i < chain.size(); i++)
        sum += chain[i];

    m = sum / N;
    sum = 0;
    for (int i = 0; i < N; i++)
        sum += (chain[i] - m) * (chain[i] - m);

    var = sqrt(sum / (N - 1));

    printf("  m=%.4f ,v=%.4f\n", m, var);

    return var;
}

//********** CREATING THE PLOT FOR MATLAB **********//
void QBtree::outPutResult(char* filepath)
{
    FILE* fs = fopen(filepath, "w");
    int x1, y1, x2, y2;
    float r = 0.8, g = 0.3, b = 0.1;

    //Draw a ROOT REGION.
    fprintf(fs, "%%region \n");
    fprintf(fs, "x1=%d; \n", qbnodes[0].boundRect.left);
    fprintf(fs, "x2=%d; \n", qbnodes[0].boundRect.right);
    fprintf(fs, "y1=%d; \n", qbnodes[0].boundRect.bottom);
    fprintf(fs, "y2=%d; \n", qbnodes[0].boundRect.top);
    fprintf(fs, "rectangle('Position',[x1,y1,x2-x1,y2-y1],'FaceColor',[1 1 1],'EdgeColor','b','LineWidth',1);\n");

    //Draw a PPM
    for (int i = 0; i < rects.size(); i++)
    {
        fprintf(fs, "%%ppm%d \n", i + 1);
        fprintf(fs, "x1=%d; \n", rects[i].left);
        fprintf(fs, "x2=%d; \n", rects[i].right);
        fprintf(fs, "y1=%d; \n", rects[i].bottom);
        fprintf(fs, "y2=%d; \n", rects[i].top);
        //fprintf(fs,"x = [x1, x2, x2, x1, x1];\n y = [y1, y1, y2, y2, y1];\n plot(x, y, 'b-');\n hold on; \n");
        fprintf(fs, "rect = rectangle('Position',[x1,y1,x2-x1,y2-y1],'FaceColor',[.1 .5 .5],'EdgeColor','b','LineWidth',1);\n");
        fprintf(fs, "text(x1+5,y2-((y2-y1)/4),'ppm'); \n");

    }

    //Draw a Modules except max_sep modules.
    for (int i = 0; i < modules_info.size(); i++)
    {
        if (is_max_sep_module(i))
            continue;
        x1 = modules_info[i].x;
        x2 = modules_info[i].rx;
        y1 = modules_info[i].y;
        y2 = modules_info[i].ry;
        if (x1 > x2)
            swap(x1, x2);
        if (y1 > y2)
            swap(y1, y2);

        fprintf(fs, "%%module%d \n", i + 1);
        fprintf(fs, "x1=%d; \n", x1);
        fprintf(fs, "x2=%d; \n", x2);
        fprintf(fs, "y1=%d; \n", y1);
        fprintf(fs, "y2=%d; \n", y2);
        fprintf(fs, "rectangle('Position',[x1,y1,x2-x1,y2-y1],'FaceColor',[.6 .6 .6],'EdgeColor','b','LineWidth',1);\n");
        fprintf(fs, "str = '%s'; \n nstr = strrep(str,'_',' '); \n text(x1+5,y2-((y2-y1)/4),nstr);", modules[i].name);
    }

    //Draw max_sep modules.
    for (int i = 0; i < constraints.max_sep.size(); i++)
    {
        x1 = modules_info[constraints.max_sep[i].mod2].rx;
        x2 = modules_info[constraints.max_sep[i].mod2].x;
        y1 = modules_info[constraints.max_sep[i].mod2].ry;
        y2 = modules_info[constraints.max_sep[i].mod2].y;
        if (x1 > x2)
            swap(x1, x2);
        if (y1 > y2)
            swap(y1, y2);
        fprintf(fs, "%%module%d \n", constraints.max_sep[i].mod2 + 1);
        fprintf(fs, "x1=%d; \n", x1);
        fprintf(fs, "x2=%d; \n", x2);
        fprintf(fs, "y1=%d; \n", y1);
        fprintf(fs, "y2=%d; \n", y2);
        fprintf(fs, "rectangle('Position',[x1,y1,x2-x1,y2-y1],'FaceColor',[%f %f %f],'EdgeColor','b','LineWidth',1);\n", r, g, b);
        fprintf(fs, "str = '%s'; \n nstr = strrep(str,'_',' '); \n text(x1+5,y2-((y2-y1)/4),nstr);", modules[constraints.max_sep[i].mod2].name);

        x1 = modules_info[constraints.max_sep[i].mod1].rx;
        x2 = modules_info[constraints.max_sep[i].mod1].x;
        y1 = modules_info[constraints.max_sep[i].mod1].ry;
        y2 = modules_info[constraints.max_sep[i].mod1].y;

        if (x1 > x2)
            swap(x1, x2);
        if (y1 > y2)
            swap(y1, y2);

        fprintf(fs, "%%module%d \n", constraints.max_sep[i].mod1 + 1);
        fprintf(fs, "x1=%d; \n", x1);
        fprintf(fs, "x2=%d; \n", x2);
        fprintf(fs, "y1=%d; \n", y1);
        fprintf(fs, "y2=%d; \n", y2);
        fprintf(fs, "rectangle('Position',[x1,y1,x2-x1,y2-y1],'FaceColor',[%f %f %f],'EdgeColor','b','LineWidth',1);\n", r, g, b);
        fprintf(fs, "str = '%s'; \n nstr = strrep(str,'_',' '); \n text(x1+5,y2-((y2-y1)/4),nstr);", modules[constraints.max_sep[i].mod1].name);

        x1 = x2 - (x2 - x1) / 2;
        y1 = y2 - (y2 - y1) / 2;

        x1 = x1 - constraints.max_sep[i].dis;
        y1 = y1 - constraints.max_sep[i].dis;

        fprintf(fs, "%%region \n");
        fprintf(fs, "x1=%d; \n", x1);
        fprintf(fs, "x2=%d; \n", x1 + constraints.max_sep[i].dis * 2);
        fprintf(fs, "y1=%d; \n", y1);
        fprintf(fs, "y2=%d; \n", y1 + constraints.max_sep[i].dis * 2);
        //fprintf(fs,"x = [x1, x2, x2, x1, x1];\n y = [y1, y1, y2, y2, y1];\n plot(x, y, 'b-');\n hold on; \n");
        fprintf(fs, "rectangle('Position',[x1,y1,x2-x1,y2-y1],'FaceColor','none','EdgeColor',[%f %f %f],'LineWidth',1,'LineStyle','--');\n", r, g, b);

        r = r + 0.3 > 1 ? r + 0.3 - 1.0 : r + 0.3;
        g = g + 0.3 > 1 ? g + 0.3 - 1.0 : g + 0.3;
        b = b + 0.3 > 1 ? b + 0.3 - 1.0 : b + 0.3;
    }

    //Draw a partition line.
    for (int i = 1; i < qbnodes.size(); i++)
    {
        if (i != qbnodes[qbnodes[i].parent].tl)
            continue;

        x1 = qbnodes[i].boundRect.left;
        x2 = qbnodes[qbnodes[i].parent].boundRect.right;
        y1 = y2 = qbnodes[i].boundRect.bottom;
        fprintf(fs, "%%region \n");
        fprintf(fs, "x1=%d; \n", x1);
        fprintf(fs, "x2=%d; \n", x2);
        fprintf(fs, "y1=%d; \n", y1);
        fprintf(fs, "y2=%d; \n", y2 + 1);
        //fprintf(fs,"x = [x1, x2, x2, x1, x1];\n y = [y1, y1, y2, y2, y1];\n plot(x, y, 'b-');\n hold on; \n");
        fprintf(fs, "rectangle('Position',[x1,y1,x2-x1,y2-y1],'FaceColor',[1 1 1],'EdgeColor','r','LineWidth',1);\n");

        x1 = x2 = qbnodes[i].boundRect.right;
        y1 = qbnodes[qbnodes[i].parent].boundRect.bottom;
        y2 = qbnodes[qbnodes[i].parent].boundRect.top;
        fprintf(fs, "%%region \n");
        fprintf(fs, "x1=%d; \n", x1);
        fprintf(fs, "x2=%d; \n", x2 + 1);
        fprintf(fs, "y1=%d; \n", y1);
        fprintf(fs, "y2=%d; \n", y2);
        //fprintf(fs,"x = [x1, x2, x2, x1, x1];\n y = [y1, y1, y2, y2, y1];\n plot(x, y, 'b-');\n hold on; \n");
        fprintf(fs, "rectangle('Position',[x1,y1,x2-x1,y2-y1],'FaceColor',[1 1 1],'EdgeColor','r','LineWidth',1);\n");
    }
    fclose(fs);
}

bool QBtree::is_max_sep_module(int mid)
{
    for (int i = 0; i < constraints.max_sep.size(); i++)
    {
        if (mid == constraints.max_sep[i].mod1 || mid == constraints.max_sep[i].mod2)
        {
            return true;
        }
    }

    return false;
}