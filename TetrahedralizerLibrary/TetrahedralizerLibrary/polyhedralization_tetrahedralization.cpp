#include "polyhedralization_tetrahedralization.hpp"

void PolyhedralizationTetrahedralization::polyhedralization_tetrahedralization(PolyhedralizationTetrahedralizationInput* input, PolyhedralizationTetrahedralizationOutput* output)
{
    m_vertices = vector<genericPoint*>(input->m_vertices, input->m_vertices+input->m_vertices_count);
    m_polyhedrons = flat_array_to_nested_vector(input->m_polyhedrons, input->m_polyhedrons_count);
    m_facets = flat_array_to_nested_vector(input->m_polyhedrons_facets, input->m_polyhedrons_facets_count);
    m_triangulated_facets = vector<vector<uint32_t>>(input->m_polyhedrons_facets_count, vector<uint32_t>(0));
    m_facets_counters = vector<vector<uint32_t>>(input->m_polyhedrons_facets_count, vector<uint32_t>(0));
    
    // convert polyhedrons to tetrahedrons
    {
        for(uint32_t i=0; i<input->m_polyhedrons_count; i++)
        {
            if(4 == m_polyhedrons[i].size()) // polyhedron is a tetrahedron
            {
                m_u_set_i_0.clear();
                for(uint32_t j=0; j<m_polyhedrons[i].size(); j++)
                {
                    uint32_t f = m_polyhedrons[i][j];
                    for(uint32_t k=0; k<m_facets[f].size(); k++)
                    {
                        m_u_set_i_0.insert(m_facets[f][k]);
                    }
                }
                
                add_tetrahedron(m_u_set_i_0);
                continue;
            }
            
            // triangulate every facet and order each triangle
            for(uint32_t j=0; j<m_polyhedrons[i].size(); j++)
            {
                uint32_t f = m_polyhedrons[i][j];
                if(0 != m_triangulated_facets[f].size())
                {
                    continue;
                }
                for(uint32_t k=1; k<m_facets[f].size()-1; k++)
                {
                    uint32_t t0 = m_facets[f][0];
                    uint32_t t1 = m_facets[f][k];
                    uint32_t t2 = m_facets[f][k+1];
                    sort_ints(t0, t1, t2);
                    m_triangulated_facets[f].push_back(t0);
                    m_triangulated_facets[f].push_back(t1);
                    m_triangulated_facets[f].push_back(t2);
                    m_facets_counters[f].push_back(0);
                }
            }
            
            uint32_t connect_vertex = find_connect_vertex(i);
            if(UNDEFINED_VALUE != connect_vertex) // if polyhedron can be tetrahedralized by connecting a vertex to every triangulated facet
            {
                connect_polyhedron_with_vertex(i, connect_vertex);
                continue;
            }
            
            connect_polyhedron_with_centroid(i); // insert a new point at the centroid and connect it to all facets
        }
        
        // copy output
        {
            output->m_inserted_vertices_count = m_vertices.size() - input->m_vertices_count;
            output->m_inserted_vertices = new double[3*output->m_inserted_vertices_count];
            for(uint32_t i=input->m_vertices_count; i<m_vertices.size(); i++)
            {
                output->m_inserted_vertices[3*(i-input->m_vertices_count)+0] = m_vertices[i]->toExplicit3D().X();
                output->m_inserted_vertices[3*(i-input->m_vertices_count)+1] = m_vertices[i]->toExplicit3D().Y();
                output->m_inserted_vertices[3*(i-input->m_vertices_count)+2] = m_vertices[i]->toExplicit3D().Z();
                delete (explicitPoint3D*)m_vertices[i];
            }
            
            output->m_tetrahedrons = vector_to_array(m_result_tetrahedrons);
            output->m_tetrahedrons_count = m_result_tetrahedrons.size() / 4;
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
            for(uint32_t j=0; j<m_facets_counters[f].size(); j++)
            {
                m_facets_counters[f][j] = 0;
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
                
                m_facets_counters[f][j/3]++;
                polyhedron_has_triangle(polyhedron,t0,t1,v);
                polyhedron_has_triangle(polyhedron,t0,t2,v);
                polyhedron_has_triangle(polyhedron,t1,t2,v);
            }
        }
        
        // check facets counters
        for(uint32_t i=0; i<m_polyhedrons[polyhedron].size(); i++)
        {
            uint32_t f = m_polyhedrons[polyhedron][i];
            for(uint32_t j=0; j<m_facets_counters[f].size(); j++)
            {
                if(1 != m_facets_counters[f][j])
                {
                    goto NEXT_VERTEX;
                }
            }
        }
        return v;
        NEXT_VERTEX:
        continue;
        
        
//        m_u_set_ii_0.clear(); // get every incident triangle's oppsite edge
//        for(uint32_t i=0; i<m_polyhedrons[polyhedron].size(); i++)
//        {
//            uint32_t f = m_polyhedrons[polyhedron][i];
//            for(uint32_t j=0; j<m_triangulated_facets[f].size(); j+=3)
//            {
//                uint32_t t0 = m_triangulated_facets[f][j+0];
//                uint32_t t1 = m_triangulated_facets[f][j+1];
//                uint32_t t2 = m_triangulated_facets[f][j+2];
//                
//                if(v == t0)
//                {
//                    m_u_set_ii_0.insert(make_pair(t1, t2));
//                }
//                else if(v == t1)
//                {
//                    m_u_set_ii_0.insert(make_pair(t0, t2));
//                }
//                else if(v == t2)
//                {
//                    m_u_set_ii_0.insert(make_pair(t0, t1));
//                }
//            }
//        }
//        
//        for(auto [e0,e1] : m_u_set_ii_0) // do orient test with every oppsite vertex
//        {
//            for(uint32_t i=0; i<m_polyhedrons[polyhedron].size(); i++)
//            {
//                uint32_t f = m_polyhedrons[polyhedron][i];
//                for(uint32_t j=0; j<m_triangulated_facets[f].size(); j+=3)
//                {
//                    uint32_t t0 = m_triangulated_facets[f][j+0];
//                    uint32_t t1 = m_triangulated_facets[f][j+1];
//                    uint32_t t2 = m_triangulated_facets[f][j+2];
//                    uint32_t o = v;
//                    if((t0 == e0 || t0 == e1) && (t1 == e0 || t1 == e1))
//                    {
//                        o = t2;
//                    }
//                    else if((t0 == e0 || t0 == e1) && (t2 == e0 || t2 == e1))
//                    {
//                        o = t1;
//                    }
//                    else if((t1 == e0 || t1 == e1) && (t2 == e0 || t2 == e1))
//                    {
//                        o = t0;
//                    }
//                    if(o == v)
//                    {
//                        continue;
//                    }
//                    
//                    if(0 == orient3d(v, e0, e1, o, m_vertices.data()))
//                    {
//                        goto NEXT_VERTEX;
//                    }
//                }
//            }
//        }
//        return v;
//        NEXT_VERTEX:
//        continue;
    }
    
    return UNDEFINED_VALUE;
}

void PolyhedralizationTetrahedralization::connect_polyhedron_with_vertex(uint32_t polyhedron, uint32_t vertex)
{
    for(uint32_t i=0; i<m_polyhedrons[polyhedron].size(); i++)
    {
        uint32_t f = m_polyhedrons[polyhedron][i];
        for(uint32_t j=0; j<m_triangulated_facets[f].size(); j+=3)
        {
            uint32_t t0 = m_triangulated_facets[f][j+0];
            uint32_t t1 = m_triangulated_facets[f][j+1];
            uint32_t t2 = m_triangulated_facets[f][j+2];
            if(t0==vertex || t1==vertex || t2==vertex || 0 == orient3d(t0, t1, t2, vertex, m_vertices.data()))
            {
                continue;
            }
            add_tetrahedron(t0, t1, t2, vertex);
        }
    }
}
void PolyhedralizationTetrahedralization::connect_polyhedron_with_centroid(uint32_t polyhedron)
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
    
    double3 center(0.0,0.0,0.0);
    for(uint32_t v : m_u_set_i_0)
    {
        center += approximate_point(m_vertices[v]);
    }
    center /= (double)m_u_set_i_0.size();
    
    uint32_t p = m_vertices.size();
    m_vertices.push_back(new explicitPoint3D(center.x, center.y, center.z));
    
    for(uint32_t i=0; i<m_polyhedrons[polyhedron].size(); i++)
    {
        uint32_t f = m_polyhedrons[polyhedron][i];
        for(uint32_t j=0; j<m_triangulated_facets[f].size(); j+=3)
        {
            uint32_t t0 = m_triangulated_facets[f][j+0];
            uint32_t t1 = m_triangulated_facets[f][j+1];
            uint32_t t2 = m_triangulated_facets[f][j+2];

            add_tetrahedron(t0, t1, t2, p);
        }
    }
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
                m_facets_counters[f][j/3]++;
            }
        }
    }
    return true;
}

void PolyhedralizationTetrahedralization::add_tetrahedron(unordered_set<uint32_t>& u_set)
{
    auto it = m_u_set_i_0.begin();
    uint32_t t0 = *it++;
    uint32_t t1 = *it++;
    uint32_t t2 = *it++;
    uint32_t t3 = *it;
    add_tetrahedron(t0,t1,t2,t3);
}

void PolyhedralizationTetrahedralization::add_tetrahedron(uint32_t t0,uint32_t t1,uint32_t t2,uint32_t t3)
{
    if(1 != orient3d(t0,t1,t2,t3,m_vertices.data()))
    {
        swap(t2,t3);
    }
    m_result_tetrahedrons.push_back(t0);
    m_result_tetrahedrons.push_back(t1);
    m_result_tetrahedrons.push_back(t2);
    m_result_tetrahedrons.push_back(t3);
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
    m_output->m_inserted_vertices = nullptr;
    m_output->m_inserted_vertices_count = 0;
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
    delete[] m_output->m_inserted_vertices;
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


uint32_t PolyhedralizationTetrahedralizationHandle::GetOutputInsertedVerticesCount()
{
    return m_output->m_inserted_vertices_count;
}
double* PolyhedralizationTetrahedralizationHandle::GetOutputInsertedVertices()
{
    return m_output->m_inserted_vertices;
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

extern "C" LIBRARY_EXPORT uint32_t GetPolyhedralizationTetrahedralizationInsertedVerticesCount(void* handle)
{
    return ((PolyhedralizationTetrahedralizationHandle*)handle)->GetOutputInsertedVerticesCount();
}
extern "C" LIBRARY_EXPORT double* GetPolyhedralizationTetrahedralizationInsertedVertices(void* handle)
{
    return ((PolyhedralizationTetrahedralizationHandle*)handle)->GetOutputInsertedVertices();
}
extern "C" LIBRARY_EXPORT uint32_t GetPolyhedralizationTetrahedralizationTetrahedronsCount(void* handle)
{
    return ((PolyhedralizationTetrahedralizationHandle*)handle)->GetOutputTetrahedronsCount();
}
extern "C" LIBRARY_EXPORT uint32_t* GetPolyhedralizationTetrahedralizationTetrahedrons(void* handle)
{
    return ((PolyhedralizationTetrahedralizationHandle*)handle)->GetOutputTetrahedrons();
}
