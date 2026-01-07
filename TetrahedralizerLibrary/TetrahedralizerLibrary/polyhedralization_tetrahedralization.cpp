#include "polyhedralization_tetrahedralization.hpp"
using namespace std;

void PolyhedralizationTetrahedralizationHandle::polyhedralization_tetrahedralization()
{
    m_triangulated_facets = vector<vector<uint32_t>>(m_facets.size(), vector<uint32_t>(0));
    
    // triangulate facets
    {
        for(uint32_t i=0; i<m_facets.size(); i++)
        {
            uint32_t connect_vertex = UNDEFINED_VALUE;
            vector<uint32_t> vs = m_facets[i].get_sorted_vertices(m_segments);
            for(uint32_t j=0; j<vs.size(); j++)
            {
                uint32_t p = vs[j];
                for(uint32_t k=0; k<vs.size(); k++)
                {
                    uint32_t e0 = vs[k];
                    uint32_t e1 = vs[(k+1)%vs.size()];
                    if(p == e0 || p == e1)
                    {
                        continue;
                    }
                    if(is_collinear(p, e0, e1, m_vertices.data()))
                    {
                        goto NEXT_VERTEX;
                    }
                }
                connect_vertex = p;
                break;
                NEXT_VERTEX:
                continue;
            }
            
            if(connect_vertex == UNDEFINED_VALUE) // add a point at the facet center
            {
                connect_vertex = m_vertices.size();
                m_vertices.push_back(m_facets[i].get_implicit_centroid(m_vertices));
                m_inserted_facets_centroids.push_back(i);
            }
            
            for(uint32_t j=0; j<vs.size(); j++) // triangulate
            {
                uint32_t e0 = vs[j];
                uint32_t e1 = vs[(j+1)%vs.size()];
                if(connect_vertex == e0 || connect_vertex == e1)
                {
                    continue;
                }
                uint32_t t0 = connect_vertex;
                uint32_t t1 = e0;
                uint32_t t2 = e1;
                sort_ints(t0, t1, t2);
                m_triangulated_facets[i].push_back(t0);
                m_triangulated_facets[i].push_back(t1);
                m_triangulated_facets[i].push_back(t2);
            }
        }
    }
    
    
    // convert polyhedrons to tetrahedrons
    {
        for(uint32_t i=0; i<m_polyhedrons.size(); i++)
        {
            uint32_t connect_vertex = find_connect_vertex(i);

            for(uint32_t j=0; j<m_polyhedrons[i].size(); j++)
            {
                uint32_t f = m_polyhedrons[i][j];
                for(uint32_t k=0; k<m_triangulated_facets[f].size(); k+=3)
                {
                    uint32_t t0 = m_triangulated_facets[f][k+0];
                    uint32_t t1 = m_triangulated_facets[f][k+1];
                    uint32_t t2 = m_triangulated_facets[f][k+2];
                    if(t0==connect_vertex || t1==connect_vertex || t2==connect_vertex || 0 == orient3d(t0, t1, t2, connect_vertex, m_vertices.data()))
                    {
                        continue;
                    }
                    add_tetrahedron(t0, t1, t2, connect_vertex);
                }
            }
        }
    }
}

uint32_t PolyhedralizationTetrahedralizationHandle::find_connect_vertex(uint32_t polyhedron)
{
    unordered_map<uint32_t,uint32_t> facets_coplanar_groups_count;
    for(uint32_t f : m_polyhedrons[polyhedron])
    {
        uint32_t cg = m_facets_coplanar_group[f];
        if(facets_coplanar_groups_count.end() == facets_coplanar_groups_count.find(cg))
        {
            facets_coplanar_groups_count[cg] = 1;
        }
        else
        {
            facets_coplanar_groups_count[cg]++;
        }
    }
    
    unordered_set<uint32_t> search_vertices;
    unordered_set<uint32_t> polyhedron_vertices;
    for(uint32_t f : m_polyhedrons[polyhedron])
    {
        vector<uint32_t> vs = m_facets[f].get_vertices(m_segments);
        polyhedron_vertices.insert(vs.begin(),vs.end());
        
        uint32_t cg = m_facets_coplanar_group[f];
        if(1 != facets_coplanar_groups_count[cg])
        {
            continue;
        }
        search_vertices.insert(vs.begin(),vs.end());
    }
    
    unordered_set<tuple<uint32_t,uint32_t,uint32_t>, iii32_hash> polyhedron_triangles;
    for(uint32_t f : m_polyhedrons[polyhedron])
    {
        for(uint32_t j=0; j<m_triangulated_facets[f].size(); j+=3)
        {
            uint32_t t0 = m_triangulated_facets[f][j+0];
            uint32_t t1 = m_triangulated_facets[f][j+1];
            uint32_t t2 = m_triangulated_facets[f][j+2];
            sort_ints(t0,t1,t2);
            polyhedron_triangles.insert(make_tuple(t0,t1,t2));
        }
    }

    for(uint32_t v : search_vertices)
    {
        unordered_set<tuple<uint32_t,uint32_t,uint32_t>, iii32_hash> cur_triangles;
        
        for(uint32_t i=0; i<m_polyhedrons[polyhedron].size(); i++)
        {
            uint32_t f = m_polyhedrons[polyhedron][i];
            for(uint32_t j=0; j<m_triangulated_facets[f].size(); j+=3)
            {
                uint32_t t0 = m_triangulated_facets[f][j+0];
                uint32_t t1 = m_triangulated_facets[f][j+1];
                uint32_t t2 = m_triangulated_facets[f][j+2];
                if(v==t0 || v==t1 || v==t2 || 0 == orient3d(t0, t1, t2, v, m_vertices.data()))
                {
                    continue;
                }
                
                auto add_triangle = [&](uint32_t p0, uint32_t p1, uint32_t p2)
                {
                    sort_ints(p0,p1,p2);
                    cur_triangles.insert(make_tuple(p0,p1,p2));
                };
                add_triangle(t0,t1,t2);
                add_triangle(t0,t1,v);
                add_triangle(t1,t2,v);
                add_triangle(t2,t0,v);
            }
        }
        
        for(auto k : polyhedron_triangles)
        {
            if(cur_triangles.end() == cur_triangles.find(k))
            {
                goto NEXT_VERTEX;
            }
        }
        return v;
        NEXT_VERTEX:
        continue;
    }
    
    // if polyhedron can't be tetrahedralized by connecting a vertex to every triangulated facet
    double3 center(0.0,0.0,0.0);
    for(uint32_t v : polyhedron_vertices)
    {
        center += m_approximated_vertices[v];
    }
    center /= (double)polyhedron_vertices.size();
    uint32_t connect_vertex = m_vertices.size();
    m_vertices.push_back(std::make_shared<explicitPoint3D> (center.x, center.y, center.z));
    m_inserted_polyhedrons_centroids.push_back(polyhedron);
    return connect_vertex;
}

void PolyhedralizationTetrahedralizationHandle::add_tetrahedron(uint32_t t0,uint32_t t1,uint32_t t2,uint32_t t3)
{
    if(-1 == orient3d(t0,t1,t2,t3,m_vertices.data()))
    {
        swap(t2,t3);
    }
    
    m_tetrahedrons.push_back(t0);
    m_tetrahedrons.push_back(t1);
    m_tetrahedrons.push_back(t2);
    m_tetrahedrons.push_back(t3);
}


void PolyhedralizationTetrahedralizationHandle::Dispose()
{}
void PolyhedralizationTetrahedralizationHandle::AddInput(uint32_t explicit_count, double* explicit_values, uint32_t implicit_count, uint32_t* implicit_values,
                                                         uint32_t polyhedrons_count, uint32_t* polyhedrons, uint32_t facets_count, FacetInteropData* facets, uint32_t segments_count, SegmentInteropData* segments)
{
    unordered_map<std::tuple<uint32_t, uint32_t, uint32_t>, uint32_t, iii32_hash> triangles_coplanar_groups;
    
    m_vertices = create_vertices(explicit_count, explicit_values, implicit_count, implicit_values);
    approximate_verteices(m_approximated_vertices, m_vertices);
    m_polyhedrons = flat_array_to_nested_vector(polyhedrons, polyhedrons_count);
    for(uint32_t i=0; i<facets_count; i++)
    {
        m_facets.push_back(facets[i].to_facet());
        uint32_t cg = search_int(m_facets[i].p0, m_facets[i].p1, m_facets[i].p2, triangles_coplanar_groups);
        if(UNDEFINED_VALUE == cg)
        {
            cg = triangles_coplanar_groups.size();
            assign_int(m_facets[i].p0, m_facets[i].p1, m_facets[i].p2, cg, triangles_coplanar_groups);
        }
        m_facets_coplanar_group.push_back(cg);
    }
    for(uint32_t i=0; i<segments_count; i++)
    {
        m_segments.push_back(segments[i].to_segment());
    }
}

void PolyhedralizationTetrahedralizationHandle::Calculate()
{
    polyhedralization_tetrahedralization();
}

uint32_t PolyhedralizationTetrahedralizationHandle::GetInsertedFacetsCentroidsCount()
{
    return m_inserted_facets_centroids.size();
}
void PolyhedralizationTetrahedralizationHandle::GetInsertedFacetsCentroids(uint32_t* out)
{
    write_buffer_with_vector(out, m_inserted_facets_centroids);
}
uint32_t PolyhedralizationTetrahedralizationHandle::GetInsertedPolyhedronsCentroidsCount()
{
    return m_inserted_polyhedrons_centroids.size();
}
void PolyhedralizationTetrahedralizationHandle::GetInsertedPolyhedronsCentroids(uint32_t* out)
{
    write_buffer_with_vector(out, m_inserted_polyhedrons_centroids);
}
uint32_t PolyhedralizationTetrahedralizationHandle::GetTetrahedronsCount()
{
    return m_tetrahedrons.size();
}
void PolyhedralizationTetrahedralizationHandle::GetTetrahedrons(uint32_t* out)
{
    write_buffer_with_vector(out, m_tetrahedrons);
}

extern "C" LIBRARY_EXPORT void* CreatePolyhedralizationTetrahedralizationHandle()
{
    return new PolyhedralizationTetrahedralizationHandle();
}
extern "C" LIBRARY_EXPORT void DisposePolyhedralizationTetrahedralizationHandle(void* handle)
{
    ((PolyhedralizationTetrahedralizationHandle*)handle)->Dispose();
    delete ((PolyhedralizationTetrahedralizationHandle*)handle);
}

extern "C" LIBRARY_EXPORT void AddPolyhedralizationTetrahedralizationInput(void* handle, uint32_t explicit_count, double* explicit_values, uint32_t implicit_count, uint32_t* implicit_values,
                                                                           uint32_t polyhedrons_count, uint32_t* polyhedrons, uint32_t facets_count, FacetInteropData* facets, uint32_t segments_count, SegmentInteropData* segments)
{
    ((PolyhedralizationTetrahedralizationHandle*)handle)->AddInput(explicit_count, explicit_values, implicit_count, implicit_values,
                                                                   polyhedrons_count, polyhedrons, facets_count, facets, segments_count, segments);
}

extern "C" LIBRARY_EXPORT void CalculatePolyhedralizationTetrahedralization(void* handle)
{
    ((PolyhedralizationTetrahedralizationHandle*)handle)->Calculate();
}

extern "C" LIBRARY_EXPORT uint32_t GetPolyhedralizationTetrahedralizationInsertedFacetsCentroidsCount(void* handle)
{
    return ((PolyhedralizationTetrahedralizationHandle*)handle)->GetInsertedFacetsCentroidsCount();
}
extern "C" LIBRARY_EXPORT void GetPolyhedralizationTetrahedralizationInsertedFacetsCentroids(void* handle, uint32_t* out)
{
    ((PolyhedralizationTetrahedralizationHandle*)handle)->GetInsertedFacetsCentroids(out);
}
extern "C" LIBRARY_EXPORT uint32_t GetPolyhedralizationTetrahedralizationInsertedPolyhedronsCentroidsCount(void* handle)
{
    return ((PolyhedralizationTetrahedralizationHandle*)handle)->GetInsertedPolyhedronsCentroidsCount();
}
extern "C" LIBRARY_EXPORT void GetPolyhedralizationTetrahedralizationInsertedPolyhedronsCentroids(void* handle,uint32_t* out)
{
    ((PolyhedralizationTetrahedralizationHandle*)handle)->GetInsertedPolyhedronsCentroids(out);
}
extern "C" LIBRARY_EXPORT uint32_t GetPolyhedralizationTetrahedralizationTetrahedronsCount(void* handle)
{
    return ((PolyhedralizationTetrahedralizationHandle*)handle)->GetTetrahedronsCount();
}
extern "C" LIBRARY_EXPORT void GetPolyhedralizationTetrahedralizationTetrahedrons(void* handle, uint32_t* out)
{
    ((PolyhedralizationTetrahedralizationHandle*)handle)->GetTetrahedrons(out);
}
