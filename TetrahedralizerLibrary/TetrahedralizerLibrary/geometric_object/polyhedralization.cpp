#include "polyhedralization.hpp"
using namespace std;

int Polyhedralization::slice_polyhedron_with_plane(uint32_t p, uint32_t c0, uint32_t c1, uint32_t c2)
{
    unordered_map<uint32_t,int> orient_cache;
    vector<uint32_t> top_facets;
    vector<uint32_t> bot_facets;
    unordered_set<uint32_t> on_segments;
    unordered_map<uint32_t,tuple<uint32_t,uint32_t,uint32_t>> split_segments; // (intersection vertex, top edge, bottom edge)
    for(uint32_t f : m_polyhedrons[p])
    {
        vector<uint32_t> top_segments;
        vector<uint32_t> bot_segments;
        bool has_edge_on_constraint = false;
        uint32_t i0(UNDEFINED_VALUE), i1(UNDEFINED_VALUE);
        for(uint32_t s : m_facets[f].segments)
        {
            if(split_segments.end() == split_segments.find(s))
            {
                auto [i_p,top_s,bot_s,vs] = Segment::slice_segment_with_plane(s, c0, c1, c2, m_vertices, m_segments, orient_cache);
                split_segments[s] = make_tuple(i_p,top_s,bot_s);
                if(0 != vs.size())
                {
                    m_inserted_vertices.push_back(vs);
                }
            }
            
            auto [i_p,top_s,bot_s] = split_segments[s];
            if(UNDEFINED_VALUE == i_p && top_s == UNDEFINED_VALUE && bot_s == UNDEFINED_VALUE)
            {
                on_segments.insert(s);
                has_edge_on_constraint = true;
                continue;
            }
            
            if(UNDEFINED_VALUE != i_p)
            {
                if(UNDEFINED_VALUE == i0 || i_p == i0)
                {
                    i0 = i_p;
                }
                else
                {
                    i1 = i_p;
                }
            }
            if(UNDEFINED_VALUE != top_s)
            {
                top_segments.push_back(top_s);
            }
            if(UNDEFINED_VALUE != bot_s)
            {
                bot_segments.push_back(bot_s);
            }
        }
        
        if(has_edge_on_constraint || i1 == UNDEFINED_VALUE) // constraint intersects on one or more edges of the facet or does not intersect the facet
        {
            if(!top_segments.empty())
            {
                top_facets.push_back(f);
            }
            else if(!bot_segments.empty())
            {
                bot_facets.push_back(f);
            }
        }
        else
        {
            uint32_t i_e = m_segments.size();
            m_segments.push_back(Segment());
            m_segments[i_e].e0 = i0;
            m_segments[i_e].e1 = i1;
            m_segments[i_e].p0 = m_facets[f].p0;
            m_segments[i_e].p1 = m_facets[f].p1;
            m_segments[i_e].p2 = m_facets[f].p2;
            m_segments[i_e].p3 = c0;
            m_segments[i_e].p4 = c1;
            m_segments[i_e].p5 = c2;
            on_segments.insert(i_e);
            
            m_facets[f].segments.clear();
            uint32_t b_f = m_facets.size();
            m_facets.push_back(Facet(m_facets[f]));
            m_facets[f].segments = top_segments;
            m_facets[f].segments.push_back(i_e);
            m_facets[b_f].segments = bot_segments;
            m_facets[b_f].segments.push_back(i_e);
            top_facets.push_back(f);
            bot_facets.push_back(b_f);
        }
    }
    
    // constraint does not slice the polyhedron
    if(0 == top_facets.size() || 0 == bot_facets.size())
    {
        if(0 != top_facets.size())
        {
            return 1;
        }
        if(0 != bot_facets.size())
        {
            return -1;
        }
    }
    if(top_facets.size()<3 || bot_facets.size()<3 || on_segments.size()<3)
    {
        throw "wtf";
    }
    
    uint32_t b_p = m_polyhedrons.size();
    m_polyhedrons.push_back(vector<uint32_t>());
    
    uint32_t common_facet = m_facets.size();
    m_facets.push_back(Facet());
    m_facets[common_facet].p0 = c0;
    m_facets[common_facet].p1 = c1;
    m_facets[common_facet].p2 = c2;
    m_facets[common_facet].ip0 = p;
    m_facets[common_facet].ip1 = b_p;
    m_facets[common_facet].segments = vector<uint32_t>(on_segments.begin(),on_segments.end());

    m_polyhedrons[p] = top_facets;
    m_polyhedrons[p].push_back(common_facet);
    m_polyhedrons[b_p] = bot_facets;
    m_polyhedrons[b_p].push_back(common_facet);
    
    for(uint32_t f : m_polyhedrons[b_p])
    {
        if(f == common_facet)
        {
            continue;
        }
        uint32_t n;
        if(p == m_facets[f].ip0)
        {
            m_facets[f].ip0 = b_p;
            n = m_facets[f].ip1;
        }
        else
        {
            m_facets[f].ip1 = b_p;
            n = m_facets[f].ip0;
        }
        if(n != UNDEFINED_VALUE)
        {
            if(m_polyhedrons[n].end() == find(m_polyhedrons[n].begin(), m_polyhedrons[n].end(), f))
            {
                m_polyhedrons[n].push_back(f);
            }
        }
    }
    
    for(auto [k, v] : split_segments)
    {
        uint32_t ori_s = k;
        auto [i_p, top_s, bot_s] = v;

        if(UNDEFINED_VALUE == i_p || UNDEFINED_VALUE == top_s || UNDEFINED_VALUE == bot_s)
        {
            continue;
        }
        
        if(ori_s == bot_s)
        {
            swap(top_s, bot_s);
        }
        
        unordered_set<uint32_t> visited_polyhedrons;
        visited_polyhedrons.insert(UNDEFINED_VALUE);
        queue<uint32_t> visit_polyhedrons;
        visit_polyhedrons.push(p);
        while(!visit_polyhedrons.empty())
        {
            uint32_t cur = visit_polyhedrons.front();
            visit_polyhedrons.pop();
            if(visited_polyhedrons.end() != visited_polyhedrons.find(cur))
            {
                continue;
            }
            visited_polyhedrons.insert(cur);
            
            bool search_neighbor = false;
            for(uint32_t f : m_polyhedrons[cur])
            {
                if(m_facets[f].contains_segment(ori_s))
                {
                    search_neighbor = true;
                    if(m_polyhedrons[p].end() != find(m_polyhedrons[p].begin(), m_polyhedrons[p].end(), f))
                    {
                        continue;
                    }
                    if(m_polyhedrons[b_p].end() != find(m_polyhedrons[b_p].begin(), m_polyhedrons[b_p].end(), f))
                    {
                        continue;
                    }
                    if(!m_facets[f].contains_segment(bot_s))
                    {
                        m_facets[f].segments.push_back(bot_s);
                    }
                }
            }
            
            if(search_neighbor)
            {
                for(uint32_t f : m_polyhedrons[cur])
                {
                    if(cur == m_facets[f].ip0)
                    {
                        visit_polyhedrons.push(m_facets[f].ip1);
                    }
                    else
                    {
                        visit_polyhedrons.push(m_facets[f].ip0);
                    }
                }
            }
        }
    }
    return 0;
}

void Polyhedralization::calculate_facets_centroids()
{
    for(uint32_t i=0; i<m_facets.size(); i++)
    {
        vector<uint32_t> vs = m_facets[i].get_sorted_vertices(m_segments);
        double3 centroid = m_facets[i].get_centroid(m_vertices, m_segments);
        double3 weight;
        barycentric_weight(m_facets[i].p0,m_facets[i].p1,m_facets[i].p2,centroid,m_vertices.data(), weight);
        m_facets[i].w0 = weight.x;
        m_facets[i].w1 = weight.y;
    }
}
