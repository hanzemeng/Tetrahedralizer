#include "polyhedralization.hpp"
using namespace std;

Polyhedralization::Polyhedralization()
{
    m_visit_index = 0;
}

int Polyhedralization::slice_polyhedron_with_plane(uint32_t p, uint32_t c0, uint32_t c1, uint32_t c2)
{
    unordered_map<uint32_t,int> orient_cache;
    vector<uint32_t> top_facets;
    vector<uint32_t> bot_facets;
    unordered_set<uint32_t> on_segments;
    unordered_map<uint32_t,tuple<uint32_t,uint32_t,uint32_t>> processed_segments; // (intersection vertex, top edge, bottom edge)
    unordered_map<uint32_t,uint32_t> split_segments;
    for(uint32_t f : m_polyhedrons[p])
    {
        vector<uint32_t> top_segments;
        vector<uint32_t> bot_segments;
        bool has_edge_on_constraint = false;
        uint32_t i0(UNDEFINED_VALUE), i1(UNDEFINED_VALUE);
        for(uint32_t s : m_facets[f].segments)
        {
            if(processed_segments.end() == processed_segments.find(s))
            {
                auto [i_p,top_s,bot_s,vs] = Segment::slice_segment_with_plane(s, c0, c1, c2, m_vertices, m_segments, orient_cache);
                processed_segments[s] = make_tuple(i_p,top_s,bot_s);
                if(0 != vs.size())
                {
                    m_inserted_vertices.push_back(vs);
                    if(s==top_s)
                    {
                        split_segments[s] = bot_s;
                    }
                    else
                    {
                        split_segments[s] = top_s;
                    }
                }
            }
            
            auto [i_p,top_s,bot_s] = processed_segments[s];
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
            m_segments.push_back(Segment(i0,i1,m_facets[f].p0,m_facets[f].p1,m_facets[f].p2,c0,c1,c2));
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
    
    m_visited_polyhedrons.resize(m_polyhedrons.size(), UNDEFINED_VALUE);
    unordered_set<uint32_t> split_polyhedron_facets;
    split_polyhedron_facets.insert(m_polyhedrons[p].begin(), m_polyhedrons[p].end());
    split_polyhedron_facets.insert(m_polyhedrons[b_p].begin(), m_polyhedrons[b_p].end());
    
    for(auto [ori_s, new_s] : split_segments)
    {
        m_visit_polyhedrons.push(p);
        while(!m_visit_polyhedrons.empty())
        {
            uint32_t cur = m_visit_polyhedrons.front();
            m_visit_polyhedrons.pop();
            if(UNDEFINED_VALUE==cur || m_visit_index==m_visited_polyhedrons[cur])
            {
                continue;
            }
            m_visited_polyhedrons[cur] = m_visit_index;
            
            bool search_neighbor = false;
            for(uint32_t f : m_polyhedrons[cur])
            {
                if(!m_facets[f].contains_segment(ori_s))
                {
                    continue;
                }
                search_neighbor = true;
                if(split_polyhedron_facets.end() != split_polyhedron_facets.find(f))
                {
                    continue;
                }
                if(!m_facets[f].contains_segment(new_s))
                {
                    m_facets[f].segments.push_back(new_s);
                }
            }
            
            if(search_neighbor)
            {
                for(uint32_t f : m_polyhedrons[cur])
                {
                    if(cur == m_facets[f].ip0)
                    {
                        m_visit_polyhedrons.push(m_facets[f].ip1);
                    }
                    else
                    {
                        m_visit_polyhedrons.push(m_facets[f].ip0);
                    }
                }
            }
        }
        m_visit_index++;
    }
    
    return 0;
}

bool Polyhedralization::slice_facet_with_plane(uint32_t f, uint32_t c0, uint32_t c1, uint32_t c2)
{
    unordered_map<uint32_t,int> orient_cache;
    unordered_map<uint32_t,uint32_t> split_segments;
    vector<uint32_t> top_segments;
    vector<uint32_t> bot_segments;
    bool has_edge_on_constraint = false;
    uint32_t i0(UNDEFINED_VALUE), i1(UNDEFINED_VALUE);
    
    for(uint32_t s : m_facets[f].segments)
    {
        auto [i_p,top_s,bot_s,vs] = Segment::slice_segment_with_plane(s, c0, c1, c2, m_vertices, m_segments, orient_cache);
        if(0 != vs.size())
        {
            m_inserted_vertices.push_back(vs);
            if(s==top_s)
            {
                split_segments[s] = bot_s;
            }
            else
            {
                split_segments[s] = top_s;
            }
        }
        
        if(UNDEFINED_VALUE == i_p && top_s == UNDEFINED_VALUE && bot_s == UNDEFINED_VALUE)
        {
            has_edge_on_constraint = true;
            break;
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
        return false;
    }

    uint32_t i_e = m_segments.size();
    m_segments.push_back(Segment(i0,i1,m_facets[f].p0,m_facets[f].p1,m_facets[f].p2,c0,c1,c2));
    
    m_facets[f].segments.clear();
    uint32_t b_f = m_facets.size();
    m_facets.push_back(Facet(m_facets[f]));
    m_facets[f].segments = top_segments;
    m_facets[f].segments.push_back(i_e);
    m_facets[b_f].segments = bot_segments;
    m_facets[b_f].segments.push_back(i_e);
    
    uint32_t p0 = m_facets[f].ip0;
    uint32_t p1 = m_facets[f].ip1;
    if(UNDEFINED_VALUE != p0)
    {
        if(m_polyhedrons[p0].end() == find(m_polyhedrons[p0].begin(), m_polyhedrons[p0].end(), f))
        {
            m_polyhedrons[p0].push_back(f);
        }
        if(m_polyhedrons[p0].end() == find(m_polyhedrons[p0].begin(), m_polyhedrons[p0].end(), b_f))
        {
            m_polyhedrons[p0].push_back(b_f);
        }
    }
    if(UNDEFINED_VALUE != p1)
    {
        if(m_polyhedrons[p1].end() == find(m_polyhedrons[p1].begin(), m_polyhedrons[p1].end(), f))
        {
            m_polyhedrons[p1].push_back(f);
        }
        if(m_polyhedrons[p1].end() == find(m_polyhedrons[p1].begin(), m_polyhedrons[p1].end(), b_f))
        {
            m_polyhedrons[p1].push_back(b_f);
        }
    }

    m_visited_polyhedrons.resize(m_polyhedrons.size(), UNDEFINED_VALUE);
    for(auto [ori_s, new_s] : split_segments)
    {
        for(uint32_t i=0; i<m_facets.size(); i++)
        {
            if(i==f || i==b_f)
            {
                continue;
            }
            if(m_facets[i].contains_segment(ori_s) && !m_facets[i].contains_segment(new_s))
            {
                m_facets[i].segments.push_back(new_s);
            }
        }
//        m_visit_polyhedrons.push(p0);
//        m_visit_polyhedrons.push(p1);
//        while(!m_visit_polyhedrons.empty())
//        {
//            uint32_t cur = m_visit_polyhedrons.front();
//            m_visit_polyhedrons.pop();
//            if(UNDEFINED_VALUE==cur || m_visit_index==m_visited_polyhedrons[cur])
//            {
//                continue;
//            }
//            m_visited_polyhedrons[cur] = m_visit_index;
//            
//            bool search_neighbor = false;
//            for(uint32_t cur_f : m_polyhedrons[cur])
//            {
//                if(!m_facets[cur_f].contains_segment(ori_s))
//                {
//                    continue;
//                }
//                search_neighbor = true;
//                if(f==cur_f || b_f==cur_f)
//                {
//                    continue;
//                }
//                if(!m_facets[cur_f].contains_segment(new_s))
//                {
//                    m_facets[cur_f].segments.push_back(new_s);
//                }
//            }
//            
//            if(search_neighbor)
//            {
//                for(uint32_t cur_f : m_polyhedrons[cur])
//                {
//                    if(cur == m_facets[cur_f].ip0)
//                    {
//                        m_visit_polyhedrons.push(m_facets[cur_f].ip1);
//                    }
//                    else
//                    {
//                        m_visit_polyhedrons.push(m_facets[cur_f].ip0);
//                    }
//                }
//            }
//        }
//        m_visit_index++;
    }
    
    for(uint32_t i=0; i<m_facets.size(); i++)
    {
        m_facets[i].get_sorted_vertices(m_segments);
    }
    
    return true;
}

vector<uint32_t> Polyhedralization::get_polyhedron_vertices(uint32_t p)
{
    unordered_set<uint32_t> res;
    for(uint32_t f : m_polyhedrons[p])
    {
        vector<uint32_t> vs = m_facets[f].get_vertices(m_segments);
        res.insert(vs.begin(),vs.end());
    }
    return vector<uint32_t>(res.begin(),res.end());
}
