#include "tetrahedralization.hpp"
using namespace std;

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
    unordered_map<tuple<uint32_t,uint32_t,uint32_t>,uint32_t,trio_iii_hash> neighbor_cache;
    for(uint32_t i=0; i<tetrahedrons_count; i++)
    {
        for(uint32_t f=0; f<4; f++)
        {
            auto [t0,t1,t2] = get_tetrahedron_facet(i,f);
            sort_ints(t0,t1,t2);
            auto it = neighbor_cache.find(make_tuple(t0,t1,t2));
            if(it != neighbor_cache.end())
            {
                uint32_t n = it->second;
                m_neighbors[n] = 4*i+f;
                m_neighbors[4*i+f] = n;
                neighbor_cache.erase(it);
            }
            else
            {
                neighbor_cache[make_tuple(t0,t1,t2)] = 4*i+f;
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
pair<uint32_t,uint32_t> Tetrahedralization::get_tetrahedron_neighbor(uint32_t t, uint32_t i)
{
    if(UNDEFINED_VALUE == m_neighbors[4*t+i])
    {
        return make_pair(UNDEFINED_VALUE,UNDEFINED_VALUE);
    }
    return make_pair(m_neighbors[4*t+i]/4, m_neighbors[4*t+i]%4);
}
tuple<uint32_t,uint32_t,uint32_t> Tetrahedralization::get_tetrahedron_facet(uint32_t t, uint32_t i)
{
    uint32_t f0 = m_tetrahedrons[4*t+0];
    uint32_t f1 = m_tetrahedrons[4*t+1];
    uint32_t f2 = m_tetrahedrons[4*t+2];
    uint32_t f3 = m_tetrahedrons[4*t+3];
    switch(i)
    {
        case 0:
            return make_tuple(f0,f1,f2);
        case 1:
            return make_tuple(f1,f0,f3);
        case 2:
            return make_tuple(f0,f2,f3);
        case 3:
            return make_tuple(f2,f1,f3);
        default:
            throw "wrong face index";
    }
}
uint32_t Tetrahedralization::get_vertex_incident_tetrahedron(uint32_t v)
{
    return m_vertices_incidents[v];
}

vector<uint32_t> Tetrahedralization::get_all_facets()
{
    vector<uint32_t> res;
    vector<bool> visited = vector<bool>(m_tetrahedrons.size(), false);
    for(uint32_t i=0; i<m_tetrahedrons.size()/4; i++)
    {
        for(uint32_t j=0; j<4; j++)
        {
            if(visited[4*i+j])
            {
                continue;
            }
            auto [p0,p1,p2] = get_tetrahedron_facet(i, j);
            res.push_back(p0);
            res.push_back(p1);
            res.push_back(p2);
            auto[n,f] = get_tetrahedron_neighbor(i, j);
            if(UNDEFINED_VALUE != n)
            {
                visited[4*n+f] = true;
            }
        }
    }
    
    return res;
}

vector<uint32_t> Tetrahedralization::get_bounding_facets()
{
    vector<uint32_t> res;
    for(uint32_t i=0; i<m_tetrahedrons.size()/4; i++)
    {
        for(uint32_t j=0; j<4; j++)
        {
            auto[n,f] = get_tetrahedron_neighbor(i, j);
            if(UNDEFINED_VALUE != n)
            {
                continue;
            }
            auto [p0,p1,p2] = get_tetrahedron_facet(i, j);
            res.push_back(p0);
            res.push_back(p1);
            res.push_back(p2);
        }
    }
    
    return res;
}
