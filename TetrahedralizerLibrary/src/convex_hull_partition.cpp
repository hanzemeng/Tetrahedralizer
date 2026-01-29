#include "convex_hull_partition.hpp"
using namespace std;

Polyhedralization ConvexHullPartitionHandle::calculate(vector<shared_ptr<genericPoint>>& vertices, vector<uint32_t>& convex_hull, vector<uint32_t>& constraints)
{
    vector<double3> approximated_vertices;
    approximate_verteices(approximated_vertices, vertices);
    vector<vector<uint32_t>> coplanar_triangles;
    vector<double> coplanar_triangles_areas;
    auto add_coplanar_triangle = [&](uint32_t cg, uint32_t c0, uint32_t c1, uint32_t c2)
    {
        coplanar_triangles[cg].push_back(c0);
        coplanar_triangles[cg].push_back(c1);
        coplanar_triangles[cg].push_back(c2);
        double cur_area = fabs(barycentric_weight_denom(approximated_vertices[c0],approximated_vertices[c1],approximated_vertices[c2]));
        if(coplanar_triangles[cg].size() > 3)
        {
            if(cur_area > coplanar_triangles_areas[cg])
            {
                uint32_t n = coplanar_triangles[cg].size();
                swap(coplanar_triangles[cg][0], coplanar_triangles[cg][n-3]);
                swap(coplanar_triangles[cg][1], coplanar_triangles[cg][n-2]);
                swap(coplanar_triangles[cg][2], coplanar_triangles[cg][n-1]);
                coplanar_triangles_areas[cg] = cur_area;
            }
        }
        else
        {
            coplanar_triangles_areas[cg] = cur_area;
        }
    };
    
    vector<vector<uint32_t>> convex_hull_coplanar_triangles;
    {
        vector<uint32_t> convex_hull_neighbor = vector<uint32_t>(convex_hull.size(), UNDEFINED_VALUE);
        unordered_map<pair<uint32_t,uint32_t>, uint32_t, ii32_hash> segments_cache;
        for(uint32_t i=0; i<convex_hull.size()/3; i++)
        {
            uint32_t p0 = convex_hull[3*i+0];
            uint32_t p1 = convex_hull[3*i+1];
            uint32_t p2 = convex_hull[3*i+2];
            
            auto add_segment = [&](uint32_t i0, uint32_t i1, uint32_t index)
            {
                uint32_t n = search_int(i0,i1,segments_cache);
                if(UNDEFINED_VALUE == n)
                {
                    assign_int(i0,i1,index,segments_cache);
                }
                else
                {
                    convex_hull_neighbor[n] = index;
                    convex_hull_neighbor[index] = n;
                    remove_int(i0, i1, segments_cache);
                }
            };
            add_segment(p0, p1, 3*i+0);
            add_segment(p1, p2, 3*i+1);
            add_segment(p2, p0, 3*i+2);
        }
        vector<uint32_t> convex_hull_coplanar_groups = vector<uint32_t>(convex_hull.size()/3, UNDEFINED_VALUE);
        uint32_t coplanar_group_index = 0;
        for(uint32_t i=0; i<convex_hull_coplanar_groups.size(); i++)
        {
            if(UNDEFINED_VALUE != convex_hull_coplanar_groups[i])
            {
                continue;
            }
            
            convex_hull_coplanar_groups[i] = coplanar_group_index++;
            uint32_t c0 = convex_hull[3*i+0];
            uint32_t c1 = convex_hull[3*i+1];
            uint32_t c2 = convex_hull[3*i+2];
            
            queue<uint32_t> search_triangles;
            search_triangles.push(convex_hull_neighbor[3*i+0]/3);
            search_triangles.push(convex_hull_neighbor[3*i+1]/3);
            search_triangles.push(convex_hull_neighbor[3*i+2]/3);
            while(!search_triangles.empty())
            {
                uint32_t n = search_triangles.front();
                search_triangles.pop();
                if(UNDEFINED_VALUE != convex_hull_coplanar_groups[n])
                {
                    continue;
                }
                
                uint32_t nc0 = convex_hull[3*n+0];
                uint32_t nc1 = convex_hull[3*n+1];
                uint32_t nc2 = convex_hull[3*n+2];
                if(!is_coplanar(c0, c1, c2, nc0, nc1, nc2, vertices.data()))
                {
                    continue;
                }
                
                convex_hull_coplanar_groups[n] = convex_hull_coplanar_groups[i];
                search_triangles.push(convex_hull_neighbor[3*n+0]/3);
                search_triangles.push(convex_hull_neighbor[3*n+1]/3);
                search_triangles.push(convex_hull_neighbor[3*n+2]/3);
            }
        }
        
        convex_hull_coplanar_triangles.resize(coplanar_group_index);
        coplanar_triangles.resize(coplanar_group_index);
        coplanar_triangles_areas.resize(coplanar_group_index, -1.0);
        for(uint32_t i=0; i<convex_hull_coplanar_groups.size(); i++)
        {
            uint32_t c0 = convex_hull[3*i+0];
            uint32_t c1 = convex_hull[3*i+1];
            uint32_t c2 = convex_hull[3*i+2];
            uint32_t cg = convex_hull_coplanar_groups[i];
            convex_hull_coplanar_triangles[cg].push_back(c0);
            convex_hull_coplanar_triangles[cg].push_back(c1);
            convex_hull_coplanar_triangles[cg].push_back(c2);
            
            add_coplanar_triangle(cg, c0, c1, c2);
        }
    }
    
    vector<Segment> constraints_segments;
    vector<Facet> constraints_facets;
    {
        vector<uint32_t> convex_hull_coplanar_groups;
        unordered_map<tuple<uint64_t,uint64_t,uint64_t,uint64_t>,vector<uint32_t>, iiii64_hash> convex_hull_planes; // plane equation, list of coplanar group indexes
        for(uint32_t i=0; i<coplanar_triangles.size(); i++)
        {
            convex_hull_coplanar_groups.push_back(i);
            uint32_t c0 = coplanar_triangles[i][0];
            uint32_t c1 = coplanar_triangles[i][1];
            uint32_t c2 = coplanar_triangles[i][2];
            convex_hull_planes[get_plane_equation(approximated_vertices[c0], approximated_vertices[c1], approximated_vertices[c2])].push_back(i);
        }
        
        unordered_map<pair<uint32_t,uint32_t>, uint32_t, ii32_hash> constraints_segments_cache;
        auto [constraints_coplanar_triangles, constraints_planes] = group_coplanar_triangles(constraints, vertices, approximated_vertices);
        for(uint32_t i=0; i<constraints_coplanar_triangles.size(); i++)
        {
            uint32_t cg = UNDEFINED_VALUE;
            uint32_t c0 = constraints_coplanar_triangles[i][0];
            uint32_t c1 = constraints_coplanar_triangles[i][1];
            uint32_t c2 = constraints_coplanar_triangles[i][2];
            
            auto search_coplanar_group = [&](vector<uint32_t>& search_groups)
            {
                for(uint32_t j=0; j<search_groups.size(); j++)
                {
                    uint32_t ncg = search_groups[j];
                    uint32_t nc0 = coplanar_triangles[ncg][0];
                    uint32_t nc1 = coplanar_triangles[ncg][1];
                    uint32_t nc2 = coplanar_triangles[ncg][2];
                    
                    if(is_coplanar(c0, c1, c2, nc0, nc1, nc2, vertices.data()))
                    {
                        cg = ncg;
                        return;
                    }
                }
            };
            
            if(UNDEFINED_VALUE == constraints_planes[4*i+0] && UNDEFINED_VALUE == constraints_planes[4*i+1] && UNDEFINED_VALUE == constraints_planes[4*i+2] && UNDEFINED_VALUE == constraints_planes[4*i+3])
            {
                search_coplanar_group(convex_hull_coplanar_groups);
            }
            else
            {
                auto it = convex_hull_planes.find(make_tuple(constraints_planes[4*i+0],constraints_planes[4*i+1],constraints_planes[4*i+2],constraints_planes[4*i+3]));
                if(convex_hull_planes.end() != it)
                {
                    search_coplanar_group(it->second);
                }
                if(UNDEFINED_VALUE == cg)
                {
                    auto it = convex_hull_planes.find(make_tuple(UNDEFINED_VALUE,UNDEFINED_VALUE,UNDEFINED_VALUE,UNDEFINED_VALUE));
                    if(convex_hull_planes.end() != it)
                    {
                        search_coplanar_group(it->second);
                    }
                }
            }
            
            if(UNDEFINED_VALUE == cg)
            {
                cg = coplanar_triangles.size();
                coplanar_triangles.push_back(vector<uint32_t>());
                coplanar_triangles_areas.push_back(-1.0);
            }
            for(uint32_t j=0; j<constraints_coplanar_triangles[i].size()/3; j++)
            {
                uint32_t nc0 = constraints_coplanar_triangles[i][3*j+0];
                uint32_t nc1 = constraints_coplanar_triangles[i][3*j+1];
                uint32_t nc2 = constraints_coplanar_triangles[i][3*j+2];
                add_coplanar_triangle(cg, nc0, nc1, nc2);
            }
            
            if(cg>=convex_hull_coplanar_groups.size())
            {
                auto add_segment = [&](uint32_t i0, uint32_t i1) -> uint32_t
                {
                    uint32_t s = search_int(i0,i1,constraints_segments_cache);
                    if(UNDEFINED_VALUE == s)
                    {
                        s = constraints_segments.size();
                        constraints_segments.push_back(Segment(i0,i1));
                        assign_int(i0, i1, s, constraints_segments_cache);
                    }
                    return s;
                };
                
                for(uint32_t j=0; j<constraints_coplanar_triangles[i].size()/3; j++)
                {
                    uint32_t nc0 = constraints_coplanar_triangles[i][3*j+0];
                    uint32_t nc1 = constraints_coplanar_triangles[i][3*j+1];
                    uint32_t nc2 = constraints_coplanar_triangles[i][3*j+2];
                    
                    uint32_t s0 = add_segment(nc0, nc1);
                    uint32_t s1 = add_segment(nc1, nc2);
                    uint32_t s2 = add_segment(nc2, nc0);
                    uint32_t cg0 = coplanar_triangles[cg][0];
                    uint32_t cg1 = coplanar_triangles[cg][1];
                    uint32_t cg2 = coplanar_triangles[cg][2];
                    constraints_facets.push_back(Facet(s0, s1, s2, cg0, cg1, cg2, cg));
                }
            }
        }
    }
    
    Polyhedralization polyhedralization;
    polyhedralization.m_vertices = vertices;
    polyhedralization.m_polyhedrons.push_back(vector<uint32_t>());
    {
        unordered_map<pair<uint32_t,uint32_t>, uint32_t, ii32_hash> segments_cache;
        for(uint32_t i=0; i<convex_hull_coplanar_triangles.size(); i++)
        {
            unordered_map<pair<uint32_t,uint32_t>, uint32_t, ii32_hash> segments_occurrences;
            for(uint32_t j=0; j<convex_hull_coplanar_triangles[i].size()/3; j++)
            {
                uint32_t c0 = convex_hull_coplanar_triangles[i][3*j+0];
                uint32_t c1 = convex_hull_coplanar_triangles[i][3*j+1];
                uint32_t c2 = convex_hull_coplanar_triangles[i][3*j+2];
                
                auto add_segment = [&](uint32_t i0, uint32_t i1)
                {
                    uint32_t cnt = search_int(i0,i1,segments_occurrences);
                    if(UNDEFINED_VALUE == cnt)
                    {
                        assign_int(i0, i1, 1, segments_occurrences);
                    }
                    else
                    {
                        assign_int(i0, i1, cnt+1, segments_occurrences);
                    }
                };
                add_segment(c0, c1);
                add_segment(c1, c2);
                add_segment(c2, c0);
            }
            
            uint32_t cg0 = coplanar_triangles[i][0];
            uint32_t cg1 = coplanar_triangles[i][1];
            uint32_t cg2 = coplanar_triangles[i][2];
            uint32_t f = polyhedralization.m_facets.size();
            polyhedralization.m_facets.push_back(Facet(UNDEFINED_VALUE,UNDEFINED_VALUE,UNDEFINED_VALUE,cg0,cg1,cg2,0,UNDEFINED_VALUE));
            polyhedralization.m_facets[f].segments.clear();
            for(auto [k,v] : segments_occurrences)
            {
                if(1 != v)
                {
                    continue;
                }
                
                auto add_segment = [&](uint32_t i0, uint32_t i1) -> uint32_t
                {
                    uint32_t s = search_int(i0,i1,segments_cache);
                    if(UNDEFINED_VALUE == s)
                    {
                        s = polyhedralization.m_segments.size();
                        polyhedralization.m_segments.push_back(Segment(i0,i1));
                        assign_int(i0, i1, s, segments_cache);
                    }
                    return s;
                };
                polyhedralization.m_facets[f].segments.push_back(add_segment(k.first, k.second));
            }
            polyhedralization.m_polyhedrons[0].push_back(f);
        }
    }
    
    vector<uint32_t> facets_order;
    if(0 != constraints_facets.size())
    {
        facets_order = order_facets(vertices, approximated_vertices, constraints_segments, constraints_facets);
    }
    
    queue<pair<uint32_t, uint32_t>> slice_order; // polyhedron, facets order index
    if(!facets_order.empty())
    {
        slice_order.push(make_pair(0, 0));
    }
    while(!slice_order.empty())
    {
        auto [p, i] = slice_order.front();
        slice_order.pop();
        if(UNDEFINED_VALUE == i)
        {
            continue;
        }
        uint32_t cg0 = coplanar_triangles[facets_order[i]][0];
        uint32_t cg1 = coplanar_triangles[facets_order[i]][1];
        uint32_t cg2 = coplanar_triangles[facets_order[i]][2];
        
        int slice_res = polyhedralization.slice_polyhedron_with_plane(p, cg0, cg1, cg2);
        if(1 == slice_res)
        {
            slice_order.push(make_pair(p, facets_order[i+1]));
        }
        else if(-1 == slice_res)
        {
            slice_order.push(make_pair(p, facets_order[i+2]));
        }
        else
        {
            slice_order.push(make_pair(p, facets_order[i+1]));
            slice_order.push(make_pair(polyhedralization.m_polyhedrons.size()-1, facets_order[i+2]));
        }
    }
    
    approximate_verteices(approximated_vertices, polyhedralization.m_vertices);
    for(uint32_t i=0; i<polyhedralization.m_facets.size(); i++)
    {
        polyhedralization.m_facets[i].calculate_implicit_centroid(approximated_vertices, polyhedralization.m_segments);
    }
    
    return polyhedralization;
}


//extern "C" LIBRARY_EXPORT void* CreateConvexHullPartitionHandle()
//{
//    return new ConvexHullPartitionHandle();
//}
//extern "C" LIBRARY_EXPORT void DisposeConvexHullPartitionHandle(void* handle)
//{
//    ((ConvexHullPartitionHandle*)handle)->Dispose();
//    delete (ConvexHullPartitionHandle*)handle;
//}
//
//extern "C" LIBRARY_EXPORT void AddConvexHullPartitionInput(void* handle, uint32_t explicit_count, double* explicit_values, uint32_t implicit_count, uint32_t* implicit_values,
//                                                           uint32_t tetrahedrons_count, uint32_t* tetrahedrons, uint32_t constraints_facets_count, FacetInteropData* constraints_facets,
//                                                           uint32_t segments_count, SegmentInteropData* segments, uint32_t coplanar_triangles_count, uint32_t* coplanar_triangles)
//{
//    ((ConvexHullPartitionHandle*)handle)->AddInput(explicit_count, explicit_values, implicit_count, implicit_values,
//                                                   tetrahedrons_count, tetrahedrons, constraints_facets_count, constraints_facets,
//                                                   segments_count, segments, coplanar_triangles_count, coplanar_triangles);
//}
//
//extern "C" LIBRARY_EXPORT void CalculateConvexHullPartition(void* handle)
//{
//    ((ConvexHullPartitionHandle*)handle)->Calculate();
//}
//
//extern "C" LIBRARY_EXPORT uint32_t GetConvexHullPartitionInsertedVerticesCount(void* handle)
//{
//    return ((ConvexHullPartitionHandle*)handle)->GetInsertedVerticesCount();
//}
//extern "C" LIBRARY_EXPORT void GetConvexHullPartitionInsertedVertices(void* handle, uint32_t* out)
//{
//    ((ConvexHullPartitionHandle*)handle)->GetInsertedVertices(out);
//}
//
//extern "C" LIBRARY_EXPORT uint32_t GetConvexHullPartitionPolyhedronsCount(void* handle)
//{
//    return ((ConvexHullPartitionHandle*)handle)->GetPolyhedronsCount();
//}
//extern "C" LIBRARY_EXPORT void GetConvexHullPartitionPolyhedrons(void* handle, uint32_t* out)
//{
//    ((ConvexHullPartitionHandle*)handle)->GetPolyhedrons(out);
//}
//
//extern "C" LIBRARY_EXPORT uint32_t GetConvexHullPartitionFacetsCount(void* handle)
//{
//    return ((ConvexHullPartitionHandle*)handle)->GetFacetsCount();
//}
//extern "C" LIBRARY_EXPORT void GetConvexHullPartitionFacets(void* handle, FacetInteropData* out)
//{
//    ((ConvexHullPartitionHandle*)handle)->GetFacets(out);
//}
//extern "C" LIBRARY_EXPORT uint32_t GetConvexHullPartitionSegmentsCount(void* handle)
//{
//    return ((ConvexHullPartitionHandle*)handle)->GetSegmentsCount();
//}
//extern "C" LIBRARY_EXPORT void GetConvexHullPartitionSegments(void* handle, SegmentInteropData* out)
//{
//    ((ConvexHullPartitionHandle*)handle)->GetSegments(out);
//}
