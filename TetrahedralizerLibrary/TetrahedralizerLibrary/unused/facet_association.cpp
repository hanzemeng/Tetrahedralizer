#include "facet_association.hpp"

void FacetAssociation::facet_association(FacetAssociationInput* input, FacetAssociationOutput* output)
{
    // calculate tetrahedron neighbors and vertices incidents
    Tetrahedralization tetrahedralization(input->m_tetrahedrons, input->m_tetrahedrons_count);
    
    vector<uint32_t> constraints_neighbors = vector<uint32_t>(3*input->m_constraints_count, UNDEFINED_VALUE); // order in 01,12,20
    // calculate constraint neighbors
    {
        m_u_map_ii_i_0.clear();
        for(uint32_t i=0; i<3*input->m_constraints_count; i+=3)
        {
            for(uint32_t j=0; j<3; j++)
            {
                uint32_t p0 = input->m_constraints[i+j];
                uint32_t p1 = input->m_constraints[i+(j+1)%3];
                sort_ints(p0,p1);
                
                auto it = m_u_map_ii_i_0.find(make_pair(p0, p1));
                if(m_u_map_ii_i_0.end() != it)
                {
                    uint32_t n = it->second;
                    constraints_neighbors[i+j] = n;
                    constraints_neighbors[n] = i+j;
                    m_u_map_ii_i_0.erase(it);
                }
                else
                {
                    m_u_map_ii_i_0[make_pair(p0,p1)] = i+j;
                }
            }
        }
    }

    vector<vector<uint32_t>> coplanar_constraints;
    // calculate coplanar constraints (that are also connected)
    {
        m_vector_i_0.resize(input->m_constraints_count); // record the facet index of every constraint
        for(uint32_t i=0; i<input->m_constraints_count; i++)
        {
            m_vector_i_0[i] = UNDEFINED_VALUE;
        }
        
        uint32_t facet_count = 0;
        for(uint32_t i=0; i<input->m_constraints_count; i++)
        {
            if(UNDEFINED_VALUE != m_vector_i_0[i])
            {
                continue;
            }

            clear_queue(m_queue_i_0);
            m_queue_i_0.push(i);
            while(!m_queue_i_0.empty())
            {
                uint32_t c = m_queue_i_0.front();
                m_queue_i_0.pop();
                
                if(UNDEFINED_VALUE != m_vector_i_0[c])
                {
                    continue;
                }
                m_vector_i_0[c] = facet_count;
                
                uint32_t p0(input->m_constraints[3*c+0]),p1(input->m_constraints[3*c+1]),p2(input->m_constraints[3*c+2]);
                for(uint32_t j=0; j<3; j++)
                {
                    uint32_t n = constraints_neighbors[3*c+j];
                    if(UNDEFINED_VALUE == n)
                    {
                        continue;
                    }
                    uint32_t r = n%3;
                    uint32_t p3 = input->m_constraints[n-r + (r+2)%3];
                    if(0 != orient3d(p0,p1,p2,p3, input->m_vertices))
                    {
                        continue;
                    }
                    m_queue_i_0.push(n/3);
                }
            }
            facet_count++;
        }
        
        coplanar_constraints.resize(facet_count);
        for(uint32_t i=0; i<input->m_constraints_count; i++)
        {
            uint32_t t0(input->m_constraints[3*i+0]),t1(input->m_constraints[3*i+1]),t2(input->m_constraints[3*i+2]);
            if(t0 == t1 || t0 == t2 || t1 == t2 || is_collinear(t0, t1, t2, input->m_vertices))
            {
                continue;
            }
            coplanar_constraints[m_vector_i_0[i]].push_back(i);
        }
    }
    
    // produce output
    {
        output->m_tetrahedrons_facets_mapping = new uint32_t[12*input->m_tetrahedrons_count];
        for(uint32_t i=0; i<12*input->m_tetrahedrons_count; i++)
        {
            output->m_tetrahedrons_facets_mapping[i] = UNDEFINED_VALUE;
        }
        
        for(uint32_t i=0; i<coplanar_constraints.size(); i++)
        {
            m_u_map_i_i_0.clear(); // stores vertices orients
            for(uint32_t j=0; j<coplanar_constraints[i].size(); j++)
            {
                uint32_t c = coplanar_constraints[i][j];
                
                uint32_t c0 = input->m_constraints[3*c+0];
                uint32_t c1 = input->m_constraints[3*c+1];
                uint32_t c2 = input->m_constraints[3*c+2];

                m_u_set_i_0.clear(); // stores the tetrahedrons that incident at one of the point of the cth constraint
                m_u_set_i_1.clear(); // stores visited tetrahedrons
                m_u_set_i_1.insert(UNDEFINED_VALUE);
                clear_queue(m_queue_i_0); // stores tetrahedrons to be visited
                m_queue_i_0.push(tetrahedralization.get_vertex_incident_tetrahedron(c0));
                m_queue_i_0.push(tetrahedralization.get_vertex_incident_tetrahedron(c1));
                m_queue_i_0.push(tetrahedralization.get_vertex_incident_tetrahedron(c2));

                while(!m_queue_i_0.empty())
                {
                    uint32_t t = m_queue_i_0.front();
                    m_queue_i_0.pop();
                    if(m_u_set_i_1.end() != m_u_set_i_1.find(t))
                    {
                        continue;
                    }
                    m_u_set_i_1.insert(t);

                    if(!tetrahedralization.tetrahedron_contains_vertex(t, c0) &&
                       !tetrahedralization.tetrahedron_contains_vertex(t, c1) &&
                       !tetrahedralization.tetrahedron_contains_vertex(t, c2))
                    {
                        continue;
                    }
                    m_u_set_i_0.insert(t);
                    uint32_t n;
                    for(uint32_t f=0; f<4; f++)
                    {
                        tetrahedralization.get_tetrahedron_neighbor(t, f, n);
                        m_queue_i_0.push(n);
                    }
                }

                m_u_set_i_1.clear(); // stores visited tetrahedrons
                m_u_set_i_1.insert(UNDEFINED_VALUE);
                for(uint32_t t : m_u_set_i_0)
                {
                    m_queue_i_0.push(t); // stores tetrahedrons to be visited
                }
                
                while(!m_queue_i_0.empty())
                {
                    uint32_t t = m_queue_i_0.front();
                    m_queue_i_0.pop();

                    if(m_u_set_i_1.end() != m_u_set_i_1.find(t))
                    {
                        continue;
                    }
                    m_u_set_i_1.insert(t);

                    uint32_t t0 = input->m_tetrahedrons[t+0];
                    uint32_t t1 = input->m_tetrahedrons[t+1];
                    uint32_t t2 = input->m_tetrahedrons[t+2];
                    uint32_t t3 = input->m_tetrahedrons[t+3];
                    
                    if(m_u_map_i_i_0.end() == m_u_map_i_i_0.find(t0))
                    {
                        m_u_map_i_i_0[t0] = orient3d(c0,c1,c2,t0,input->m_vertices);
                    }
                    if(m_u_map_i_i_0.end() == m_u_map_i_i_0.find(t1))
                    {
                        m_u_map_i_i_0[t1] = orient3d(c0,c1,c2,t1,input->m_vertices);
                    }
                    if(m_u_map_i_i_0.end() == m_u_map_i_i_0.find(t2))
                    {
                        m_u_map_i_i_0[t2] = orient3d(c0,c1,c2,t2,input->m_vertices);
                    }
                    if(m_u_map_i_i_0.end() == m_u_map_i_i_0.find(t3))
                    {
                        m_u_map_i_i_0[t3] = orient3d(c0,c1,c2,t3,input->m_vertices);
                    }
                    
                    int o0 = m_u_map_i_i_0[t0];
                    int o1 = m_u_map_i_i_0[t1];
                    int o2 = m_u_map_i_i_0[t2];
                    int o3 = m_u_map_i_i_0[t3];
                    
                    if(o0 == o1 && o1 == o2 && o2 == o3 && 0 != o0)
                    {
                        continue;
                    }
                    
                    for(uint32_t k=0; k<4; k++)
                    {
                        uint32_t n;
                        tetrahedralization.get_tetrahedron_neighbor(t, k, n);
                        m_queue_i_0.push(n);
                    }
                    m_u_set_i_0.insert(t);
                }

                m_u_set_i_0.erase(UNDEFINED_VALUE);
                for(uint32_t t : m_u_set_i_0)
                {
                    for(uint32_t k=0; k<4; k++)
                    {
                        uint32_t ps[3];
                        tetrahedralization.get_tetrahedron_face(t, k, ps[0], ps[1], ps[2]);
                        bool is_coplanar_face = true;
                        for(uint32_t l=0; l<3; l++)
                        {
                            if(m_u_map_i_i_0.end() == m_u_map_i_i_0.find(ps[l]))
                            {
                                m_u_map_i_i_0[ps[l]] = orient3d(c0,c1,c2,ps[l],input->m_vertices);
                            }
                            
                            if(0 != m_u_map_i_i_0[ps[l]])
                            {
                                is_coplanar_face = false;
                                break;
                            }
                        }
                        if(!is_coplanar_face)
                        {
                            continue;
                        }
                        
                        for(uint32_t l=0; l<3; l++)
                        {
                            if(UNDEFINED_VALUE != output->m_tetrahedrons_facets_mapping[3*t+3*k+l])
                            {
                                continue;
                            }
                            if(ps[l] == c0 || ps[l] == c1 || ps[l] == c2 ||
                               vertex_in_segment(ps[l], c0, c1, input->m_vertices) ||
                               vertex_in_segment(ps[l], c1, c2, input->m_vertices) ||
                               vertex_in_segment(ps[l], c2, c0, input->m_vertices) ||
                               vertex_in_triangle(ps[l], c0, c1, c2, input->m_vertices))
                            {
                                output->m_tetrahedrons_facets_mapping[3*t+3*k+l] = c;
                            }
                        }
                    }
                }
            }
        }
    }
}


FacetAssociationHandle::FacetAssociationHandle()
{
    m_input = new FacetAssociationInput();
    m_input->m_vertices_count = 0;
    m_input->m_tetrahedrons_count = 0;
    m_input->m_constraints_count = 0;
    m_input->m_vertices = nullptr;
    m_input->m_tetrahedrons = nullptr;
    m_input->m_constraints = nullptr;
    m_output = new FacetAssociationOutput();
    m_output->m_tetrahedrons_facets_mapping = nullptr;
    m_facetAssociation = new FacetAssociation();
}
void FacetAssociationHandle::Dispose()
{
    delete_vertices(m_input->m_vertices, m_input->m_vertices_count);
    delete[] m_input->m_tetrahedrons;
    delete[] m_input->m_constraints;
    delete m_input;
    delete[] m_output->m_tetrahedrons_facets_mapping;
    delete m_output;
    delete m_facetAssociation;
}


void FacetAssociationHandle::AddFacetAssociationInput(uint32_t explicit_count, double* explicit_values, uint32_t implicit_count, uint32_t* implicit_values, uint32_t tetrahedrons_count, uint32_t* tetrahedrons, uint32_t constraints_count, uint32_t* constraints)
{
    create_vertices(explicit_count, explicit_values, implicit_count, implicit_values, m_input->m_vertices, m_input->m_vertices_count);
    m_input->m_tetrahedrons_count = tetrahedrons_count;
    m_input->m_tetrahedrons = new uint32_t[4*tetrahedrons_count];
    for(uint32_t i=0; i<4*tetrahedrons_count; i++)
    {
        m_input->m_tetrahedrons[i] = tetrahedrons[i];
    }
    
    m_input->m_constraints_count = constraints_count;
    m_input->m_constraints = new uint32_t[3*constraints_count];
    for(uint32_t i=0; i<3*constraints_count; i++)
    {
        m_input->m_constraints[i] = constraints[i];
    }
}
void FacetAssociationHandle::CalculateFacetAssociation()
{
    m_facetAssociation->facet_association(m_input, m_output);
}

uint32_t* FacetAssociationHandle::GetOutputFacetAssociation()
{
    return m_output->m_tetrahedrons_facets_mapping;
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

extern "C" LIBRARY_EXPORT void AddFacetAssociationInput(void* handle, uint32_t explicit_count, double* explicit_values, uint32_t implicit_count, uint32_t* implicit_values, uint32_t tetrahedron_count, uint32_t* tetrahedrons, uint32_t constraints_count, uint32_t* constraints)
{
    ((FacetAssociationHandle*)handle)->AddFacetAssociationInput(explicit_count, explicit_values, implicit_count, implicit_values, tetrahedron_count, tetrahedrons, constraints_count, constraints);
}

extern "C" LIBRARY_EXPORT void CalculateFacetAssociation(void* handle)
{
    ((FacetAssociationHandle*)handle)->CalculateFacetAssociation();
}

extern "C" LIBRARY_EXPORT uint32_t* GetOutputFacetAssociation(void* handle)
{
    return ((FacetAssociationHandle*)handle)->GetOutputFacetAssociation();
}
