#include "mesh_raycaster.hpp"

MeshRaycasterNode::MeshRaycasterNode()
{
    m_min = double3(numeric_limits<double>::max(),numeric_limits<double>::max(),numeric_limits<double>::max());
    m_max = double3(numeric_limits<double>::min(),numeric_limits<double>::min(),numeric_limits<double>::min());
    m_left_index = m_right_index = UNDEFINED_VALUE;
    m_constraints_indexes = nullptr;
    m_constraints_count = 0;
}

MeshRaycasterNode::~MeshRaycasterNode()
{
    delete[] m_constraints_indexes;
}

void MeshRaycaster::Dispose()
{
    delete_vertices(m_vertices, m_vertices_count);
    delete[] m_constraints;
    
    for(uint32_t i=0; i<m_tree_nodes.size(); i++)
    {
        delete m_tree_nodes[i];
    }
}

void MeshRaycaster::AddMeshRaycasterInput(uint32_t explicit_count, double* explicit_values, uint32_t constraints_count, uint32_t* constraints)
{
    create_vertices(explicit_count, explicit_values, 0, nullptr, m_vertices, m_vertices_count);
    m_constraints = duplicate_array(constraints, 3*constraints_count);
    
    double3* constraints_bounds = new double3[2*constraints_count];
    double3* constraints_centers = new double3[constraints_count];
    
    for(uint32_t i=0; i<constraints_count; i++)
    {
        uint32_t c0 = m_constraints[3*i+0];
        uint32_t c1 = m_constraints[3*i+1];
        uint32_t c2 = m_constraints[3*i+2];
        double3 p0 = double3(explicit_values[3*c0+0],explicit_values[3*c0+1],explicit_values[3*c0+2]);
        double3 p1 = double3(explicit_values[3*c1+0],explicit_values[3*c1+1],explicit_values[3*c1+2]);
        double3 p2 = double3(explicit_values[3*c2+0],explicit_values[3*c2+1],explicit_values[3*c2+2]);
        
        constraints_bounds[2*i+0] = p0.min(p1).min(p2);
        constraints_bounds[2*i+1] = p0.max(p1).max(p2);
        constraints_centers[i] = (p0+p1+p2) / 3.0;
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

bool MeshRaycaster::Raycast(const double3& p, const double3& n, uint32_t& t_i, double& t, double3& w)
{
    t = numeric_limits<double>::max();
    return Raycast(p,n,t_i,t,w, m_tree_nodes[0]);
}

bool MeshRaycaster::Raycast(const double3& p, const double3& n, uint32_t& t_i, double& t, double3& w, MeshRaycasterNode* node)
{
    double tmin, tmax;
    if(!raycast_AABB(p, n, node->m_min, node->m_max, tmin, tmax))
    {
        return false;
    }

    if(0 != node->m_constraints_count)
    {
        bool hit = false;
        for(uint32_t i=0; i<node->m_constraints_count; i++)
        {
            uint32_t c0 = m_constraints[3*node->m_constraints_indexes[i]+0];
            uint32_t c1 = m_constraints[3*node->m_constraints_indexes[i]+1];
            uint32_t c2 = m_constraints[3*node->m_constraints_indexes[i]+2];
            
            double3 p0 = approximate_point(m_vertices[c0]);
            double3 p1 = approximate_point(m_vertices[c1]);
            double3 p2 = approximate_point(m_vertices[c2]);
            
            double new_t;
            double3 new_w;
            if(raycast_triangle(p, n, p0, p1, p2, new_t, new_w))
            {
                hit = true;
                if(abs(new_t) < t)
                {
                    t = abs(new_t);
                    w = new_w;
                    t_i = node->m_constraints_indexes[i];
                }
            }
        }
        return hit;
    }
    else
    {
        bool hit_left = Raycast(p,n,t_i,t,w, m_tree_nodes[node->m_left_index]);
        bool hit_right = Raycast(p,n,t_i,t,w, m_tree_nodes[node->m_right_index]);
        return hit_left || hit_right;
    }
}


uint32_t MeshRaycaster::AddTreeNode(vector<uint32_t>& constraints_indexes, double3* constraints_bounds, double3* constraints_centers)
{
    uint32_t res = m_tree_nodes.size();
    MeshRaycasterNode* node = new MeshRaycasterNode();
    m_tree_nodes.push_back(node);

    double3 m_min = constraints_bounds[2*constraints_indexes[0]+0];
    double3 m_max = constraints_bounds[2*constraints_indexes[0]+1];
    for(uint32_t i=1; i<constraints_indexes.size(); i++)
    {
        m_min = m_min.min(constraints_bounds[2*constraints_indexes[i]+0]);
        m_max = m_max.max(constraints_bounds[2*constraints_indexes[i]+1]);
    }
    
    node->m_min = m_min;
    node->m_max = m_max;
    
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
        double3 del = node->m_max - node->m_min;
        if(del.y>del.x && del.y>del.z)
        {
            o=1;
        }
        else if(del.z>del.x)
        {
            o=2;
        }
        sort(constraints_indexes.begin(), constraints_indexes.end(), [&](uint32_t a, uint32_t b){
            return constraints_centers[a][o] < constraints_centers[b][o];
        });
        
        vector<uint32_t> new_constraints_indexes;
        for(int i=constraints_indexes.size()/2; i>=0; i--)
        {
            new_constraints_indexes.push_back(constraints_indexes.back());
            constraints_indexes.pop_back();
        }
        node->m_left_index = AddTreeNode(constraints_indexes, constraints_bounds, constraints_centers);
        node->m_right_index = AddTreeNode(new_constraints_indexes, constraints_bounds, constraints_centers);
    }
    
    return res;
}
