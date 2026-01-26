#include "tetrahedralization.hpp"
using namespace std;

void Tetrahedralization::assign_tetrahedrons(const std::vector<uint32_t>& tetrahedrons)
{
    uint32_t vertices_count = tetrahedrons[0];
    m_tetrahedrons.resize(tetrahedrons.size());
    m_neighbors.resize(tetrahedrons.size());
    for(uint32_t i=0; i<tetrahedrons.size(); i++)
    {
        vertices_count = max(vertices_count, tetrahedrons[i]);
        m_tetrahedrons[i] = tetrahedrons[i];
        m_neighbors[i] = UNDEFINED_VALUE;
    }
    m_vertices_incidents.resize(vertices_count+1);
    for(uint32_t i=0; i<tetrahedrons.size(); i++)
    {
        m_vertices_incidents[m_tetrahedrons[i]] = i/4;
    }

    // calculate neighbors and vertices incidents
    unordered_map<tuple<uint32_t,uint32_t,uint32_t>,uint32_t,iii32_hash> neighbor_cache;
    for(uint32_t i=0; i<tetrahedrons.size()/4; i++)
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
    
    m_visited = vector<uint32_t>(tetrahedrons.size()/4, UNDEFINED_VALUE);
    m_visited_index = 0;
}

uint32_t Tetrahedralization::get_tetrahedrons_count()
{
    return m_tetrahedrons.size() / 4;
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


uint32_t Tetrahedralization::get_tetrahedron_vertex(uint32_t t, uint32_t i)
{
    return m_tetrahedrons[4*t+i];
}
tuple<uint32_t,uint32_t,uint32_t,uint32_t> Tetrahedralization::get_tetrahedron_vertices(uint32_t t)
{
    return make_tuple(m_tetrahedrons[4*t+0],m_tetrahedrons[4*t+1],m_tetrahedrons[4*t+2],m_tetrahedrons[4*t+3]);
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

uint32_t Tetrahedralization::get_incident_tetrahedron(uint32_t v)
{
    return m_vertices_incidents[v];
}
vector<uint32_t> Tetrahedralization::get_incident_tetrahedrons(uint32_t v)
{
    vector<uint32_t> res;
    m_visit.push(get_incident_tetrahedron(v));
    while(!m_visit.empty())
    {
        uint32_t t = m_visit.front();
        m_visit.pop();
        if(UNDEFINED_VALUE==t || m_visited[t]==m_visited_index)
        {
            continue;
        }
        m_visited[t] = m_visited_index;
        if(!tetrahedron_has_vertex(t, v))
        {
            continue;
        }
        res.push_back(t);
        for(uint32_t i=0; i<4; i++)
        {
            m_visit.push(get_tetrahedron_neighbor(t, i).first);
        }
    }
    m_visited_index++;
    
    return res;
}
vector<uint32_t> Tetrahedralization::get_incident_tetrahedrons(uint32_t v0, uint32_t v1)
{
    vector<uint32_t> res = get_incident_tetrahedrons(v0);
    for(uint32_t i=0; i<res.size(); i++)
    {
        if(tetrahedron_has_vertex(res[i], v1))
        {
            continue;
        }
        res[i] = res.back();
        res.pop_back();
        i--;
    }
    
    return res;
}
uint32_t Tetrahedralization::get_incident_tetrahedron(uint32_t v0, uint32_t v1, uint32_t v2)
{
    vector<uint32_t> res = get_incident_tetrahedrons(v0);
    for(uint32_t i=0; i<res.size(); i++)
    {
        if(tetrahedron_has_vertex(res[i], v1) && tetrahedron_has_vertex(res[i], v2))
        {
            return res[i];
        }
    }
    
    return UNDEFINED_VALUE;
}

bool Tetrahedralization::tetrahedron_has_vertex(uint32_t t, uint32_t v)
{
    return m_tetrahedrons[4*t+0]==v || m_tetrahedrons[4*t+1]==v || m_tetrahedrons[4*t+2]==v || m_tetrahedrons[4*t+3]==v;
}

