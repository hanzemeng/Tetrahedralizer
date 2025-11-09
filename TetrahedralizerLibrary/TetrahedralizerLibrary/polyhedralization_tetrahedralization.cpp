#include "polyhedralization_tetrahedralization.hpp"

void PolyhedralizationTetrahedralization::polyhedralization_tetrahedralization(PolyhedralizationTetrahedralizationInput* input, PolyhedralizationTetrahedralizationOutput* output)
{
    m_vertices = vector<genericPoint*>(input->m_vertices, input->m_vertices+input->m_vertices_count);
    m_polyhedrons = flat_array_to_nested_vector(input->m_polyhedrons, input->m_polyhedrons_count);
    m_facets = flat_array_to_nested_vector(input->m_polyhedrons_facets, input->m_polyhedrons_facets_count);
    m_triangulated_facets = vector<vector<uint32_t>>(input->m_polyhedrons_facets_count, vector<uint32_t>(0));
    m_triangulated_facets_counters = vector<vector<uint32_t>>(input->m_polyhedrons_facets_count, vector<uint32_t>(0));
    
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
                double3 center(0.0, 0.0, 0.0);
                for(uint32_t j=0; j<m_facets[i].size(); j++)
                {
                    center += approximate_point(m_vertices[m_facets[i][j]]);
                }
                center /= (double)m_facets[i].size();
                connect_vertex = m_vertices.size();
                m_vertices.push_back(new explicitPoint3D(center.x,center.y,center.z));
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
        for(uint32_t i=0; i<input->m_polyhedrons_count; i++)
        {
            uint32_t connect_vertex = find_connect_vertex(i);
            if(UNDEFINED_VALUE == connect_vertex) // if polyhedron can be tetrahedralized by connecting a vertex to every triangulated facet
            {
                m_u_set_i_0.clear(); // get all vertices
                for(uint32_t j=0; j<m_polyhedrons[i].size(); j++)
                {
                    uint32_t f = m_polyhedrons[i][j];
                    for(uint32_t k=0; k<m_facets[f].size(); k++)
                    {
                        m_u_set_i_0.insert(m_facets[f][k]);
                    }
                }
                
                double3 center(0.0,0.0,0.0);
                for(uint32_t v : m_u_set_i_0)
                {
                    center += approximate_point(m_vertices[v]);
                }
                center /= (double)m_u_set_i_0.size();
                
                connect_vertex = m_vertices.size();
                m_vertices.push_back(new explicitPoint3D(center.x, center.y, center.z));
                m_inserted_polyhedrons_centroids.push_back(i);
            }
            
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
        
        // copy output
        {
            output->m_inserted_facets_centroids = vector_to_array(m_inserted_facets_centroids);
            output->m_inserted_facets_centroids_count = m_inserted_facets_centroids.size();
            output->m_inserted_polyhedrons_centroids = vector_to_array(m_inserted_polyhedrons_centroids);
            output->m_inserted_polyhedrons_centroids_count = m_inserted_polyhedrons_centroids.size();
            output->m_tetrahedrons = vector_to_array(m_tetrahedrons);
            output->m_tetrahedrons_count = m_tetrahedrons.size() / 4;
            
            for(uint32_t i=input->m_vertices_count; i<m_vertices.size(); i++)
            {
                delete (explicitPoint3D*)m_vertices[i];
            }
        }
    }
}

uint32_t PolyhedralizationTetrahedralization::find_connect_vertex(uint32_t polyhedron)
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
                polyhedron_has_triangle(polyhedron,t0,t1,v);
                polyhedron_has_triangle(polyhedron,t0,t2,v);
                polyhedron_has_triangle(polyhedron,t1,t2,v);
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
    
    return UNDEFINED_VALUE;
}

// true if no facet has the triangle or the facet has the triangle in order
bool PolyhedralizationTetrahedralization::polyhedron_has_triangle(uint32_t p, uint32_t t0,uint32_t t1,uint32_t t2)
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
            }
        }
    }
    return true;
}

void PolyhedralizationTetrahedralization::add_tetrahedron(uint32_t t0,uint32_t t1,uint32_t t2,uint32_t t3)
{
    if(1 != orient3d(t0,t1,t2,t3,m_vertices.data()))
    {
        swap(t2,t3);
    }
    
    m_tetrahedrons.push_back(t0);
    m_tetrahedrons.push_back(t1);
    m_tetrahedrons.push_back(t2);
    m_tetrahedrons.push_back(t3);
}


PolyhedralizationTetrahedralizationHandle::PolyhedralizationTetrahedralizationHandle()
{
    m_input = new PolyhedralizationTetrahedralizationInput();
    m_input->m_vertices_count = 0;
    m_input->m_polyhedrons_count = 0;
    m_input->m_polyhedrons_facets_count = 0;
    m_input->m_vertices = nullptr;
    m_input->m_polyhedrons = nullptr;
    m_input->m_polyhedrons_facets = nullptr;
    m_output = new PolyhedralizationTetrahedralizationOutput();
    m_output->m_inserted_facets_centroids = nullptr;
    m_output->m_inserted_facets_centroids_count = 0;
    m_output->m_inserted_polyhedrons_centroids = nullptr;
    m_output->m_inserted_polyhedrons_centroids_count = 0;
    m_output->m_tetrahedrons_count = 0;
    m_output->m_tetrahedrons = nullptr;
    m_polyhedralizationTetrahedralization = new PolyhedralizationTetrahedralization();
}

void PolyhedralizationTetrahedralizationHandle::Dispose()
{
    delete_vertices(m_input->m_vertices, m_input->m_vertices_count);
    delete[] m_input->m_polyhedrons;
    delete[] m_input->m_polyhedrons_facets;
    delete m_input;
    delete[] m_output->m_inserted_facets_centroids;
    delete[] m_output->m_inserted_polyhedrons_centroids;
    delete[] m_output->m_tetrahedrons;
    delete m_output;
    delete m_polyhedralizationTetrahedralization;
}

void PolyhedralizationTetrahedralizationHandle::AddPolyhedralizationTetrahedralizationInput(uint32_t explicit_count, double* explicit_values, uint32_t implicit_count, uint32_t* implicit_values, uint32_t polyhedrons_count, uint32_t* polyhedrons, uint32_t polyhedrons_facets_count, uint32_t* polyhedrons_facets)
{
    create_vertices(explicit_count, explicit_values, implicit_count, implicit_values, m_input->m_vertices, m_input->m_vertices_count);

    vector<uint32_t> vec = flat_array_to_vector(polyhedrons, polyhedrons_count);
    m_input->m_polyhedrons = vector_to_array(vec);
    m_input->m_polyhedrons_count = polyhedrons_count;
    
    vec = flat_array_to_vector(polyhedrons_facets, polyhedrons_facets_count);
    m_input->m_polyhedrons_facets = vector_to_array(vec);
    m_input->m_polyhedrons_facets_count = polyhedrons_facets_count;
}

void PolyhedralizationTetrahedralizationHandle::CalculatePolyhedralizationTetrahedralization()
{
    m_polyhedralizationTetrahedralization->polyhedralization_tetrahedralization(m_input, m_output);
}


uint32_t PolyhedralizationTetrahedralizationHandle::GetOutputInsertedFacetsCentroidsCount()
{
    return m_output->m_inserted_facets_centroids_count;
}
uint32_t* PolyhedralizationTetrahedralizationHandle::GetOutputInsertedFacetsCentroids()
{
    return m_output->m_inserted_facets_centroids;
}
uint32_t PolyhedralizationTetrahedralizationHandle::GetOutputInsertedPolyhedronsCentroidsCount()
{
    return m_output->m_inserted_polyhedrons_centroids_count;
}
uint32_t* PolyhedralizationTetrahedralizationHandle::GetOutputInsertedPolyhedronsCentroids()
{
    return m_output->m_inserted_polyhedrons_centroids;
}

uint32_t PolyhedralizationTetrahedralizationHandle::GetOutputTetrahedronsCount()
{
    return m_output->m_tetrahedrons_count;
}
uint32_t* PolyhedralizationTetrahedralizationHandle::GetOutputTetrahedrons()
{
    return m_output->m_tetrahedrons;
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

extern "C" LIBRARY_EXPORT void AddPolyhedralizationTetrahedralizationInput(void* handle, uint32_t explicit_count, double* explicit_values, uint32_t implicit_count, uint32_t* implicit_values, uint32_t polyhedrons_count, uint32_t* polyhedrons, uint32_t polyhedrons_facets_count, uint32_t* polyhedrons_facets)
{
    ((PolyhedralizationTetrahedralizationHandle*)handle)->AddPolyhedralizationTetrahedralizationInput(explicit_count, explicit_values, implicit_count, implicit_values, polyhedrons_count, polyhedrons, polyhedrons_facets_count, polyhedrons_facets);
}

extern "C" LIBRARY_EXPORT void CalculatePolyhedralizationTetrahedralization(void* handle)
{
    ((PolyhedralizationTetrahedralizationHandle*)handle)->CalculatePolyhedralizationTetrahedralization();
}

extern "C" LIBRARY_EXPORT uint32_t GetPolyhedralizationTetrahedralizationInsertedFacetsCentroidsCount(void* handle)
{
    return ((PolyhedralizationTetrahedralizationHandle*)handle)->GetOutputInsertedFacetsCentroidsCount();
}
extern "C" LIBRARY_EXPORT uint32_t* GetPolyhedralizationTetrahedralizationInsertedFacetsCentroids(void* handle)
{
    return ((PolyhedralizationTetrahedralizationHandle*)handle)->GetOutputInsertedFacetsCentroids();
}
extern "C" LIBRARY_EXPORT uint32_t GetPolyhedralizationTetrahedralizationInsertedPolyhedronsCentroidsCount(void* handle)
{
    return ((PolyhedralizationTetrahedralizationHandle*)handle)->GetOutputInsertedPolyhedronsCentroidsCount();
}
extern "C" LIBRARY_EXPORT uint32_t* GetPolyhedralizationTetrahedralizationInsertedPolyhedronsCentroids(void* handle)
{
    return ((PolyhedralizationTetrahedralizationHandle*)handle)->GetOutputInsertedPolyhedronsCentroids();
}
extern "C" LIBRARY_EXPORT uint32_t GetPolyhedralizationTetrahedralizationTetrahedronsCount(void* handle)
{
    return ((PolyhedralizationTetrahedralizationHandle*)handle)->GetOutputTetrahedronsCount();
}
extern "C" LIBRARY_EXPORT uint32_t* GetPolyhedralizationTetrahedralizationTetrahedrons(void* handle)
{
    return ((PolyhedralizationTetrahedralizationHandle*)handle)->GetOutputTetrahedrons();
}
