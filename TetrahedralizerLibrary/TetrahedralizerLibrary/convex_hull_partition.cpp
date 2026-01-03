#include "convex_hull_partition.hpp"
using namespace std;

void ConvexHullPartitionHandle::Dispose()
{}
void ConvexHullPartitionHandle::AddInput(uint32_t explicit_count, double* explicit_values, uint32_t implicit_count, uint32_t* implicit_values,
                                         uint32_t tetrahedrons_count, uint32_t* tetrahedrons, uint32_t constraints_facets_count, FacetInteropData* constraints_facets,
                                         uint32_t segments_count, SegmentInteropData* segments, uint32_t coplanar_triangles_count, uint32_t* coplanar_triangles)
{
    m_polyhedralization.m_vertices = create_vertices(explicit_count, explicit_values, implicit_count, implicit_values);
    approximate_verteices(m_approximated_vertices, m_polyhedralization.m_vertices);
    for(uint32_t i=0; i<segments_count; i++)
    {
        m_constraints_segments.push_back(segments[i].to_segment());
    }
    for(uint32_t i=0; i<constraints_facets_count; i++)
    {
        m_constraints_facets.push_back(constraints_facets[i].to_facet());
    }
    
    vector<vector<uint32_t>> coplanar_groups_triangles = flat_array_to_nested_vector(coplanar_triangles, coplanar_triangles_count);
    for(uint32_t i=0; i<coplanar_groups_triangles.size(); i++)
    {
        for(uint32_t j=0; j<coplanar_groups_triangles[i].size()/3; j++)
        {
            uint32_t c0 = coplanar_groups_triangles[i][3*j+0];
            uint32_t c1 = coplanar_groups_triangles[i][3*j+1];
            uint32_t c2 = coplanar_groups_triangles[i][3*j+2];
            sort_ints(c0, c1, c2);
            m_triangles_coplanar_groups[make_tuple(c0,c1,c2)] = i;
        }
    }
    
    m_polyhedralization.m_polyhedrons.push_back(vector<uint32_t>());
    Tetrahedralization tetrahedralization;
    tetrahedralization.assign_tetrahedrons(tetrahedrons, tetrahedrons_count);
    vector<uint32_t> convex_hull = tetrahedralization.get_bounding_facets();
    unordered_map<pair<uint32_t,uint32_t>, uint32_t, ii32_hash> segments_cache;
    for(uint32_t i=0; i<convex_hull.size()/3; i++)
    {
        uint32_t p0 = convex_hull[3*i+0];
        uint32_t p1 = convex_hull[3*i+1];
        uint32_t p2 = convex_hull[3*i+2];

        auto add_segment = [&](uint32_t i0, uint32_t i1) -> uint32_t
        {
            sort_ints(i0, i1);
            auto it = segments_cache.find(make_pair(i0, i1));
            if(segments_cache.end() == it)
            {
                uint32_t res = m_polyhedralization.m_segments.size();
                m_polyhedralization.m_segments.push_back(Segment(i0,i1));
                segments_cache[make_pair(i0, i1)] = res;
                return res;
            }
            return it->second;
        };
        uint32_t s0 = add_segment(p0,p1);
        uint32_t s1 = add_segment(p1,p2);
        uint32_t s2 = add_segment(p2,p0);
        
        uint32_t cg = search_int(p0, p1, p2, m_triangles_coplanar_groups);
        uint32_t cc0 = coplanar_groups_triangles[cg][0];
        uint32_t cc1 = coplanar_groups_triangles[cg][1];
        uint32_t cc2 = coplanar_groups_triangles[cg][2];
        uint32_t f = m_polyhedralization.m_facets.size();
        m_polyhedralization.m_facets.push_back(Facet(s0,s1,s2,cc0,cc1,cc2,0,UNDEFINED_VALUE));
        m_polyhedralization.m_polyhedrons[0].push_back(f);
    }
}
void ConvexHullPartitionHandle::Calculate()
{
    convex_hull_partition();
}

uint32_t ConvexHullPartitionHandle::GetInsertedVerticesCount()
{
    return count_nested_vector_size(m_polyhedralization.m_inserted_vertices);
}
void ConvexHullPartitionHandle::GetInsertedVertices(uint32_t* out)
{
    vector<uint32_t> temp = nested_vector_to_flat_vector(m_polyhedralization.m_inserted_vertices);
    write_buffer_with_vector(out, temp);
}

uint32_t ConvexHullPartitionHandle::GetPolyhedronsCount()
{
    return count_nested_vector_size(m_polyhedralization.m_polyhedrons);
}
void ConvexHullPartitionHandle::GetPolyhedrons(uint32_t* out)
{
    vector<uint32_t> temp = nested_vector_to_flat_vector(m_polyhedralization.m_polyhedrons);
    write_buffer_with_vector(out, temp);
}

uint32_t ConvexHullPartitionHandle::GetFacetsCount()
{
    return m_polyhedralization.m_facets.size();
}
void ConvexHullPartitionHandle::GetFacets(FacetInteropData* out)
{
    for(uint32_t i=0; i<m_polyhedralization.m_facets.size(); i++)
    {
        out[i] = m_polyhedralization.m_facets[i];
    }
}
uint32_t ConvexHullPartitionHandle::GetSegmentsCount()
{
    return m_polyhedralization.m_segments.size();
}
void ConvexHullPartitionHandle::GetSegments(SegmentInteropData* out)
{
    for(uint32_t i=0; i<m_polyhedralization.m_segments.size(); i++)
    {
        out[i] = m_polyhedralization.m_segments[i];
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

extern "C" LIBRARY_EXPORT void AddConvexHullPartitionInput(void* handle, uint32_t explicit_count, double* explicit_values, uint32_t implicit_count, uint32_t* implicit_values,
                                                           uint32_t tetrahedrons_count, uint32_t* tetrahedrons, uint32_t constraints_facets_count, FacetInteropData* constraints_facets,
                                                           uint32_t segments_count, SegmentInteropData* segments, uint32_t coplanar_triangles_count, uint32_t* coplanar_triangles)
{
    ((ConvexHullPartitionHandle*)handle)->AddInput(explicit_count, explicit_values, implicit_count, implicit_values,
                                                   tetrahedrons_count, tetrahedrons, constraints_facets_count, constraints_facets,
                                                   segments_count, segments, coplanar_triangles_count, coplanar_triangles);
}

extern "C" LIBRARY_EXPORT void CalculateConvexHullPartition(void* handle)
{
    ((ConvexHullPartitionHandle*)handle)->Calculate();
}

extern "C" LIBRARY_EXPORT uint32_t GetConvexHullPartitionInsertedVerticesCount(void* handle)
{
    return ((ConvexHullPartitionHandle*)handle)->GetInsertedVerticesCount();
}
extern "C" LIBRARY_EXPORT void GetConvexHullPartitionInsertedVertices(void* handle, uint32_t* out)
{
    ((ConvexHullPartitionHandle*)handle)->GetInsertedVertices(out);
}

extern "C" LIBRARY_EXPORT uint32_t GetConvexHullPartitionPolyhedronsCount(void* handle)
{
    return ((ConvexHullPartitionHandle*)handle)->GetPolyhedronsCount();
}
extern "C" LIBRARY_EXPORT void GetConvexHullPartitionPolyhedrons(void* handle, uint32_t* out)
{
    ((ConvexHullPartitionHandle*)handle)->GetPolyhedrons(out);
}

extern "C" LIBRARY_EXPORT uint32_t GetConvexHullPartitionFacetsCount(void* handle)
{
    return ((ConvexHullPartitionHandle*)handle)->GetFacetsCount();
}
extern "C" LIBRARY_EXPORT void GetConvexHullPartitionFacets(void* handle, FacetInteropData* out)
{
    ((ConvexHullPartitionHandle*)handle)->GetFacets(out);
}
extern "C" LIBRARY_EXPORT uint32_t GetConvexHullPartitionSegmentsCount(void* handle)
{
    return ((ConvexHullPartitionHandle*)handle)->GetSegmentsCount();
}
extern "C" LIBRARY_EXPORT void GetConvexHullPartitionSegments(void* handle, SegmentInteropData* out)
{
    ((ConvexHullPartitionHandle*)handle)->GetSegments(out);
}


void ConvexHullPartitionHandle::convex_hull_partition()
{
    auto get_coplanar_group = [&](uint32_t p0,uint32_t p1,uint32_t p2) -> uint32_t
    {
        sort_ints(p0,p1,p2);
        return m_triangles_coplanar_groups[make_tuple(p0,p1,p2)];
    };
    
    unordered_map<uint32_t, tuple<uint32_t,uint32_t,uint32_t>> coplanar_groups_to_triangles;
    // calculate slice order
    vector<FacetOrder> facets_order;
    if(0 != m_constraints_facets.size())
    {
        for(uint32_t i=0; i<m_constraints_facets.size(); i++)
        {
            m_constraints_facets[i].ip0 = get_coplanar_group(m_constraints_facets[i].p0,m_constraints_facets[i].p1,m_constraints_facets[i].p2);
            coplanar_groups_to_triangles[m_constraints_facets[i].ip0] = make_tuple(m_constraints_facets[i].p0,m_constraints_facets[i].p1,m_constraints_facets[i].p2);
        }
        facets_order = order_facets(m_polyhedralization.m_vertices, m_approximated_vertices, m_constraints_segments, m_constraints_facets);
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
        FacetOrder facet_order = facets_order[i];
        auto [c0,c1,c2] = coplanar_groups_to_triangles[facet_order.f];
        
        int slice_res = m_polyhedralization.slice_polyhedron_with_plane(p, c0, c1, c2);
        if(1 == slice_res)
        {
            slice_order.push(make_pair(p, facet_order.top));
        }
        else if(-1 == slice_res)
        {
            slice_order.push(make_pair(p, facet_order.bot));
        }
        else
        {
            slice_order.push(make_pair(p, facet_order.top));
            slice_order.push(make_pair(m_polyhedralization.m_polyhedrons.size()-1, facet_order.bot));
        }
    }
    
    approximate_verteices(m_approximated_vertices, m_polyhedralization.m_vertices);
    m_polyhedralization.calculate_facets_centroids(m_approximated_vertices);
}
