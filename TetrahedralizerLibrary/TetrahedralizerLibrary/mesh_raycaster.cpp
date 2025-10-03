#include "mesh_raycaster.hpp"

MeshRaycasterNode::MeshRaycasterNode()
{
    m_min_x = m_min_y = m_min_z = numeric_limits<double>::max();
    m_max_x = m_max_y = m_max_z = numeric_limits<double>::min();
    m_left_index = m_right_index = UNDEFINED_VALUE;
    m_constraints_indexes = nullptr;
    m_constraints_count = 0;
}

MeshRaycasterNode::~MeshRaycasterNode()
{
    delete[] m_constraints_indexes;
}

void MeshRaycasterHandle::Dispose()
{
    delete_vertices(m_vertices, m_vertices_count);
    delete[] m_constraints;
    
    for(uint32_t i=0; i<m_tree_nodes.size(); i++)
    {
        delete m_tree_nodes[i];
    }
}

void MeshRaycasterHandle::AddMeshRaycasterInput(uint32_t explicit_count, double* explicit_values, uint32_t constraints_count, uint32_t* constraints)
{
    create_vertices(explicit_count, explicit_values, 0, nullptr, m_vertices, m_vertices_count);
    
    m_constraints = new uint32_t[3*constraints_count];
    for(uint32_t i=0; i<3*constraints_count; i++)
    {
        m_constraints[i] = constraints[i];
    }
    
    double* constraints_bounds = new double[6*constraints_count];
    double* constraints_centers = new double[3*constraints_count];
    
    for(uint32_t i=0; i<constraints_count; i++)
    {
        uint32_t c0 = m_constraints[3*i+0];
        uint32_t c1 = m_constraints[3*i+1];
        uint32_t c2 = m_constraints[3*i+2];
        
        constraints_bounds[6*i+0] = min(explicit_values[3*c0+0], min(explicit_values[3*c1+0], explicit_values[3*c2+0]));
        constraints_bounds[6*i+1] = min(explicit_values[3*c0+1], min(explicit_values[3*c1+1], explicit_values[3*c2+1]));
        constraints_bounds[6*i+2] = min(explicit_values[3*c0+2], min(explicit_values[3*c1+2], explicit_values[3*c2+2]));
        
        constraints_bounds[6*i+3] = max(explicit_values[3*c0+0], max(explicit_values[3*c1+0], explicit_values[3*c2+0]));
        constraints_bounds[6*i+4] = max(explicit_values[3*c0+1], max(explicit_values[3*c1+1], explicit_values[3*c2+1]));
        constraints_bounds[6*i+5] = max(explicit_values[3*c0+2], max(explicit_values[3*c1+2], explicit_values[3*c2+2]));
        
        constraints_centers[3*i+0] = (explicit_values[3*c0+0]+explicit_values[3*c1+0]+explicit_values[3*c2+0]) / 3.0;
        constraints_centers[3*i+1] = (explicit_values[3*c0+1]+explicit_values[3*c1+1]+explicit_values[3*c2+1]) / 3.0;
        constraints_centers[3*i+2] = (explicit_values[3*c0+2]+explicit_values[3*c1+2]+explicit_values[3*c2+2]) / 3.0;
    }
    
    vector<uint32_t> constraints_indexes;
    for(uint32_t i=0; i<constraints_count; i++)
    {
        constraints_indexes.push_back(i);
    }
    AddTreeNode(constraints_indexes, constraints_bounds, constraints_centers);
    
    delete[] constraints_bounds;
    delete[] constraints_centers;
}


uint32_t MeshRaycasterHandle::AddTreeNode(vector<uint32_t>& constraints_indexes, double* constraints_bounds, double* constraints_centers)
{
    uint32_t res = m_tree_nodes.size();
    MeshRaycasterNode* node = new MeshRaycasterNode();
    m_tree_nodes.push_back(node);

    double m_min_x(constraints_bounds[6*constraints_indexes[0]+0]),m_min_y(constraints_bounds[6*constraints_indexes[0]+1]),m_min_z(constraints_bounds[6*constraints_indexes[0]+2]);
    double m_max_x(constraints_bounds[6*constraints_indexes[0]+3]),m_max_y(constraints_bounds[6*constraints_indexes[0]+4]),m_max_z(constraints_bounds[6*constraints_indexes[0]+5]);
    
    for(uint32_t i=1; i<constraints_indexes.size(); i++)
    {
        m_min_x = min(m_min_x, constraints_bounds[6*constraints_indexes[i]+0]);
        m_min_y = min(m_min_y, constraints_bounds[6*constraints_indexes[i]+1]);
        m_min_z = min(m_min_z, constraints_bounds[6*constraints_indexes[i]+2]);
        
        m_max_x = max(m_max_x, constraints_bounds[6*constraints_indexes[i]+3]);
        m_max_y = max(m_max_y, constraints_bounds[6*constraints_indexes[i]+4]);
        m_max_z = max(m_max_z, constraints_bounds[6*constraints_indexes[i]+5]);
    }
    
    node->m_min_x = m_min_x;
    node->m_min_y = m_min_y;
    node->m_min_z = m_min_z;
    node->m_max_x = m_max_x;
    node->m_max_y = m_max_y;
    node->m_max_z = m_max_z;
    
    if(constraints_indexes.size()<=4)
    {
        node->m_constraints_count = constraints_indexes.size();
        node->m_constraints_indexes = new uint32_t[node->m_constraints_count];
        for(uint32_t i=0; i<node->m_constraints_count; i++)
        {
            node->m_constraints_indexes[i] = constraints_indexes[i];
        }
    }
    else
    {
        uint32_t o = 0;
        double dx = m_max_x-m_min_x;
        double dy = m_max_y-m_min_y;
        double dz = m_max_z-m_min_z;
        if(dy>dx && dy>dz)
        {
            o=1;
        }
        else if(dz>dx)
        {
            o=2;
        }
        sort(constraints_indexes.begin(), constraints_indexes.end(), [&](uint32_t a, uint32_t b){
            return constraints_centers[3*a+o] < constraints_centers[3*b+o];
        });
        
        vector<uint32_t> new_constraints_indexes;
        for(uint32_t i=constraints_indexes.size()/2; i>=0; i--)
        {
            new_constraints_indexes.push_back(constraints_indexes.back());
            constraints_indexes.pop_back();
        }
        node->m_left_index = AddTreeNode(constraints_indexes, constraints_bounds, constraints_centers);
        node->m_right_index = AddTreeNode(new_constraints_indexes, constraints_bounds, constraints_centers);
    }
    
    return res;
}
