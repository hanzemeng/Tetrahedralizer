#include "polyhedralization.hpp"
using namespace std;

void Polyhedralization::prepare_to_slice()
{
    m_slice_index = 0;
    m_slice_facets_cache = vector<uint32_t>(m_facets.size(), UNDEFINED_VALUE);
    m_slice_segments_cache = vector<tuple<uint32_t,uint32_t,uint32_t,uint32_t>>(m_segments.size(), make_tuple(UNDEFINED_VALUE,UNDEFINED_VALUE,UNDEFINED_VALUE,UNDEFINED_VALUE));
    m_slice_vertices_cache = vector<pair<uint32_t,int>>(m_vertices.size(), make_pair(UNDEFINED_VALUE,0));
    
    calculate_facets_incident_polyhedrons();
    
    m_segments_incident_facets = vector<vector<uint32_t>>(m_segments.size());
    for(uint32_t i=0; i<m_facets.size(); i++)
    {
        for(uint32_t s : m_facets[i].segments)
        {
            m_segments_incident_facets[s].push_back(i);
        }
    }
}

void Polyhedralization::calculate_facets_incident_polyhedrons()
{
    m_facets_incident_polyhedrons = vector<uint32_t>(2*m_facets.size(), UNDEFINED_VALUE);
    for(uint32_t i=0; i<m_polyhedrons.size(); i++)
    {
        for(uint32_t f : m_polyhedrons[i])
        {
            if(UNDEFINED_VALUE == m_facets_incident_polyhedrons[2*f+0])
            {
                m_facets_incident_polyhedrons[2*f+0] = i;
            }
            else
            {
                m_facets_incident_polyhedrons[2*f+1] = i;
            }
        }
    }
}

int Polyhedralization::slice_polyhedron_with_plane(uint32_t p, uint32_t c0, uint32_t c1, uint32_t c2)
{
    auto t0 = chrono::steady_clock::now();
    vector<uint32_t> top_facets;
    vector<uint32_t> bot_facets;
    vector<uint32_t> on_segments;
    vector<uint32_t> split_segments;
    
    vector<uint32_t> top_segments;
    vector<uint32_t> bot_segments;
    m_slice_segments_cache.resize(m_segments.size(), make_tuple(UNDEFINED_VALUE,UNDEFINED_VALUE,UNDEFINED_VALUE,UNDEFINED_VALUE));
    m_slice_vertices_cache.resize(m_vertices.size(), make_pair(UNDEFINED_VALUE,0));
    for(uint32_t f : m_polyhedrons[p])
    {
        top_segments.clear();
        bot_segments.clear();
        bool has_edge_on_constraint = false;
        uint32_t i0(UNDEFINED_VALUE), i1(UNDEFINED_VALUE);
        for(uint32_t s : m_facets[f].segments)
        {
            if(m_slice_index != get<0>(m_slice_segments_cache[s]))
            {
                uint32_t e0 = m_segments[s].e0;
                uint32_t e1 = m_segments[s].e1;
                if(m_slice_index != m_slice_vertices_cache[e0].first)
                {
                    m_slice_vertices_cache[e0] = make_pair(m_slice_index, orient3d(c0,c1,c2,e0,m_vertices.data()));
                }
                if(m_slice_index != m_slice_vertices_cache[e1].first)
                {
                    m_slice_vertices_cache[e1] = make_pair(m_slice_index, orient3d(c0,c1,c2,e1,m_vertices.data()));
                }
                auto [i_p,top_s,bot_s,vs] = Segment::slice_segment_with_plane(s, c0, c1, c2, m_vertices, m_segments, m_slice_vertices_cache[e0].second, m_slice_vertices_cache[e1].second);
                m_slice_segments_cache[s] = make_tuple(m_slice_index,i_p,top_s,bot_s);
                if(UNDEFINED_VALUE != vs.p0)
                {
                    vs.push_back_to_vector(m_inserted_vertices);
                    split_segments.push_back(bot_s);
                    m_segments_incident_facets.push_back(m_segments_incident_facets[s]);
                }
                else if(UNDEFINED_VALUE == i_p && top_s == UNDEFINED_VALUE && bot_s == UNDEFINED_VALUE)
                {
                    on_segments.push_back(s);
                    has_edge_on_constraint = true;
                }
            }
            
            auto [_,i_p,top_s,bot_s] = m_slice_segments_cache[s];
            if(UNDEFINED_VALUE == i_p && top_s == UNDEFINED_VALUE && bot_s == UNDEFINED_VALUE)
            {
                has_edge_on_constraint = true;
                continue;
            }
            
            if(UNDEFINED_VALUE != i_p)
            {
                if(UNDEFINED_VALUE == i0 || i_p == i0)
                {
                    i0 = i_p;
                }
                else if(UNDEFINED_VALUE == i1 || i_p == i1)
                {
                    i1 = i_p;
                }
                else
                {
                    cerr << p << " " << c0 << " " << c1 << " "<< c2 << "\n";
                    throw "wtf";
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
            on_segments.push_back(i_e);
            
            m_facets[f].segments.clear();
            uint32_t b_f = m_facets.size();
            m_facets.push_back(Facet(m_facets[f]));
            m_facets_incident_polyhedrons.push_back(m_facets_incident_polyhedrons[2*f+0]);
            m_facets_incident_polyhedrons.push_back(m_facets_incident_polyhedrons[2*f+1]);
            m_facets[f].segments = top_segments;
            m_facets[b_f].segments = bot_segments;
            for(uint32_t s : m_facets[b_f].segments)
            {
                *find(m_segments_incident_facets[s].begin(), m_segments_incident_facets[s].end(), f) = b_f;
            }
            m_facets[f].segments.push_back(i_e);
            m_facets[b_f].segments.push_back(i_e);
            m_segments_incident_facets.push_back(vector<uint32_t>{f,b_f});
            
            top_facets.push_back(f);
            bot_facets.push_back(b_f);
            m_slice_facets_cache[f] = m_slice_index;
            m_slice_facets_cache.push_back(m_slice_index);
        }
    }
    
    // constraint does not slice the polyhedron
    if(0 == top_facets.size() || 0 == bot_facets.size())
    {
        m_slice_index++;
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
        cerr << p << " " << c0 << " " << c1 << " "<< c2 << "\n";
        throw "wtf";
    }
    
    auto t1 = chrono::steady_clock::now();
    
    m_polyhedrons[p] = std::move(top_facets);
    uint32_t b_p = m_polyhedrons.size();
    m_polyhedrons.push_back(std::move(bot_facets));
    
    uint32_t common_facet = m_facets.size();
    m_facets.push_back(Facet());
    m_facets[common_facet].p0 = c0;
    m_facets[common_facet].p1 = c1;
    m_facets[common_facet].p2 = c2;
    m_facets_incident_polyhedrons.push_back(p);
    m_facets_incident_polyhedrons.push_back(b_p);
    m_facets[common_facet].segments = std::move(on_segments);
    for(uint32_t s : m_facets[common_facet].segments)
    {
        m_segments_incident_facets[s].push_back(common_facet);
    }
    
    for(uint32_t f : m_polyhedrons[b_p])
    {
        uint32_t n;
        if(p == m_facets_incident_polyhedrons[2*f+0])
        {
            m_facets_incident_polyhedrons[2*f+0] = b_p;
            n = m_facets_incident_polyhedrons[2*f+1];
        }
        else
        {
            m_facets_incident_polyhedrons[2*f+1] = b_p;
            n = m_facets_incident_polyhedrons[2*f+0];
        }
        if(n != UNDEFINED_VALUE)
        {
            if(m_polyhedrons[n].end() == find(m_polyhedrons[n].begin(), m_polyhedrons[n].end(), f))
            {
                m_polyhedrons[n].push_back(f);
            }
        }
    }
    m_polyhedrons[p].push_back(common_facet);
    m_polyhedrons[b_p].push_back(common_facet);
    
    m_slice_facets_cache.resize(m_facets.size(), UNDEFINED_VALUE);
    for(uint32_t new_s : split_segments)
    {
        for(uint32_t i=0; i<m_segments_incident_facets[new_s].size(); i++)
        {
            uint32_t f = m_segments_incident_facets[new_s][i];
            if(m_slice_index == m_slice_facets_cache[f])
            {
                continue;
            }
            m_facets[f].segments.push_back(new_s);
        }
    }
    
//    for(uint32_t f : m_polyhedrons[p])
//    {
//        m_facets[f].check_validity(m_vertices, m_segments);
//    }
//    for(uint32_t f : m_polyhedrons[b_p])
//    {
//        m_facets[f].check_validity(m_vertices, m_segments);
//    }
    
    m_slice_index++;
    
    auto t2 = chrono::steady_clock::now();
    
//    cout << chrono::duration_cast<std::chrono::microseconds>(t1-t0).count() << ", " << chrono::duration_cast<std::chrono::microseconds>(t2-t1).count() << "\n";
    return 0;
}

bool Polyhedralization::slice_facet_with_plane(uint32_t f, uint32_t c0, uint32_t c1, uint32_t c2)
{
//    unordered_map<uint32_t,int> orient_cache;
//    unordered_map<uint32_t,uint32_t> split_segments; // new s, old s
//    vector<uint32_t> top_segments;
//    vector<uint32_t> bot_segments;
//    bool has_edge_on_constraint = false;
//    uint32_t i0(UNDEFINED_VALUE), i1(UNDEFINED_VALUE);
//    
//    for(uint32_t s : m_facets[f].segments)
//    {
//        auto [i_p,top_s,bot_s,vs] = Segment::slice_segment_with_plane(s, c0, c1, c2, m_vertices, m_segments, orient_cache);
//        if(0 != vs.size())
//        {
//            m_inserted_vertices.push_back(vs);
//            split_segments[bot_s] = s;
//        }
//        
//        if(UNDEFINED_VALUE == i_p && top_s == UNDEFINED_VALUE && bot_s == UNDEFINED_VALUE)
//        {
//            has_edge_on_constraint = true;
//            break;
//        }
//        
//        if(UNDEFINED_VALUE != i_p)
//        {
//            if(UNDEFINED_VALUE == i0 || i_p == i0)
//            {
//                i0 = i_p;
//            }
//            else
//            {
//                i1 = i_p;
//            }
//        }
//        if(UNDEFINED_VALUE != top_s)
//        {
//            top_segments.push_back(top_s);
//        }
//        if(UNDEFINED_VALUE != bot_s)
//        {
//            bot_segments.push_back(bot_s);
//        }
//    }
//    
//    if(has_edge_on_constraint || i1 == UNDEFINED_VALUE) // constraint intersects on one or more edges of the facet or does not intersect the facet
//    {
//        return false;
//    }
//
//    uint32_t i_e = m_segments.size();
//    m_segments.push_back(Segment(i0,i1,m_facets[f].p0,m_facets[f].p1,m_facets[f].p2,c0,c1,c2));
//    
//    m_facets[f].segments.clear();
//    uint32_t b_f = m_facets.size();
//    m_facets.push_back(Facet(m_facets[f]));
//    m_facets[f].segments = top_segments;
//    m_facets[f].segments.push_back(i_e);
//    m_facets[b_f].segments = bot_segments;
//    m_facets[b_f].segments.push_back(i_e);
//    
//    m_segments_incident_facets.resize(m_segments.size());
//    for(uint32_t s : bot_segments)
//    {
//        if(0==m_segments_incident_facets[s].size())
//        {
//            continue;
//        }
//        for(uint32_t i=0; i<m_segments_incident_facets[s].size(); i++)
//        {
//            if(f==m_segments_incident_facets[s][i])
//            {
//                m_segments_incident_facets[s][i] = b_f;
//                break;
//            }
//        }
//    }
//    m_segments_incident_facets[i_e].push_back(f);
//    m_segments_incident_facets[i_e].push_back(b_f);
//    for(auto [new_s, ori_s] : split_segments)
//    {
//        m_segments_incident_facets[new_s] = m_segments_incident_facets[ori_s];
//        for(uint32_t i=0; i<m_segments_incident_facets[new_s].size(); i++)
//        {
//            if(f==m_segments_incident_facets[new_s][i])
//            {
//                m_segments_incident_facets[new_s][i] = b_f;
//                break;
//            }
//        }
//        
//        for(uint32_t n_f : m_segments_incident_facets[new_s])
//        {
//            if(b_f==n_f)
//            {
//                continue;
//            }
//            m_facets[n_f].segments.push_back(new_s);
//        }
//    }
//    
//    uint32_t p0 = m_facets[f].ip0;
//    uint32_t p1 = m_facets[f].ip1;
//    if(UNDEFINED_VALUE != p0)
//    {
//        if(m_polyhedrons[p0].end() == find(m_polyhedrons[p0].begin(), m_polyhedrons[p0].end(), f))
//        {
//            m_polyhedrons[p0].push_back(f);
//        }
//        if(m_polyhedrons[p0].end() == find(m_polyhedrons[p0].begin(), m_polyhedrons[p0].end(), b_f))
//        {
//            m_polyhedrons[p0].push_back(b_f);
//        }
//    }
//    if(UNDEFINED_VALUE != p1)
//    {
//        if(m_polyhedrons[p1].end() == find(m_polyhedrons[p1].begin(), m_polyhedrons[p1].end(), f))
//        {
//            m_polyhedrons[p1].push_back(f);
//        }
//        if(m_polyhedrons[p1].end() == find(m_polyhedrons[p1].begin(), m_polyhedrons[p1].end(), b_f))
//        {
//            m_polyhedrons[p1].push_back(b_f);
//        }
//    }
//    
////    for(uint32_t i=0; i<m_facets.size(); i++)
////    {
////        m_facets[i].get_sorted_vertices(m_segments);
////    }
////    
    return true;
}

std::vector<uint8_t> Polyhedralization::to_bytes()
{
    uint32_t explicit_count = m_vertices.size()-count_flat_vector_elements(m_inserted_vertices);
    vector<uint32_t> polyhedrons = nested_vector_to_flat_vector(m_polyhedrons);
    uint32_t facets_count = m_facets.size();
    uint32_t segments_count = m_segments.size();
    
    uint32_t buffer_size = 0;
    buffer_size += 4 + explicit_count*24;
    buffer_size += write_vector_to_byte_buffer_size(m_inserted_vertices);
    buffer_size += write_vector_to_byte_buffer_size(polyhedrons);
    buffer_size += 4;
    for(uint32_t i=0; i<facets_count; i++)
    {
        buffer_size += m_facets[i].write_to_byte_buffer_size();
    }
    buffer_size += 4 + segments_count*m_segments[0].write_to_byte_buffer_size();
    
    vector<uint8_t> buffer(buffer_size);
    uint32_t offset = 0;
    
    memcpy(buffer.data()+offset, &explicit_count, 4);
    offset+=4;
    for(uint32_t i=0; i<explicit_count; i++)
    {
        double3 p = approximate_vertex(m_vertices[i]);
        p.write_to_byte_buffer(buffer.data()+offset);
        offset += p.write_to_byte_buffer_size();
    }
    
    write_vector_to_byte_buffer(m_inserted_vertices, buffer.data()+offset);
    offset += write_vector_to_byte_buffer_size(m_inserted_vertices);
    write_vector_to_byte_buffer(polyhedrons, buffer.data()+offset);
    offset += write_vector_to_byte_buffer_size(polyhedrons);
    
    memcpy(buffer.data()+offset, &facets_count, 4);
    offset+=4;
    for(uint32_t i=0; i<facets_count; i++)
    {
        m_facets[i].write_to_byte_buffer(buffer.data()+offset);
        offset += m_facets[i].write_to_byte_buffer_size();
    }
    
    memcpy(buffer.data()+offset, &segments_count, 4);
    offset+=4;
    for(uint32_t i=0; i<segments_count; i++)
    {
        m_segments[i].write_to_byte_buffer(buffer.data()+offset);
        offset += m_segments[i].write_to_byte_buffer_size();
    }
    
    return buffer;
}

void Polyhedralization::clear_auxiliary_data()
{
    vector<uint32_t>().swap(m_slice_facets_cache);
    vector<std::tuple<uint32_t,uint32_t,uint32_t,uint32_t>>().swap(m_slice_segments_cache);
    vector<std::pair<uint32_t,int>>().swap(m_slice_vertices_cache);
    vector<std::vector<uint32_t>>().swap(m_segments_incident_facets);
}
