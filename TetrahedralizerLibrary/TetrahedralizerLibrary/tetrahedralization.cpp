#include "tetrahedralization.hpp"

Tetrahedralization::Tetrahedralization()
{}
Tetrahedralization::Tetrahedralization(uint32_t* tetrahedrons, uint32_t tetrahedrons_count)
{
    assign_tetrahedrons(tetrahedrons, tetrahedrons_count);
}
Tetrahedralization::~Tetrahedralization()
{}


void Tetrahedralization::assign_tetrahedrons(uint32_t* tetrahedrons, uint32_t tetrahedrons_count)
{
    uint32_t vertices_count = tetrahedrons[0];
    m_tetrahedrons.resize(4*tetrahedrons_count);
    m_neighbors.resize(4*tetrahedrons_count);
    for(uint32_t i=0; i<4*tetrahedrons_count; i++)
    {
        vertices_count = max(vertices_count, tetrahedrons[i]);
        m_tetrahedrons[i] = tetrahedrons[i];
        m_neighbors[i] = UNDEFINED_VALUE;
    }
    m_vertices_incidents.resize(vertices_count+1);
    for(uint32_t i=0; i<4*tetrahedrons_count; i++)
    {
        m_vertices_incidents[m_tetrahedrons[i]] = i/4;
    }

    // calculate neighbors and vertices incidents
    m_u_map_iii_i_0.clear();
    for(uint32_t i=0; i<tetrahedrons_count; i++)
    {
        for(uint32_t f=0; f<4; f++)
        {
            uint32_t t0,t1,t2;
            get_tetrahedron_facet(i,f,t0,t1,t2);
            sort_ints(t0,t1,t2);
            auto it = m_u_map_iii_i_0.find(make_tuple(t0,t1,t2));
            if(it != m_u_map_iii_i_0.end())
            {
                uint32_t n = it->second;
                m_neighbors[n] = 4*i+f;
                m_neighbors[4*i+f] = n;
                m_u_map_iii_i_0.erase(it);
            }
            else
            {
                m_u_map_iii_i_0[make_tuple(t0,t1,t2)] = 4*i+f;
            }
        }
    }
}

uint32_t Tetrahedralization::get_tetrahedrons_count()
{
    return m_tetrahedrons.size() / 4;
}

uint32_t Tetrahedralization::get_tetrahedron_vertex(uint32_t t, uint32_t i)
{
    return m_tetrahedrons[4*t+i];
}
uint32_t Tetrahedralization::get_tetrahedron_neighbor(uint32_t t, uint32_t i)
{
    if(UNDEFINED_VALUE == m_neighbors[4*t+i])
    {
        return UNDEFINED_VALUE;
    }
    return m_neighbors[4*t+i] / 4;
}
void Tetrahedralization::get_tetrahedron_facet(uint32_t t, uint32_t i, uint32_t& f0,uint32_t& f1,uint32_t& f2)
{
    switch(i)
    {
        case 0:
            f0 = m_tetrahedrons[4*t+0];
            f1 = m_tetrahedrons[4*t+1];
            f2 = m_tetrahedrons[4*t+2];
            break;
        case 1:
            f0 = m_tetrahedrons[4*t+1];
            f1 = m_tetrahedrons[4*t+0];
            f2 = m_tetrahedrons[4*t+3];
            break;
        case 2:
            f0 = m_tetrahedrons[4*t+0];
            f1 = m_tetrahedrons[4*t+2];
            f2 = m_tetrahedrons[4*t+3];
            break;
        case 3:
            f0 = m_tetrahedrons[4*t+2];
            f1 = m_tetrahedrons[4*t+1];
            f2 = m_tetrahedrons[4*t+3];
            break;
        default:
            throw "wrong face index";
    }
}
uint32_t Tetrahedralization::get_vertex_incident_tetrahedron(uint32_t v)
{
    return m_vertices_incidents[v];
}
