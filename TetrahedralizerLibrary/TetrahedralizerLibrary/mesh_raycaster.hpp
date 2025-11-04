#ifndef mesh_raycaster_hpp
#define mesh_raycaster_hpp

#include "common_header.h"
#include "common_function.h"

class MeshRaycasterNode
{
public:
    double3 m_min;
    double3 m_max;
    uint32_t m_left_index, m_right_index;
    uint32_t* m_constraints_indexes;
    uint32_t m_constraints_count;
    
    MeshRaycasterNode();
    ~MeshRaycasterNode();
};

class MeshRaycaster
{
public:
    void Dispose();
    
    // vertices and constraints
    void AddMeshRaycasterInput(uint32_t explicit_count, double* explicit_values, uint32_t constraints_count, uint32_t* constraints);
    // ray position, ray direction, hit triangle index, hit distance, hit barycentric weight
    bool Raycast(const double3& p, const double3& n, uint32_t& t_i, double& t, double3& w);
    
private:
    genericPoint** m_vertices;
    uint32_t m_vertices_count;
    
    uint32_t* m_constraints; // a constraint points out using the right hand rule 
    uint32_t m_constraints_count; // number of constraints, same as m_constraints.size()/3
    
    vector<MeshRaycasterNode*> m_tree_nodes;
    
    uint32_t AddTreeNode(vector<uint32_t>&, double3*, double3*);
    
    bool Raycast(const double3& p, const double3& n, uint32_t&, double& t, double3& w, MeshRaycasterNode* node);
};

#endif
