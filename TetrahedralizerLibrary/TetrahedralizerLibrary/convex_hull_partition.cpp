#include "convex_hull_partition.hpp"
using namespace std;

void ConvexHullPartitionHandle::Dispose()
{}
void ConvexHullPartitionHandle::AddInput(uint32_t explicit_count, double* explicit_values, uint32_t implicit_count, uint32_t* implicit_values, uint32_t convex_hull_facets_count, FacetInteropData* convex_hull_facets, uint32_t constraints_facets_count, FacetInteropData* constraints_facets, uint32_t segments_count, SegmentInteropData* segments)
{
    m_vertices = create_vertices(explicit_count, explicit_values, implicit_count, implicit_values);
    m_polyhedrons.push_back(vector<uint32_t>());
    for(uint32_t i=0; i<convex_hull_facets_count; i++)
    {
        m_polyhedrons[0].push_back(i);
        m_facets.push_back(convex_hull_facets[i].to_facet());
        m_facets[i].ip0 = 0;
        m_facets[i].ip1 = UNDEFINED_VALUE;
    }
    for(uint32_t i=0; i<constraints_facets_count; i++)
    {
        m_constraints_facets.push_back(constraints_facets[i].to_facet());
    }
    for(uint32_t i=0; i<segments_count; i++)
    {
        m_segments.push_back(segments[i].to_segment());
    }
}
void ConvexHullPartitionHandle::Calculate()
{
    convex_hull_partition();
}

uint32_t ConvexHullPartitionHandle::GetInsertedVerticesCount()
{
    return m_inserted_vertices_count;
}
uint32_t* ConvexHullPartitionHandle::GetInsertedVertices()
{
    return m_inserted_vertices.data();
}

uint32_t ConvexHullPartitionHandle::GetPolyhedronsCount()
{
    return m_output_polyhedrons_count;
}
uint32_t* ConvexHullPartitionHandle::GetPolyhedrons()
{
    return m_output_polyhedrons.data();
}

uint32_t ConvexHullPartitionHandle::GetFacetsCount()
{
    return m_facets.size();
}
void ConvexHullPartitionHandle::GetFacets(FacetInteropData* outArray)
{
    for(uint32_t i=0; i<m_facets.size(); i++)
    {
        outArray[i] = m_facets[i];
    }
}
uint32_t ConvexHullPartitionHandle::GetSegmentsCount()
{
    return m_segments.size();
}
void ConvexHullPartitionHandle::GetSegments(SegmentInteropData* outArray)
{
    for(uint32_t i=0; i<m_segments.size(); i++)
    {
        outArray[i] = m_segments[i];
    }
}

extern "C" LIBRARY_EXPORT void* CreateConvexHullPartitionHandle()
{
    return new ConvexHullPartitionHandle();
}
extern "C" LIBRARY_EXPORT void DisposeConvexHullPartitionHandle(void* handle)
{
    ((ConvexHullPartitionHandle*)handle)->Dispose();
    delete (ConvexHullPartitionHandle*)handle;
}

extern "C" LIBRARY_EXPORT void AddConvexHullPartitionInput(void* handle, uint32_t explicit_count, double* explicit_values, uint32_t implicit_count, uint32_t* implicit_values, uint32_t convex_hull_facets_count, FacetInteropData* convex_hull_facets, uint32_t constraints_facets_count, FacetInteropData* constraints_facets, uint32_t segments_count, SegmentInteropData* segments)
{
    ((ConvexHullPartitionHandle*)handle)->AddInput(explicit_count, explicit_values, implicit_count, implicit_values, convex_hull_facets_count, convex_hull_facets, constraints_facets_count, constraints_facets, segments_count, segments);
}

extern "C" LIBRARY_EXPORT void CalculateConvexHullPartition(void* handle)
{
    ((ConvexHullPartitionHandle*)handle)->Calculate();
}

extern "C" LIBRARY_EXPORT uint32_t GetConvexHullPartitionInsertedVerticesCount(void* handle)
{
    return ((ConvexHullPartitionHandle*)handle)->GetInsertedVerticesCount();
}
extern "C" LIBRARY_EXPORT uint32_t* GetConvexHullPartitionInsertedVertices(void* handle)
{
    return  ((ConvexHullPartitionHandle*)handle)->GetInsertedVertices();
}

extern "C" LIBRARY_EXPORT uint32_t GetConvexHullPartitionPolyhedronsCount(void* handle)
{
    return ((ConvexHullPartitionHandle*)handle)->GetPolyhedronsCount();
}
extern "C" LIBRARY_EXPORT uint32_t* GetConvexHullPartitionPolyhedrons(void* handle)
{
    return ((ConvexHullPartitionHandle*)handle)->GetPolyhedrons();
}

extern "C" LIBRARY_EXPORT uint32_t GetConvexHullPartitionFacetsCount(void* handle)
{
    return ((ConvexHullPartitionHandle*)handle)->GetFacetsCount();
}
extern "C" LIBRARY_EXPORT void GetConvexHullPartitionFacets(void* handle, FacetInteropData* outArray)
{
    ((ConvexHullPartitionHandle*)handle)->GetFacets(outArray);
}
extern "C" LIBRARY_EXPORT uint32_t GetConvexHullPartitionSegmentsCount(void* handle)
{
    return ((ConvexHullPartitionHandle*)handle)->GetSegmentsCount();
}
extern "C" LIBRARY_EXPORT void GetConvexHullPartitionSegments(void* handle, SegmentInteropData* outArray)
{
    ((ConvexHullPartitionHandle*)handle)->GetSegments(outArray);
}


void ConvexHullPartitionHandle::convex_hull_partition()
{
    // group coplanar constrains together
    vector<unordered_set<uint32_t>> coplanar_constraints;
    {
        vector<uint32_t> constraints_to_coplanar_group_mapping = vector<uint32_t>(m_constraints_facets.size(),UNDEFINED_VALUE);
        for(uint32_t i=0; i<m_constraints_facets.size(); i++)
        {
            m_u_set_i_0.clear(); // checked constraints group
            for(uint32_t j=0; j<i; j++)
            {
                if(m_u_set_i_0.end() != m_u_set_i_0.find(constraints_to_coplanar_group_mapping[j]))
                {
                    continue;
                }
                m_u_set_i_0.insert(constraints_to_coplanar_group_mapping[j]);
                
                if(m_constraints_facets[i].is_coplanar_constraint(m_constraints_facets[j].p0, m_constraints_facets[j].p1, m_constraints_facets[j].p2, m_vertices))
                {
                    constraints_to_coplanar_group_mapping[i] = constraints_to_coplanar_group_mapping[j];
                    coplanar_constraints[constraints_to_coplanar_group_mapping[j]].insert(m_constraints_facets[i].constrains.begin(),m_constraints_facets[i].constrains.end());
                    break;
                }
            }
            if(UNDEFINED_VALUE == constraints_to_coplanar_group_mapping[i])
            {
                constraints_to_coplanar_group_mapping[i] = coplanar_constraints.size();
                coplanar_constraints.push_back(unordered_set<uint32_t>(m_constraints_facets[i].constrains.begin(),m_constraints_facets[i].constrains.end()));
                m_coplanar_constraints_planes.push_back(m_constraints_facets[i].p0);
                m_coplanar_constraints_planes.push_back(m_constraints_facets[i].p1);
                m_coplanar_constraints_planes.push_back(m_constraints_facets[i].p2);
            }
            m_constraints_facets[i].ip0 = constraints_to_coplanar_group_mapping[i]; // ip0 suppose to be incident polyhedron, but here it is incident coplanar constraints
        }
    }
    
    // calculate slice order
    stack<pair<uint32_t,uint32_t>> polyhedrons_slice_order;
    {
        if(0 != m_constraints_facets.size())
        {
            vector<shared_ptr<genericPoint>> vertices = m_vertices;
            vector<Segment> segments = m_segments;
            vector<Facet> facets = m_constraints_facets;
            vector<uint32_t> facets_indexes;
            for(uint32_t i=0; i<facets.size(); i++)
            {
                facets_indexes.push_back(i);
                facets[i].constrains.clear(); // make duplication faster
            }
            unordered_map<uint32_t, unordered_map<uint32_t, int>> constraints_facets_cache;
            unordered_map<uint32_t, unordered_map<uint32_t, int>> constraints_vertices_cache;
            polyhedrons_slice_order.push(make_pair(0, build_polyhedrons_slice_tree(facets_indexes, vertices, segments, facets, constraints_facets_cache, constraints_vertices_cache)));
        }
    }
    
    m_inserted_vertices_count = 0;
    // slice polyhedrons
    while(!polyhedrons_slice_order.empty())
    {
        uint32_t p = polyhedrons_slice_order.top().first;
        PolyhedronConstraint& polyhedronConstraint = m_polyhedrons_slice_tree[polyhedrons_slice_order.top().second];
        polyhedrons_slice_order.pop();
        uint32_t c = polyhedronConstraint.c;
        uint32_t c0 = m_coplanar_constraints_planes[3*c+0];
        uint32_t c1 = m_coplanar_constraints_planes[3*c+1];
        uint32_t c2 = m_coplanar_constraints_planes[3*c+2];
        
        m_vector_i_0.clear(); // top polyhedron's facets
        m_vector_i_1.clear(); // bottom polyhedron's facets
        m_u_set_i_0.clear(); // edges on the constraint
        m_u_map_i_iii_0.clear(); // keys are processed edges, values are (intersection vertex, top edge, bottom edge)
        m_u_map_i_si_0.clear(); // orientation cache
        for(uint32_t f : m_polyhedrons[p])
        {
            m_vector_i_2.clear(); // edges on top of the constraint
            m_vector_i_3.clear(); // edges on bottom of the constraint
            bool has_edge_on_constraint = false;
            uint32_t i0(UNDEFINED_VALUE), i1(UNDEFINED_VALUE);
            for(uint32_t e : m_facets[f].segments)
            {
                if(m_u_map_i_iii_0.end() == m_u_map_i_iii_0.find(e))
                {
                    auto [i_p,top_e,bot_e,vs] = Segment::slice_segment_with_plane(e, c0, c1, c2, m_vertices, m_segments, m_u_map_i_si_0);
                    m_u_map_i_iii_0[e] = make_tuple(i_p,top_e,bot_e);
                    if(0 != vs.size())
                    {
                        m_inserted_vertices_count++;
                        m_inserted_vertices.push_back(vs.size());
                        for(uint32_t i=0; i<vs.size(); i++)
                        {
                            m_inserted_vertices.push_back(vs[i]);
                        }
                    }
                }
                
                auto [i_p,top_e,bottom_e] = m_u_map_i_iii_0[e];
                if(UNDEFINED_VALUE == i_p && top_e == UNDEFINED_VALUE && bottom_e == UNDEFINED_VALUE)
                {
                    m_u_set_i_0.insert(e); // e is on the constraint
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
                if(UNDEFINED_VALUE != top_e)
                {
                    m_vector_i_2.push_back(top_e);
                }
                if(UNDEFINED_VALUE != bottom_e)
                {
                    m_vector_i_3.push_back(bottom_e);
                }
            }
            
            if(has_edge_on_constraint || i1 == UNDEFINED_VALUE) // constraint intersects on one or more edges of the facet or does not intersect the facet
            {
                if(!m_vector_i_2.empty())
                {
                    m_vector_i_0.push_back(f);
                }
                else if(!m_vector_i_3.empty())
                {
                    m_vector_i_1.push_back(f);
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
                m_u_set_i_0.insert(i_e);
                
                m_facets[f].segments.clear();
                uint32_t bottom_f = m_facets.size();
                m_facets.push_back(Facet(m_facets[f]));
                m_facets[f].segments = m_vector_i_2;
                m_facets[f].segments.push_back(i_e);
                m_facets[bottom_f].segments = m_vector_i_3;
                m_facets[bottom_f].segments.push_back(i_e);
                m_vector_i_0.push_back(f);
                m_vector_i_1.push_back(bottom_f);
            }
        }
        
        // constraint does not slice the polyhedron
        if(0 == m_vector_i_0.size() || 0 == m_vector_i_1.size())
        {
            if(0 != m_vector_i_0.size() && UNDEFINED_VALUE != polyhedronConstraint.top)
            {
                polyhedrons_slice_order.push(make_pair(p, polyhedronConstraint.top));
            }
            if(0 != m_vector_i_1.size() && UNDEFINED_VALUE != polyhedronConstraint.bot)
            {
                polyhedrons_slice_order.push(make_pair(p, polyhedronConstraint.bot));
            }
            continue;
        }
//        if(m_vector_i_0.size()<3 || m_vector_i_1.size()<3 || m_u_set_i_0.size()<3)
//        {
//            throw "wtf";
//        }
        
        uint32_t bottom_polyhedron = m_polyhedrons.size();
        m_polyhedrons.push_back(vector<uint32_t>());
        
        uint32_t common_facet = m_facets.size();
        m_facets.push_back(Facet());
        m_facets[common_facet].p0 = c0;
        m_facets[common_facet].p1 = c1;
        m_facets[common_facet].p2 = c2;
        m_facets[common_facet].ip0 = p;
        m_facets[common_facet].ip1 = bottom_polyhedron;
        unordered_set<uint32_t>& cc = coplanar_constraints[c];
        m_facets[common_facet].constrains = vector<uint32_t>(cc.begin(),cc.end());
        for(uint32_t e : m_u_set_i_0)
        {
            m_facets[common_facet].segments.push_back(e);
        }
        
        m_polyhedrons[p] = m_vector_i_0;
        m_polyhedrons[p].push_back(common_facet);
        m_polyhedrons[bottom_polyhedron] = m_vector_i_1;
        m_polyhedrons[bottom_polyhedron].push_back(common_facet);
        
        for(uint32_t f : m_polyhedrons[bottom_polyhedron])
        {
            if(f == common_facet)
            {
                continue;
            }
            uint32_t n;
            if(p == m_facets[f].ip0)
            {
                m_facets[f].ip0 = bottom_polyhedron;
                n = m_facets[f].ip1;
            }
            else
            {
                m_facets[f].ip1 = bottom_polyhedron;
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
        
        for(auto [k, v] : m_u_map_i_iii_0)
        {
            uint32_t original_e = k;
            uint32_t i_e = get<0>(v);
            uint32_t top_e = get<1>(v);
            uint32_t bottom_e = get<2>(v);
            
            if(UNDEFINED_VALUE == i_e || UNDEFINED_VALUE == top_e || UNDEFINED_VALUE == bottom_e)
            {
                continue;
            }
            
            if(original_e == bottom_e)
            {
                swap(top_e,bottom_e);
            }
            m_u_set_i_0.clear(); // visited polyhedrons
            m_u_set_i_0.insert(UNDEFINED_VALUE);
            clear_queue(m_queue_i_0); // polyhedrons to be visited
            m_queue_i_0.push(p);
            while(!m_queue_i_0.empty())
            {
                uint32_t cur = m_queue_i_0.front();
                m_queue_i_0.pop();
                if(m_u_set_i_0.end() != m_u_set_i_0.find(cur))
                {
                    continue;
                }
                m_u_set_i_0.insert(cur);
                
                bool search_neighbor = false;
                for(uint32_t f : m_polyhedrons[cur])
                {
                    if(m_facets[f].contains_segment(original_e))
                    {
                        search_neighbor = true;
                        if(m_polyhedrons[p].end() != find(m_polyhedrons[p].begin(), m_polyhedrons[p].end(), f))
                        {
                            continue;
                        }
                        if(m_polyhedrons[bottom_polyhedron].end() != find(m_polyhedrons[bottom_polyhedron].begin(), m_polyhedrons[bottom_polyhedron].end(), f))
                        {
                            continue;
                        }
                        if(!m_facets[f].contains_segment(bottom_e))
                        {
                            m_facets[f].segments.push_back(bottom_e);
                        }
                    }
                }
                
                if(search_neighbor)
                {
                    for(uint32_t f : m_polyhedrons[cur])
                    {
                        if(cur == m_facets[f].ip0)
                        {
                            m_queue_i_0.push(m_facets[f].ip1);
                        }
                        else
                        {
                            m_queue_i_0.push(m_facets[f].ip0);
                        }
                    }
                }
            }
        }
        
        if(UNDEFINED_VALUE != polyhedronConstraint.top)
        {
            polyhedrons_slice_order.push(make_pair(p, polyhedronConstraint.top));
        }
        if(UNDEFINED_VALUE != polyhedronConstraint.bot)
        {
            polyhedrons_slice_order.push(make_pair(bottom_polyhedron, polyhedronConstraint.bot));
        }
    }
    
    // prepare output and calculate more facet info
    {
        m_output_polyhedrons_count = m_polyhedrons.size();
        for(uint32_t i=0; i<m_polyhedrons.size(); i++)
        {
            m_output_polyhedrons.push_back(m_polyhedrons[i].size());
            for(uint32_t j=0; j<m_polyhedrons[i].size(); j++)
            {
                m_output_polyhedrons.push_back(m_polyhedrons[i][j]);
            }
        }
        for(uint32_t i=0; i<m_facets.size(); i++)
        {
            vector<uint32_t> vs = m_facets[i].get_sorted_vertices(m_segments);
            double3 centroid = approximate_facet_centroid(vs, m_vertices.data());
            double3 weight;
            barycentric_weight(m_facets[i].p0,m_facets[i].p1,m_facets[i].p2,centroid,m_vertices.data(), weight);
            m_facets[i].w0 = weight.x;
            m_facets[i].w1 = weight.y;
        }
    }
}

// facets are all facets (ip0 is the coplanar group), facets_indexes are the facets that need to be checked
uint32_t ConvexHullPartitionHandle::build_polyhedrons_slice_tree(std::vector<uint32_t>& facets_indexes, std::vector<std::shared_ptr<genericPoint>>& vertices, std::vector<Segment>& segments, std::vector<Facet>& facets,
                                                                 std::unordered_map<uint32_t, std::unordered_map<uint32_t, int>>& constraints_facets_cache,
                                                                 std::unordered_map<uint32_t, std::unordered_map<uint32_t, int>>& constraints_vertices_cache)
{
    if(facets_indexes.empty())
    {
        return UNDEFINED_VALUE;
    }

    uint32_t res = m_polyhedrons_slice_tree.size();
    m_polyhedrons_slice_tree.push_back(PolyhedronConstraint());
    
    uint32_t best_i = UNDEFINED_VALUE;
    vector<uint32_t> best_top;
    vector<uint32_t> best_bot;
    vector<uint32_t> best_both;
    for(uint32_t i=0; i<facets_indexes.size(); i++)
    {
        uint32_t c = facets_indexes[i];
        uint32_t cg = facets[c].ip0;
        uint32_t c0 = m_coplanar_constraints_planes[3*cg+0];
        uint32_t c1 = m_coplanar_constraints_planes[3*cg+1];
        uint32_t c2 = m_coplanar_constraints_planes[3*cg+2];
        
        if(constraints_facets_cache.end() == constraints_facets_cache.find(cg))
        {
            constraints_facets_cache[cg] = unordered_map<uint32_t, int>();
            constraints_vertices_cache[cg] = unordered_map<uint32_t, int>();
        }
        
        vector<uint32_t> top;
        vector<uint32_t> bot;
        vector<uint32_t> both;
        for(uint32_t j=0; j<facets_indexes.size(); j++)
        {
            if(i==j)
            {
                continue;
            }
            uint32_t nc = facets_indexes[j];
            uint32_t ncg = facets[nc].ip0;
            
            if(constraints_facets_cache[cg].end() == constraints_facets_cache[cg].find(nc))
            {
                if(cg == ncg)
                {
                    constraints_facets_cache[cg][nc] = 69; // coplanar
                }
                else
                {
                    bool has_top = false;
                    bool has_bot = false;
                    vector<uint32_t> vs = facets[nc].get_vertices(segments);
                    for(uint32_t v : vs)
                    {
                        if(constraints_vertices_cache[cg].end() == constraints_vertices_cache[cg].find(v))
                        {
                            constraints_vertices_cache[cg][v] = orient3d(c0,c1,c2,v,vertices.data());
                        }
                        
                        has_top |= 1==constraints_vertices_cache[cg][v];
                        has_bot |= -1==constraints_vertices_cache[cg][v];
                        if(has_top && has_bot)
                        {
                            break;
                        }
                    }
                    if(has_top && has_bot)
                    {
                        constraints_facets_cache[cg][nc] = 0;
                    }
                    else if(has_top && !has_bot)
                    {
                        constraints_facets_cache[cg][nc] = 1;
                    }
                    else if(!has_top && has_bot)
                    {
                        constraints_facets_cache[cg][nc] = -1;
                    }
                    else
                    {
                        constraints_facets_cache[cg][nc] = 69; // coplanar, should not happen
                    }
                }
            }
            if(0 == constraints_facets_cache[cg][nc])
            {
                both.push_back(nc);
            }
            else if(1 == constraints_facets_cache[cg][nc])
            {
                top.push_back(nc);
            }
            else if(-1 == constraints_facets_cache[cg][nc])
            {
                bot.push_back(nc);
            }
            if(UNDEFINED_VALUE!=best_i && both.size()>=best_both.size())
            {
                break;
            }
        }
        if(UNDEFINED_VALUE==best_i || best_both.size() > both.size())
        {
            best_i = i;
            best_top = std::move(top);
            best_bot = std::move(bot);
            best_both = std::move(both);
            if(0 == best_both.size())
            {
                break;
            }
        }
    }
    
    uint32_t c = facets_indexes[best_i];
    uint32_t cg = facets[c].ip0;
    uint32_t c0 = m_coplanar_constraints_planes[3*cg+0];
    uint32_t c1 = m_coplanar_constraints_planes[3*cg+1];
    uint32_t c2 = m_coplanar_constraints_planes[3*cg+2];
    m_polyhedrons_slice_tree[res].c = cg;
    
    m_u_map_i_iii_0.clear(); // stores sliced segments
    for(uint32_t nc : best_both)
    {
        uint32_t noc = facets[nc].ip0;
        uint32_t nc0 = m_coplanar_constraints_planes[3*noc+0];
        uint32_t nc1 = m_coplanar_constraints_planes[3*noc+1];
        uint32_t nc2 = m_coplanar_constraints_planes[3*noc+2];
        
        uint32_t top_facet = facets.size();
        facets.push_back(Facet(facets[nc]));
        facets[top_facet].segments.clear();
        uint32_t bot_facet = facets.size();
        facets.push_back(Facet(facets[top_facet]));
        
        uint32_t i0(UNDEFINED_VALUE), i1(UNDEFINED_VALUE);
        for(uint32_t s : facets[nc].segments)
        {
            if(m_u_map_i_iii_0.end() == m_u_map_i_iii_0.find(s))
            {
                auto [i_p,top_e,bot_e,vs] = Segment::slice_segment_with_plane(s, c0, c1, c2, vertices, segments, constraints_vertices_cache[cg], false);
                m_u_map_i_iii_0[s] = make_tuple(i_p,top_e,bot_e);
            }
            auto [i_p,top_e,bot_e] = m_u_map_i_iii_0[s];
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
            if(UNDEFINED_VALUE != top_e)
            {
                facets[top_facet].segments.push_back(top_e);
            }
            if(UNDEFINED_VALUE != bot_e)
            {
                facets[bot_facet].segments.push_back(bot_e);
            }
        }
        
        uint32_t i_e = segments.size();
        segments.push_back(Segment(i0,i1,nc0,nc1,nc2,c0,c1,c2));
        facets[top_facet].segments.push_back(i_e);
        facets[bot_facet].segments.push_back(i_e);
        best_top.push_back(top_facet);
        best_bot.push_back(bot_facet);
    }
    
    m_polyhedrons_slice_tree[res].top = build_polyhedrons_slice_tree(best_top, vertices, segments, facets, constraints_facets_cache, constraints_vertices_cache);
    m_polyhedrons_slice_tree[res].bot = build_polyhedrons_slice_tree(best_bot, vertices, segments, facets, constraints_facets_cache, constraints_vertices_cache);
    return res;
}
