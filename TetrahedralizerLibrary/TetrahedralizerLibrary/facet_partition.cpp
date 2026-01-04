#include "facet_partition.hpp"
using namespace std;

void FacetPartitionHandle::Dispose()
{}

void FacetPartitionHandle::AddInput(uint32_t explicit_count, double* explicit_values, uint32_t implicit_count, uint32_t* implicit_values,
                                    uint32_t polyhedrons_count, uint32_t* polyhedrons, uint32_t facets_count, FacetInteropData* facets, uint32_t* facets_centroids_mapping, uint32_t segments_count, SegmentInteropData* segments,
                                    uint32_t coplanar_triangles_count, uint32_t* coplanar_triangles, uint32_t constraints_count, uint32_t* constraints)
{
    m_explicit_count = explicit_count;
    m_polyhedralization.m_vertices = create_vertices(explicit_count, explicit_values, implicit_count, implicit_values);
    approximate_verteices(m_approximated_vertices, m_polyhedralization.m_vertices);
    m_polyhedralization.m_polyhedrons = flat_array_to_nested_vector(polyhedrons, polyhedrons_count);
    for(uint32_t i=0; i<facets_count; i++)
    {
        m_polyhedralization.m_facets.push_back(facets[i].to_facet());
    }
    m_facets_centroids_mapping = vector<uint32_t>(facets_centroids_mapping,facets_centroids_mapping+facets_count);
    for(uint32_t i=0; i<segments_count; i++)
    {
        m_polyhedralization.m_segments.push_back(segments[i].to_segment());
    }
    
    vector<vector<uint32_t>> temp = flat_array_to_nested_vector(coplanar_triangles, coplanar_triangles_count);
    for(uint32_t i=0; i<temp.size(); i++)
    {
        m_coplanar_groups_normals.push_back(max_component_in_triangle_normal(temp[i][0], temp[i][1], temp[i][2], m_polyhedralization.m_vertices.data()));
        m_coplanar_triangles.push_back(temp[i][0]);
        m_coplanar_triangles.push_back(temp[i][1]);
        m_coplanar_triangles.push_back(temp[i][2]);
        for(uint32_t j=0; j<temp[i].size()/3; j++)
        {
            uint32_t c0 = temp[i][3*j+0];
            uint32_t c1 = temp[i][3*j+1];
            uint32_t c2 = temp[i][3*j+2];
            sort_ints(c0, c1, c2);
            m_triangles_coplanar_groups[make_tuple(c0,c1,c2)] = i;
        }
    }
    
    m_constraints = create_constraints(constraints_count, constraints, m_polyhedralization.m_vertices.data(), true);
}

void FacetPartitionHandle::Calculate()
{
    vector<vector<uint32_t>> coplanar_constraints_groups;
    for(uint32_t i=0; i<m_constraints.size()/3; i++)
    {
        uint32_t c0 = m_constraints[3*i+0];
        uint32_t c1 = m_constraints[3*i+1];
        uint32_t c2 = m_constraints[3*i+2];
        if(UNDEFINED_VALUE == c0)
        {
            continue;
        }
        uint32_t cg = search_int(c0,c1,c2,m_triangles_coplanar_groups);
        while(coplanar_constraints_groups.size() <= cg)
        {
            coplanar_constraints_groups.push_back(vector<uint32_t>());
        }
        coplanar_constraints_groups[cg].push_back(i);
    }
    vector<vector<uint32_t>> coplanar_constraints_split_segments;
    for(uint32_t i=0; i<coplanar_constraints_groups.size(); i++)
    {
        unordered_map<pair<uint32_t,uint32_t>, uint32_t, ii32_hash> segments_cache;
        auto add_segement = [&](uint32_t p0, uint32_t p1)
        {
            sort_ints(p0,p1);
            if(segments_cache.end() == segments_cache.find(make_pair(p0, p1)))
            {
                segments_cache[make_pair(p0, p1)] = 1;
            }
            else
            {
                segments_cache[make_pair(p0, p1)]++;
            }
        };
        for(uint32_t c : coplanar_constraints_groups[i])
        {
            uint32_t c0 = m_constraints[3*c+0];
            uint32_t c1 = m_constraints[3*c+1];
            uint32_t c2 = m_constraints[3*c+2];
            add_segement(c0, c1);
            add_segement(c1, c2);
            add_segement(c2, c0);
        }
        
        coplanar_constraints_split_segments.push_back(vector<uint32_t>());
        for(auto [k,v] : segments_cache)
        {
            if(2 != v)
            {
                continue;
            }
            coplanar_constraints_split_segments[i].push_back(k.first);
            coplanar_constraints_split_segments[i].push_back(k.second);
        }
    }
    vector<vector<uint32_t>> coplanar_facets_groups; // only keep boundary facets
    for(uint32_t i=0; i<m_polyhedralization.m_facets.size(); i++)
    {
        if(UNDEFINED_VALUE != m_polyhedralization.m_facets[i].ip1)
        {
            continue;
        }
        uint32_t c0 = m_polyhedralization.m_facets[i].p0;
        uint32_t c1 = m_polyhedralization.m_facets[i].p1;
        uint32_t c2 = m_polyhedralization.m_facets[i].p2;

        uint32_t cg = search_int(c0,c1,c2,m_triangles_coplanar_groups);
        while(coplanar_facets_groups.size() <= cg)
        {
            coplanar_facets_groups.push_back(vector<uint32_t>());
        }
        coplanar_facets_groups[cg].push_back(i);
    }
    
    for(uint32_t i=0; i<coplanar_constraints_split_segments.size(); i++)
    {
        uint32_t cp0 = m_coplanar_triangles[3*i+0];
        uint32_t cp1 = m_coplanar_triangles[3*i+1];
        uint32_t cp2 = m_coplanar_triangles[3*i+2];
        unordered_set<uint32_t> split_facets;
        for(uint32_t j=0; j<coplanar_constraints_split_segments[i].size()/2; j++)
        {
            uint32_t s0 = coplanar_constraints_split_segments[i][2*j+0];
            uint32_t s1 = coplanar_constraints_split_segments[i][2*j+1];
            
            vector<uint32_t> intersect_facets;
            for(uint32_t f : coplanar_facets_groups[i])
            {
                if(m_polyhedralization.m_facets[f].intersects_segment(s0, s1, m_coplanar_groups_normals[i], m_polyhedralization.m_vertices, m_polyhedralization.m_segments))
                {
                    intersect_facets.push_back(f);
                }
            }
            
            uint32_t s2 = UNDEFINED_VALUE;
            if(0 != intersect_facets.size())
            {
                for(uint32_t i=0; i<m_explicit_count; i++)
                {
                    if(0 != orient3d(cp0,cp1,cp2,i,m_polyhedralization.m_vertices.data()))
                    {
                        s2 = i;
                        break;
                    }
                }
            }
            if(UNDEFINED_VALUE == s2)
            {
                continue;
            }
            for(uint32_t k=0; k<intersect_facets.size(); k++)
            {
                if(m_polyhedralization.slice_facet_with_plane(intersect_facets[k], s0, s1, s2))
                {
                    coplanar_facets_groups[i].push_back(m_polyhedralization.m_facets.size()-1);
                    m_facets_centroids_mapping.push_back(UNDEFINED_VALUE);
                    split_facets.insert(intersect_facets[k]);
                    split_facets.insert(m_polyhedralization.m_facets.size()-1);
                }
            }
        }
        approximate_verteices(m_approximated_vertices, m_polyhedralization.m_vertices);
        for(uint32_t f : split_facets)
        {
            m_polyhedralization.m_facets[f].calculate_implicit_centroid(m_approximated_vertices, m_polyhedralization.m_segments);
            m_facets_centroids_mapping[f] = UNDEFINED_VALUE;
            for(uint32_t c : coplanar_constraints_groups[i])
            {
                uint32_t c0 = m_coplanar_triangles[3*c+0];
                uint32_t c1 = m_coplanar_triangles[3*c+1];
                uint32_t c2 = m_coplanar_triangles[3*c+2];
                if(genericPoint::pointInTriangle(*m_polyhedralization.m_facets[f].get_implicit_centroid(m_polyhedralization.m_vertices),*m_polyhedralization.m_vertices[c0],*m_polyhedralization.m_vertices[c1],*m_polyhedralization.m_vertices[c2], m_coplanar_groups_normals[i]))
                {
                    m_facets_centroids_mapping[f] = c;
                    break;
                }
            }
        }
    }
}

uint32_t FacetPartitionHandle::GetInsertedVerticesCount()
{
    return count_nested_vector_size(m_polyhedralization.m_inserted_vertices);
}
void FacetPartitionHandle::GetInsertedVertices(uint32_t* out)
{
    vector<uint32_t> temp = nested_vector_to_flat_vector(m_polyhedralization.m_inserted_vertices);
    write_buffer_with_vector(out, temp);
}

uint32_t FacetPartitionHandle::GetPolyhedronsCount()
{
    return count_nested_vector_size(m_polyhedralization.m_polyhedrons);
}
void FacetPartitionHandle::GetPolyhedrons(uint32_t* out)
{
    vector<uint32_t> temp = nested_vector_to_flat_vector(m_polyhedralization.m_polyhedrons);
    write_buffer_with_vector(out, temp);
}

uint32_t FacetPartitionHandle::GetFacetsCount()
{
    return m_polyhedralization.m_facets.size();
}
void FacetPartitionHandle::GetFacets(FacetInteropData* out)
{
    for(uint32_t i=0; i<m_polyhedralization.m_facets.size(); i++)
    {
        out[i] = m_polyhedralization.m_facets[i];
    }
}
void FacetPartitionHandle::GetFacetsCentroidsMapping(uint32_t* out)
{
    write_buffer_with_vector(out, m_facets_centroids_mapping);
}
uint32_t FacetPartitionHandle::GetSegmentsCount()
{
    return m_polyhedralization.m_segments.size();
}
void FacetPartitionHandle::GetSegments(SegmentInteropData* out)
{
    for(uint32_t i=0; i<m_polyhedralization.m_segments.size(); i++)
    {
        out[i] = m_polyhedralization.m_segments[i];
    }
}

extern "C" LIBRARY_EXPORT void* CreateFacetPartitionHandle()
{
    return new FacetPartitionHandle();
}
extern "C" LIBRARY_EXPORT void DisposeFacetPartitionHandle(void* handle)
{
    ((FacetPartitionHandle*)handle)->Dispose();
    delete (FacetPartitionHandle*)handle;
}

extern "C" LIBRARY_EXPORT void AddFacetPartitionInput(void* handle, uint32_t explicit_count, double* explicit_values, uint32_t implicit_count, uint32_t* implicit_values,
                                                      uint32_t polyhedrons_count, uint32_t* polyhedrons, uint32_t facets_count, FacetInteropData* facets, uint32_t* facets_centroids_mapping, uint32_t segments_count, SegmentInteropData* segments,
                                                      uint32_t coplanar_triangles_count, uint32_t* coplanar_triangles, uint32_t constraints_count, uint32_t* constraints)
{
    ((FacetPartitionHandle*)handle)->AddInput(explicit_count, explicit_values, implicit_count, implicit_values,
                                              polyhedrons_count, polyhedrons, facets_count, facets, facets_centroids_mapping, segments_count, segments,
                                              coplanar_triangles_count, coplanar_triangles, constraints_count, constraints);
}

extern "C" LIBRARY_EXPORT void CalculateFacetPartition(void* handle)
{
    ((FacetPartitionHandle*)handle)->Calculate();
}

extern "C" LIBRARY_EXPORT uint32_t GetFacetPartitionInsertedVerticesCount(void* handle)
{
    return ((FacetPartitionHandle*)handle)->GetInsertedVerticesCount();
}
extern "C" LIBRARY_EXPORT void GetFacetPartitionInsertedVertices(void* handle, uint32_t* out)
{
    return  ((FacetPartitionHandle*)handle)->GetInsertedVertices(out);
}

extern "C" LIBRARY_EXPORT uint32_t GetFacetPartitionPolyhedronsCount(void* handle)
{
    return ((FacetPartitionHandle*)handle)->GetPolyhedronsCount();
}
extern "C" LIBRARY_EXPORT void GetFacetPartitionPolyhedrons(void* handle, uint32_t* out)
{
    return ((FacetPartitionHandle*)handle)->GetPolyhedrons(out);
}
extern "C" LIBRARY_EXPORT uint32_t GetFacetPartitionFacetsCount(void* handle)
{
    return ((FacetPartitionHandle*)handle)->GetFacetsCount();
}
extern "C" LIBRARY_EXPORT void GetFacetPartitionFacets(void* handle, FacetInteropData* out)
{
    ((FacetPartitionHandle*)handle)->GetFacets(out);
}
extern "C" LIBRARY_EXPORT void GetFacetPartitionFacetsCentrodisMapping(void* handle, uint32_t* out)
{
    ((FacetPartitionHandle*)handle)->GetFacetsCentroidsMapping(out);
}
extern "C" LIBRARY_EXPORT uint32_t GetFacetPartitionSegmentsCount(void* handle)
{
    return ((FacetPartitionHandle*)handle)->GetSegmentsCount();
}
extern "C" LIBRARY_EXPORT void GetFacetPartitionSegments(void* handle, SegmentInteropData* out)
{
    ((FacetPartitionHandle*)handle)->GetSegments(out);
}
