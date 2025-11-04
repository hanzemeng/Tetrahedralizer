#include "facet_association.hpp"

void FacetAssociation::facet_association(FacetAssociationInput* input, FacetAssociationOutput* output)
{
    vector<vector<uint32_t>> facets = flat_array_to_nested_vector(input->m_facets, input->m_facets_count);
    vector<uint32_t> constraints(input->m_constraints, input->m_constraints+3*input->m_constraints_count);
    
    vector<vector<vector<uint32_t>>> vertices_mapping; // duplicated per facet
    
    for(uint32_t i=0; i<constraints.size(); i+=3)
    {
        uint32_t c0 = constraints[i+0];
        uint32_t c1 = constraints[i+1];
        uint32_t c2 = constraints[i+2];
        
        // skip degenerate triangles
        if(UNDEFINED_VALUE == c0 || c0 == c1 || c0 == c2 || c1 == c2 || is_collinear(c0, c1, c2, input->m_vertices))
        {
            constraints[i+0] = UNDEFINED_VALUE;
        }
    }
    
    for(uint32_t i=0; i<facets.size(); i++)
    {
        vertices_mapping.push_back(vector<vector<uint32_t>>(facets[i].size()));
        uint32_t f0 = facets[i][0];
        uint32_t f1 = facets[i][1];
        uint32_t f2 = facets[i][2];
        for(uint32_t j=0; j<constraints.size(); j+=3)
        {
            uint32_t c0 = constraints[j+0];
            uint32_t c1 = constraints[j+1];
            uint32_t c2 = constraints[j+2];
            
            if(UNDEFINED_VALUE == c0 ||
               0 != orient3d(f0,f1,f2,c0,input->m_vertices) ||
               0 != orient3d(f0,f1,f2,c1,input->m_vertices) ||
               0 != orient3d(f0,f1,f2,c2,input->m_vertices))
            {
                continue;
            }
            
            for(uint32_t k=0; k<facets[i].size(); k++)
            {
                uint32_t f = facets[i][k];
                if(!vertex_in_triangle(f, c0, c1, c2, input->m_vertices))
                {
                    continue;
                }
                vertices_mapping[i][k].push_back(j/3);
            }
        }
    }
    
    {
        vector<uint32_t> temp;
        for(uint32_t i=0; i<vertices_mapping.size(); i++)
        {
            for(uint32_t j=0; j<vertices_mapping[i].size(); j++)
            {
                temp.push_back(vertices_mapping[i][j].size());
                for(uint32_t k=0; k<vertices_mapping[i][j].size(); k++)
                {
                    temp.push_back(vertices_mapping[i][j][k]);
                }
            }
        }
        
        output->m_facets_vertices_mapping = vector_to_array(temp);
    }
}


FacetAssociationHandle::FacetAssociationHandle()
{
    m_input = new FacetAssociationInput();
    m_input->m_vertices_count = 0;
    m_input->m_facets_count = 0;
    m_input->m_constraints_count = 0;
    m_input->m_vertices = nullptr;
    m_input->m_facets = nullptr;
    m_input->m_constraints = nullptr;
    m_output = new FacetAssociationOutput();
    m_output->m_facets_vertices_mapping = nullptr;
    m_facetAssociation = new FacetAssociation();
}
void FacetAssociationHandle::Dispose()
{
    delete_vertices(m_input->m_vertices, m_input->m_vertices_count);
    delete[] m_input->m_facets;
    delete[] m_input->m_constraints;
    delete m_input;
    delete[] m_output->m_facets_vertices_mapping;
    delete m_output;
    delete m_facetAssociation;
}


void FacetAssociationHandle::AddFacetAssociationInput(uint32_t explicit_count, double* explicit_values, uint32_t implicit_count, uint32_t* implicit_values, uint32_t facets_count, uint32_t* facets, uint32_t constraints_count, uint32_t* constraints)
{
    create_vertices(explicit_count, explicit_values, implicit_count, implicit_values, m_input->m_vertices, m_input->m_vertices_count);
    m_input->m_facets_count = facets_count;
    vector<uint32_t> temp = flat_array_to_vector(facets, facets_count);
    m_input->m_facets = vector_to_array(temp);
    m_input->m_constraints_count = constraints_count;
    m_input->m_constraints = duplicate_array(constraints, 3*constraints_count);
}
void FacetAssociationHandle::CalculateFacetAssociation()
{
    m_facetAssociation->facet_association(m_input, m_output);
}

uint32_t* FacetAssociationHandle::GetOutputFacetAssociation()
{
    return m_output->m_facets_vertices_mapping;
}


extern "C" LIBRARY_EXPORT void* CreateFacetAssociationHandle()
{
    return new FacetAssociationHandle();
}
extern "C" LIBRARY_EXPORT void DisposeFacetAssociationHandle(void* handle)
{
    ((FacetAssociationHandle*)handle)->Dispose();
    delete ((FacetAssociationHandle*)handle);
}

extern "C" LIBRARY_EXPORT void AddFacetAssociationInput(void* handle, uint32_t explicit_count, double* explicit_values, uint32_t implicit_count, uint32_t* implicit_values, uint32_t facets_count, uint32_t* facets, uint32_t constraints_count, uint32_t* constraints)
{
    ((FacetAssociationHandle*)handle)->AddFacetAssociationInput(explicit_count, explicit_values, implicit_count, implicit_values, facets_count, facets, constraints_count, constraints);
}

extern "C" LIBRARY_EXPORT void CalculateFacetAssociation(void* handle)
{
    ((FacetAssociationHandle*)handle)->CalculateFacetAssociation();
}

extern "C" LIBRARY_EXPORT uint32_t* GetOutputFacetAssociation(void* handle)
{
    return ((FacetAssociationHandle*)handle)->GetOutputFacetAssociation();
}
