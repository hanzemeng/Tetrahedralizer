#include "facet_association.hpp"

void FacetAssociation::facet_association(FacetAssociationInput* input, FacetAssociationOutput* output)
{
    m_vertices = input->m_vertices;
    m_facets = flat_array_to_nested_vector(input->m_facets, input->m_facets_count);
    m_constraints = input->m_constraints;
    m_centroids_mapping = vector<uint32_t>(input->m_facets_count, UNDEFINED_VALUE);
    
    // group coplanar facets
    for(uint32_t i=0; i<m_facets.size(); i++)
    {
        uint32_t t0,t1,t2;
        get_non_collinear_vertices(m_facets[i], input->m_vertices, t0, t1, t2);
        if(UNDEFINED_VALUE == t0)
        {
            continue;
        }
        
        bool is_new_plane = true;
        for(uint32_t j=0; j<m_coplanar_facets.size(); j++)
        {
            uint32_t p0,p1,p2;
            get_non_collinear_vertices(m_facets[m_coplanar_facets[j][0]], input->m_vertices, p0, p1, p2);
            if(0 != orient3d(p0,p1,p2,t0,input->m_vertices) ||
               0 != orient3d(p0,p1,p2,t1,input->m_vertices) ||
               0 != orient3d(p0,p1,p2,t2,input->m_vertices))
            {
                continue;
            }
            m_coplanar_facets[j].push_back(i);
            is_new_plane = false;
            break;
        }
        if(is_new_plane)
        {
            m_coplanar_facets.push_back(vector<uint32_t>{i});
        }
    }
    
    // group coplanar constraints
    for(uint32_t i=0; i<input->m_constraints_count; i++)
    {
        uint32_t c0 = input->m_constraints[3*i+0];
        uint32_t c1 = input->m_constraints[3*i+1];
        uint32_t c2 = input->m_constraints[3*i+2];
        
        if(c0 == c1 || c0 == c2 || c1 == c2 || is_collinear(c0, c1, c2, input->m_vertices))
        {
            continue;
        }
        
        bool is_new_plane = true;
        for(uint32_t j=0; j<m_coplanar_constraints.size(); j++)
        {
            uint32_t t0 = input->m_constraints[3*m_coplanar_constraints[j][0]+0];
            uint32_t t1 = input->m_constraints[3*m_coplanar_constraints[j][0]+1];
            uint32_t t2 = input->m_constraints[3*m_coplanar_constraints[j][0]+2];
            if(0 != orient3d(c0,c1,c2,t0,input->m_vertices) ||
               0 != orient3d(c0,c1,c2,t1,input->m_vertices) ||
               0 != orient3d(c0,c1,c2,t2,input->m_vertices))
            {
                continue;
            }
            m_coplanar_constraints[j].push_back(i);
            is_new_plane = false;
            break;
        }
        if(is_new_plane)
        {
            m_coplanar_constraints.push_back(vector<uint32_t>{i});
        }
    }
    
    for(uint32_t i=0; i<m_facets.size(); i++)
    {
        m_vertices_mapping.push_back(vector<vector<uint32_t>>(m_facets[i].size()));
    }
    
    for(uint32_t i=0; i<m_coplanar_facets.size(); i++)
    {
        uint32_t f0,f1,f2;
        get_non_collinear_vertices(m_facets[m_coplanar_facets[i][0]], input->m_vertices, f0, f1, f2);
        
        for(uint32_t j=0; j<m_coplanar_constraints.size(); j++)
        {
            uint32_t c0 = m_constraints[3*m_coplanar_constraints[j][0]+0];
            uint32_t c1 = m_constraints[3*m_coplanar_constraints[j][0]+1];
            uint32_t c2 = m_constraints[3*m_coplanar_constraints[j][0]+2];
            if(0 != orient3d(f0,f1,f2,c0,input->m_vertices) ||
               0 != orient3d(f0,f1,f2,c1,input->m_vertices) ||
               0 != orient3d(f0,f1,f2,c2,input->m_vertices))
            {
                continue;
            }
            
            facet_constraint_association(i, j);
            break;
        }
    }
    
    {
        vector<uint32_t> temp;
        for(uint32_t i=0; i<m_vertices_mapping.size(); i++)
        {
            for(uint32_t j=0; j<m_vertices_mapping[i].size(); j++)
            {
                temp.push_back(m_vertices_mapping[i][j].size());
                for(uint32_t k=0; k<m_vertices_mapping[i][j].size(); k++)
                {
                    temp.push_back(m_vertices_mapping[i][j][k]);
                }
            }
        }
        output->m_facets_vertices_mapping = vector_to_array(temp);
        output->m_facets_centroids_mapping = vector_to_array(m_centroids_mapping);
    }
}

void FacetAssociation::facet_constraint_association(uint32_t facet_index, uint32_t constraint_index)
{
    for(uint32_t facet : m_coplanar_facets[facet_index])
    {
        double3 centroid(0.0, 0.0, 0.0);
        for(uint32_t i=0; i<m_facets[facet].size(); i++)
        {
            centroid += approximate_point(m_vertices[m_facets[facet][i]]);
        }
        centroid /= (double)m_facets[facet].size();
        explicitPoint3D cen(centroid.x,centroid.y,centroid.z);
        
        for(uint32_t constraint : m_coplanar_constraints[constraint_index])
        {
            uint32_t c0 = m_constraints[3*constraint+0];
            uint32_t c1 = m_constraints[3*constraint+1];
            uint32_t c2 = m_constraints[3*constraint+2];
            
            if(UNDEFINED_VALUE==m_centroids_mapping[facet] && genericPoint::pointInTriangle(cen,*m_vertices[c0],*m_vertices[c1],*m_vertices[c2]))
            {
                m_centroids_mapping[facet] = constraint;
            }
            for(uint32_t i=0; i<m_facets[facet].size(); i++)
            {
                uint32_t f = m_facets[facet][i];
                if(!vertex_in_triangle(f, c0, c1, c2, m_vertices))
                {
                    continue;
                }
                m_vertices_mapping[facet][i].push_back(constraint);
            }
        }
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
