#include "polyhedralization_tetrahedralization.hpp"
using namespace std;

void PolyhedralizationTetrahedralizationHandle::polyhedralization_tetrahedralization()
{
    m_triangulated_facets = vector<vector<uint32_t>>(m_facets.size(), vector<uint32_t>(0));
    m_triangulated_facets_counters = vector<vector<uint32_t>>(m_facets.size(), vector<uint32_t>(0));
    
    // triangulate facets
    {
        for(uint32_t i=0; i<m_facets.size(); i++)
        {
            uint32_t connect_vertex = UNDEFINED_VALUE;
            for(uint32_t j=0; j<m_facets[i].size(); j++)
            {
                uint32_t p = m_facets[i][j];
                for(uint32_t k=0; k<m_facets[i].size(); k++)
                {
                    uint32_t e0 = m_facets[i][k];
                    uint32_t e1 = m_facets[i][(k+1)%m_facets[i].size()];
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
                m_vertices.push_back(m_facets_centroids[i]);
                m_inserted_facets_centroids.push_back(i);
            }
            
            for(uint32_t j=0; j<m_facets[i].size(); j++) // triangulate
            {
                uint32_t e0 = m_facets[i][j];
                uint32_t e1 = m_facets[i][(j+1)%m_facets[i].size()];
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
                m_triangulated_facets_counters[i].push_back(0);
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
    m_u_set_i_0.clear(); // get all vertices
    for(uint32_t i=0; i<m_polyhedrons[polyhedron].size(); i++)
    {
        uint32_t f = m_polyhedrons[polyhedron][i];
        for(uint32_t j=0; j<m_facets[f].size(); j++)
        {
            m_u_set_i_0.insert(m_facets[f][j]);
        }
    }
    
    for(uint32_t v : m_u_set_i_0)
    {
        // clear facets counters
        for(uint32_t i=0; i<m_polyhedrons[polyhedron].size(); i++)
        {
            uint32_t f = m_polyhedrons[polyhedron][i];
            for(uint32_t j=0; j<m_triangulated_facets_counters[f].size(); j++)
            {
                m_triangulated_facets_counters[f][j] = 0;
            }
        }
        
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
                
                m_triangulated_facets_counters[f][j/3]++;
                find_connect_vertex_helper(polyhedron,t0,t1,v);
                find_connect_vertex_helper(polyhedron,t0,t2,v);
                find_connect_vertex_helper(polyhedron,t1,t2,v);
            }
        }
        
        // check facets counters
        for(uint32_t i=0; i<m_polyhedrons[polyhedron].size(); i++)
        {
            uint32_t f = m_polyhedrons[polyhedron][i];
            for(uint32_t j=0; j<m_triangulated_facets_counters[f].size(); j++)
            {
                if(1 != m_triangulated_facets_counters[f][j])
                {
                    goto NEXT_VERTEX;
                }
            }
        }
        return v;
        NEXT_VERTEX:
        continue;
    }
    
    // if polyhedron can't be tetrahedralized by connecting a vertex to every triangulated facet
    double3 center(0.0,0.0,0.0);
    for(uint32_t v : m_u_set_i_0)
    {
        center += approximate_point(m_vertices[v]);
    }
    center /= (double)m_u_set_i_0.size();
    uint32_t connect_vertex = m_vertices.size();
    m_vertices.push_back(std::make_shared<explicitPoint3D> (center.x, center.y, center.z));
    m_inserted_polyhedrons_centroids.push_back(polyhedron);
    return connect_vertex;
}

void PolyhedralizationTetrahedralizationHandle::find_connect_vertex_helper(uint32_t p, uint32_t t0,uint32_t t1,uint32_t t2)
{
    sort_ints(t0,t1,t2);
    for(uint32_t i=0; i<m_polyhedrons[p].size(); i++)
    {
        uint32_t f = m_polyhedrons[p][i];
        for(uint32_t j=0; j<m_triangulated_facets[f].size(); j+=3)
        {
            uint32_t p0 = m_triangulated_facets[f][j+0];
            uint32_t p1 = m_triangulated_facets[f][j+1];
            uint32_t p2 = m_triangulated_facets[f][j+2];
            if(p0==t0 && p1==t1 && p2==t2)
            {
                m_triangulated_facets_counters[f][j/3]++;
                return;
            }
        }
    }
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
void PolyhedralizationTetrahedralizationHandle::AddInput(uint32_t explicit_count, double* explicit_values, uint32_t implicit_count, uint32_t* implicit_values, uint32_t polyhedrons_count, uint32_t* polyhedrons, uint32_t facets_count, uint32_t* facets, uint32_t* facets_centroids, double* facets_centroids_weights)
{
    m_vertices = create_vertices(explicit_count, explicit_values, implicit_count, implicit_values);
    m_polyhedrons = flat_array_to_nested_vector(polyhedrons, polyhedrons_count);
    m_facets = flat_array_to_nested_vector(facets, facets_count);
    m_facets_centroids = create_vertices(facets_count, facets_centroids, facets_centroids_weights, m_vertices.data());
}

void PolyhedralizationTetrahedralizationHandle::Calculate()
{
    polyhedralization_tetrahedralization();
}

uint32_t PolyhedralizationTetrahedralizationHandle::GetInsertedFacetsCentroidsCount()
{
    return m_inserted_facets_centroids.size();
}
uint32_t* PolyhedralizationTetrahedralizationHandle::GetInsertedFacetsCentroids()
{
    return m_inserted_facets_centroids.data();
}
uint32_t PolyhedralizationTetrahedralizationHandle::GetInsertedPolyhedronsCentroidsCount()
{
    return m_inserted_polyhedrons_centroids.size();
}
uint32_t* PolyhedralizationTetrahedralizationHandle::GetInsertedPolyhedronsCentroids()
{
    return m_inserted_polyhedrons_centroids.data();
}
uint32_t PolyhedralizationTetrahedralizationHandle::GetTetrahedronsCount()
{
    return m_tetrahedrons.size() / 4;
}
uint32_t* PolyhedralizationTetrahedralizationHandle::GetTetrahedrons()
{
    return m_tetrahedrons.data();
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

extern "C" LIBRARY_EXPORT void AddPolyhedralizationTetrahedralizationInput(void* handle, uint32_t explicit_count, double* explicit_values, uint32_t implicit_count, uint32_t* implicit_values, uint32_t polyhedrons_count, uint32_t* polyhedrons, uint32_t facets_count, uint32_t* facets, uint32_t* facets_centroids, double* facets_centroids_weights)
{
    ((PolyhedralizationTetrahedralizationHandle*)handle)->AddInput(explicit_count, explicit_values, implicit_count, implicit_values, polyhedrons_count, polyhedrons, facets_count, facets, facets_centroids, facets_centroids_weights);
}

extern "C" LIBRARY_EXPORT void CalculatePolyhedralizationTetrahedralization(void* handle)
{
    ((PolyhedralizationTetrahedralizationHandle*)handle)->Calculate();
}

extern "C" LIBRARY_EXPORT uint32_t GetPolyhedralizationTetrahedralizationInsertedFacetsCentroidsCount(void* handle)
{
    return ((PolyhedralizationTetrahedralizationHandle*)handle)->GetInsertedFacetsCentroidsCount();
}
extern "C" LIBRARY_EXPORT uint32_t* GetPolyhedralizationTetrahedralizationInsertedFacetsCentroids(void* handle)
{
    return ((PolyhedralizationTetrahedralizationHandle*)handle)->GetInsertedFacetsCentroids();
}
extern "C" LIBRARY_EXPORT uint32_t GetPolyhedralizationTetrahedralizationInsertedPolyhedronsCentroidsCount(void* handle)
{
    return ((PolyhedralizationTetrahedralizationHandle*)handle)->GetInsertedPolyhedronsCentroidsCount();
}
extern "C" LIBRARY_EXPORT uint32_t* GetPolyhedralizationTetrahedralizationInsertedPolyhedronsCentroids(void* handle)
{
    return ((PolyhedralizationTetrahedralizationHandle*)handle)->GetInsertedPolyhedronsCentroids();
}
extern "C" LIBRARY_EXPORT uint32_t GetPolyhedralizationTetrahedralizationTetrahedronsCount(void* handle)
{
    return ((PolyhedralizationTetrahedralizationHandle*)handle)->GetTetrahedronsCount();
}
extern "C" LIBRARY_EXPORT uint32_t* GetPolyhedralizationTetrahedralizationTetrahedrons(void* handle)
{
    return ((PolyhedralizationTetrahedralizationHandle*)handle)->GetTetrahedrons();
}
