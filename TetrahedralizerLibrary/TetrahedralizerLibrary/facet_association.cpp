#include "facet_association.hpp"

void FacetAssociation::facet_association(FacetAssociationInput* input, FacetAssociationOutput* output)
{
    vector<vector<uint32_t>> facets = flat_array_to_nested_vector(input->m_facets, input->m_facets_count);
    vector<vector<uint32_t>> constraints; // every vector are the coplanar constrains
    vector<vector<vector<uint32_t>>> vertices_mapping; // duplicated per facet
    vector<uint32_t> centroids_mapping = vector<uint32_t>(input->m_facets_count, UNDEFINED_VALUE);
    
    for(uint32_t i=0; i<input->m_constraints_count; i++)
    {
        uint32_t c0 = input->m_constraints[3*i+0];
        uint32_t c1 = input->m_constraints[3*i+1];
        uint32_t c2 = input->m_constraints[3*i+2];
        
        // skip degenerate triangles
        if(c0 == c1 || c0 == c2 || c1 == c2 || is_collinear(c0, c1, c2, input->m_vertices))
        {
            continue;
        }
        
        bool is_new_plane = true;
        for(uint32_t j=0; j<constraints.size(); j++)
        {
            uint32_t t0 = input->m_constraints[3*constraints[j][0]+0];
            uint32_t t1 = input->m_constraints[3*constraints[j][0]+1];
            uint32_t t2 = input->m_constraints[3*constraints[j][0]+2];
            if(0 != orient3d(c0,c1,c2,t0,input->m_vertices) ||
               0 != orient3d(c0,c1,c2,t1,input->m_vertices) ||
               0 != orient3d(c0,c1,c2,t2,input->m_vertices))
            {
                continue;
            }
            constraints[j].push_back(i);
            is_new_plane = false;
            break;
        }
        if(is_new_plane)
        {
            constraints.push_back(vector<uint32_t>{i});
        }
    }
    
    for(uint32_t i=0; i<facets.size(); i++)
    {
        vertices_mapping.push_back(vector<vector<uint32_t>>(facets[i].size()));
        uint32_t f0(UNDEFINED_VALUE),f1(UNDEFINED_VALUE),f2(UNDEFINED_VALUE);
        for(uint32_t j=0; j<facets[i].size(); j++)
        {
            uint32_t p = 0==j ? facets[i].size()-1:j-1;
            uint32_t n = facets[i].size()-1==j ? 0:j+1;
            f0 = facets[i][p];
            f1 = facets[i][j];
            f2 = facets[i][n];
            if(!is_collinear(f0, f1, f2, input->m_vertices))
            {
                break;
            }
        }
        for(uint32_t j=0; j<constraints.size(); j++)
        {
            uint32_t c0 = input->m_constraints[3*constraints[j][0]+0];
            uint32_t c1 = input->m_constraints[3*constraints[j][0]+1];
            uint32_t c2 = input->m_constraints[3*constraints[j][0]+2];
            if(0 != orient3d(f0,f1,f2,c0,input->m_vertices) ||
               0 != orient3d(f0,f1,f2,c1,input->m_vertices) ||
               0 != orient3d(f0,f1,f2,c2,input->m_vertices))
            {
                continue;
            }
            
            double3 centroid(0.0, 0.0, 0.0);
            for(uint32_t l=0; l<facets[i].size(); l++)
            {
                centroid += approximate_point(input->m_vertices[facets[i][l]]);
            }
            centroid /= (double)facets[i].size();
            explicitPoint3D cen(centroid.x,centroid.y,centroid.z);
            
            for(uint32_t k=0; k<constraints[j].size(); k++)
            {
                c0 = input->m_constraints[3*constraints[j][k]+0];
                c1 = input->m_constraints[3*constraints[j][k]+1];
                c2 = input->m_constraints[3*constraints[j][k]+2];
                
                if(UNDEFINED_VALUE==centroids_mapping[i] && genericPoint::pointInTriangle(cen,*input->m_vertices[c0],*input->m_vertices[c1],*input->m_vertices[c2]))
                {
                    centroids_mapping[i] = constraints[j][k];
                }
                for(uint32_t l=0; l<facets[i].size(); l++)
                {
                    uint32_t f = facets[i][l];
                    if(!vertex_in_triangle(f, c0, c1, c2, input->m_vertices))
                    {
                        continue;
                    }
                    vertices_mapping[i][l].push_back(constraints[j][k]);
                }
            }
            break;
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
        output->m_facets_centroids_mapping = vector_to_array(centroids_mapping);
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
    m_output->m_facets_centroids_mapping = nullptr;
    m_facetAssociation = new FacetAssociation();
}
void FacetAssociationHandle::Dispose()
{
    delete_vertices(m_input->m_vertices, m_input->m_vertices_count);
    delete[] m_input->m_facets;
    delete[] m_input->m_constraints;
    delete m_input;
    delete[] m_output->m_facets_vertices_mapping;
    delete[] m_output->m_facets_centroids_mapping;
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

uint32_t* FacetAssociationHandle::GetFacetsVerticesMapping()
{
    return m_output->m_facets_vertices_mapping;
}
uint32_t* FacetAssociationHandle::GetFacetsCentroidsMapping()
{
    return m_output->m_facets_centroids_mapping;
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

extern "C" LIBRARY_EXPORT uint32_t* GetFacetAssociationFacetsVerticesMapping(void* handle)
{
    return ((FacetAssociationHandle*)handle)->GetFacetsVerticesMapping();
}
extern "C" LIBRARY_EXPORT uint32_t* GetFacetAssociationFacetsCentroidsMapping(void* handle)
{
    return ((FacetAssociationHandle*)handle)->GetFacetsCentroidsMapping();
}
