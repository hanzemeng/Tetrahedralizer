#ifndef mesh_raycaster_hpp
#define mesh_raycaster_hpp

#include "common_header.h"
#include "common_function.h"

class MeshRaycasterNode
{
public:
    double m_min_x,m_min_y,m_min_z;
    double m_max_x,m_max_y,m_max_z;
    uint32_t m_left_index, m_right_index;
    uint32_t* m_constraints_indexes;
    uint32_t m_constraints_count;
    
    MeshRaycasterNode();
    ~MeshRaycasterNode();
};

class MeshRaycasterHandle
{
public:
    void Dispose();
    
    // vertices and constraints
    void AddMeshRaycasterInput(uint32_t, double*, uint32_t, uint32_t*);
    
private:
    genericPoint** m_vertices;
    uint32_t m_vertices_count;
    
    uint32_t* m_constraints; // a constraint points out using the right hand rule 
    uint32_t m_constraints_count; // number of constraints, same as m_constraints.size()/3
    
    vector<MeshRaycasterNode*> m_tree_nodes;
    
    uint32_t AddTreeNode(vector<uint32_t>&, double*, double*);
};

#endif
