#include "polyhedralization_tetrahedralization.hpp"

void PolyhedralizationTetrahedralization::polyhedralization_tetrahedralization(PolyhedralizationTetrahedralizationInput* input, PolyhedralizationTetrahedralizationOutput* output)
{
    // build nested vector of polyhedrons facets
    vector<vector<uint32_t>> polyhedrons_facets = vector<vector<uint32_t>>(input->m_polyhedrons_facets_count);
    {
        uint32_t p = 0;
        for(uint32_t i=0; i<input->m_polyhedrons_facets_count; i++)
        {
            uint32_t n=input->m_polyhedrons_facets[p];
            for(uint32_t j=p+1; j<p+n+1; j++)
            {
                polyhedrons_facets[i].push_back(input->m_polyhedrons_facets[j]);
            }
            p+=n+1;
        }
    }
    
    vector<uint32_t> res;
    // convert polyhedrons to tetrahedrons
    {
        uint32_t p = 0;
        for(uint32_t i=0; i<input->m_polyhedrons_count; i++)
        {
            m_vector_i_0.clear(); // store polyhedron facets
            uint32_t n=input->m_polyhedrons[p];
            for(uint32_t j=p+1; j<p+n+1; j++)
            {
                m_vector_i_0.push_back(input->m_polyhedrons[j]);
            }
            p+=n+1;
            
            uint32_t v = polyhedrons_facets[m_vector_i_0.back()][0];
            m_vector_i_0.pop_back();
            for(uint32_t f : m_vector_i_0)
            {
                if(polyhedrons_facets[f].end() != find(polyhedrons_facets[f].begin(), polyhedrons_facets[f].end(), v)) // if v is on the facet
                {
                    continue;
                }
                
                int o = 0;
                for(uint32_t j=2; j<polyhedrons_facets[f].size(); j++)
                {
                    if(is_collinear(polyhedrons_facets[f][0],polyhedrons_facets[f][1],polyhedrons_facets[f][j], input->m_vertices))
                    {
                        continue;
                    }
                    o = orient3d(polyhedrons_facets[f][0],polyhedrons_facets[f][1],polyhedrons_facets[f][j], v, input->m_vertices);
                    break;
                }

                if(0 == o) // v on the same plane as the facet
                {
                    continue;
                }
                
                if(1 == o)
                {
                    for(uint32_t j=1; j<polyhedrons_facets[f].size()-1; j++)
                    {
                        res.push_back(polyhedrons_facets[f][0]);
                        res.push_back(polyhedrons_facets[f][j]);
                        res.push_back(polyhedrons_facets[f][j+1]);
                        res.push_back(v);
                    }
                }
                else
                {
                    for(uint32_t j=1; j<polyhedrons_facets[f].size()-1; j++)
                    {
                        res.push_back(polyhedrons_facets[f][0]);
                        res.push_back(polyhedrons_facets[f][j+1]);
                        res.push_back(polyhedrons_facets[f][j]);
                        res.push_back(v);
                    }
                }
            }
        }
        
        // copy output
        {
            vector_to_array(res, output->m_tetrahedrons);
            output->m_tetrahedrons_count = res.size() / 4;
        }
    }
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
    delete[] m_output->m_tetrahedrons;
    delete m_output;
    delete m_polyhedralizationTetrahedralization;
}

void PolyhedralizationTetrahedralizationHandle::AddPolyhedralizationTetrahedralizationInput(uint32_t explicit_count, double* explicit_values, uint32_t implicit_count, uint32_t* implicit_values, uint32_t polyhedrons_count, uint32_t* polyhedrons, uint32_t polyhedrons_facets_count, uint32_t* polyhedrons_facets)
{
    create_vertices(explicit_count, explicit_values, implicit_count, implicit_values, m_input->m_vertices, m_input->m_vertices_count);

    vector<uint32_t> vec = read_flat_vector(polyhedrons_count, polyhedrons);
    vector_to_array(vec , m_input->m_polyhedrons);
    m_input->m_polyhedrons_count = polyhedrons_count;
    
    vec = read_flat_vector(polyhedrons_facets_count, polyhedrons_facets);
    vector_to_array(vec , m_input->m_polyhedrons_facets);
    m_input->m_polyhedrons_facets_count = polyhedrons_facets_count;
}

void PolyhedralizationTetrahedralizationHandle::CalculatePolyhedralizationTetrahedralization()
{
    m_polyhedralizationTetrahedralization->polyhedralization_tetrahedralization(m_input, m_output);
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

extern "C" LIBRARY_EXPORT uint32_t GetPolyhedralizationTetrahedralizationTetrahedronsCount(void* handle)
{
    return ((PolyhedralizationTetrahedralizationHandle*)handle)->GetOutputTetrahedronsCount();
}
extern "C" LIBRARY_EXPORT uint32_t* GetPolyhedralizationTetrahedralizationTetrahedrons(void* handle)
{
    return ((PolyhedralizationTetrahedralizationHandle*)handle)->GetOutputTetrahedrons();
}
