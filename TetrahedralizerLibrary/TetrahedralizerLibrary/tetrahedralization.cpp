#include "tetrahedralization.hpp"

Tetrahedralization::Tetrahedralization()
{
    m_tetrahedrons = nullptr;
    m_neighbors = nullptr;
    m_vertices_incidents = nullptr;
}
Tetrahedralization::Tetrahedralization(uint32_t* tetrahedrons, uint32_t tetrahedrons_count)
{
    m_tetrahedrons = nullptr;
    m_neighbors = nullptr;
    m_vertices_incidents = nullptr;
    assign_tetrahedrons(tetrahedrons, tetrahedrons_count);
}
Tetrahedralization::~Tetrahedralization()
{
    delete[] m_tetrahedrons;
    delete[] m_neighbors;
    delete[] m_vertices_incidents;
}


void Tetrahedralization::assign_tetrahedrons(uint32_t* tetrahedrons, uint32_t tetrahedrons_count)
{
    delete[] m_tetrahedrons;
    delete[] m_neighbors;
    delete[] m_vertices_incidents;
    
    m_tetrahedrons_count = tetrahedrons_count;
    m_tetrahedrons = new uint32_t[4*m_tetrahedrons_count];
    m_neighbors = new uint32_t[4*m_tetrahedrons_count];
    for(uint32_t i=0; i<4*m_tetrahedrons_count; i++)
    {
        m_tetrahedrons[i] = tetrahedrons[i];
        m_neighbors[i] = UNDEFINED_VALUE;
    }
    
    uint32_t max_index = m_tetrahedrons[0];
    for(uint32_t i=1; i<4*tetrahedrons_count; i++)
    {
        max_index = max(max_index, m_tetrahedrons[i]);
    }
    m_vertices_incidents = new uint32_t[1+max_index];
    for(uint32_t i=0; i<1+max_index; i++)
    {
        m_vertices_incidents[i] = UNDEFINED_VALUE;
    }

    // calculate neighbors and vertices incidents
    m_u_map_iii_i_0.clear();
    for(uint32_t i=0; i<4*m_tetrahedrons_count; i+=4)
    {
        for(uint32_t f=0; f<4; f++)
        {
            m_vertices_incidents[m_tetrahedrons[i+f]] = i;

            uint32_t t0,t1,t2;
            get_tetrahedron_face(i,f,t0,t1,t2);
            sort_ints(t0,t1,t2);
            auto it = m_u_map_iii_i_0.find(make_tuple(t0,t1,t2));
            if(it != m_u_map_iii_i_0.end())
            {
                uint32_t n = it->second;
                m_neighbors[n] = i+f;
                m_neighbors[i+f] = n;
                m_u_map_iii_i_0.erase(it);
            }
            else
            {
                m_u_map_iii_i_0[make_tuple(t0,t1,t2)] = i+f;
            }
        }
    }
}

void Tetrahedralization::get_tetrahedron_opposite_vertex(uint32_t t, uint32_t& p)
{
    uint32_t i = t & 3;
    t = t & 0xfffffffc;
    switch(i)
    {
        case 0:
            p = m_tetrahedrons[t+3];
            break;
        case 1:
            p = m_tetrahedrons[t+2];
            break;
        case 2:
            p = m_tetrahedrons[t+1];
            break;
        case 3:
            p = m_tetrahedrons[t+0];
            break;
        default:
            throw "wrong face index";
    }
}
void Tetrahedralization::get_tetrahedron_neighbor(uint32_t t, uint32_t i, uint32_t& n)
{
    if(UNDEFINED_VALUE == m_neighbors[t+i])
    {
        n = UNDEFINED_VALUE;
    }
    else
    {
        n = m_neighbors[t+i] & 0xfffffffc;
    }
}
void Tetrahedralization::get_tetrahedron_face(uint32_t t, uint32_t i, uint32_t& f0,uint32_t& f1,uint32_t& f2)
{
    switch(i)
    {
        case 0:
            f0 = m_tetrahedrons[t+0];
            f1 = m_tetrahedrons[t+1];
            f2 = m_tetrahedrons[t+2];
            break;
        case 1:
            f0 = m_tetrahedrons[t+1];
            f1 = m_tetrahedrons[t+0];
            f2 = m_tetrahedrons[t+3];
            break;
        case 2:
            f0 = m_tetrahedrons[t+0];
            f1 = m_tetrahedrons[t+2];
            f2 = m_tetrahedrons[t+3];
            break;
        case 3:
            f0 = m_tetrahedrons[t+2];
            f1 = m_tetrahedrons[t+1];
            f2 = m_tetrahedrons[t+3];
            break;
        default:
            throw "wrong face index";
    }
}
void Tetrahedralization::get_tetrahedron_face(uint32_t t, uint32_t& f0,uint32_t& f1,uint32_t& f2)
{
    get_tetrahedron_face(t&0xfffffffc,t&3,f0,f1,f2);
}

void Tetrahedralization::get_vertex_incident_tetrahedron(uint32_t v, uint32_t& t)
{
    t = m_vertices_incidents[v];
}

uint32_t Tetrahedralization::get_vertex_incident_tetrahedron(uint32_t v)
{
    return m_vertices_incidents[v];
}

bool Tetrahedralization::tetrahedron_contains_vertex(uint32_t t, uint32_t v)
{
    return m_tetrahedrons[t+0] == v || m_tetrahedrons[t+1] == v || m_tetrahedrons[t+2] == v || m_tetrahedrons[t+3] == v;
}
