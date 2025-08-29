#include "binary_space_partition.hpp"

#pragma mark - External_Handle

BinarySpacePartitionHandle::BinarySpacePartitionHandle()
{
    m_input = new BinarySpacePartitionInput();
    m_input->m_vertices = nullptr;
    m_input->m_vertices_count = 0;
    m_input->m_tetrahedrons = nullptr;
    m_input->m_tetrahedrons_count = 0;
    m_input->m_constraints = nullptr;
    m_input->m_constraints_count = 0;
    
    m_output = new BinarySpacePartitionOutput();
    m_output->m_inserted_vertices = nullptr;
    m_output->m_inserted_vertices_count = 0;
    m_output->m_polyhedrons = nullptr;
    m_output->m_polyhedrons_count = 0;
    m_output->m_polyhedrons_facets = nullptr;
    m_output->m_polyhedrons_facets_count = 0;
    
    m_binarySpacePartition = new BinarySpacePartition();
}
void BinarySpacePartitionHandle::Dispose()
{
    delete_vertices(m_input->m_vertices, m_input->m_vertices_count);
    delete[] m_input->m_tetrahedrons;
    delete[] m_input->m_constraints;
    delete m_input;
    
    delete[] m_output->m_inserted_vertices;
    delete[] m_output->m_polyhedrons;
    delete[] m_output->m_polyhedrons_facets;
    delete m_output;
    
    delete m_binarySpacePartition;
}

void BinarySpacePartitionHandle::AddBinarySpacePartitionInput(uint32_t explicit_count, double* explicit_values, uint32_t tetrahedrons_count, uint32_t* tetrahedrons, uint32_t constraints_count, uint32_t* constraints)
{
    create_vertices(explicit_count, explicit_values, 0, nullptr, m_input->m_vertices, m_input->m_vertices_count);
    
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
void BinarySpacePartitionHandle::CalculateBinarySpacePartition()
{
    m_binarySpacePartition->binary_space_partition(m_input,m_output);
}

uint32_t BinarySpacePartitionHandle::GetOutputInsertedVerticesCount()
{
    return m_output->m_inserted_vertices_count;
}
uint32_t* BinarySpacePartitionHandle::GetOutputInsertedVertices()
{
    return m_output->m_inserted_vertices;
}

uint32_t BinarySpacePartitionHandle::GetOutputPolyhedronsCount()
{
    return m_output->m_polyhedrons_count;
}
uint32_t* BinarySpacePartitionHandle::GetOutputPolyhedrons()
{
    return m_output->m_polyhedrons;
}

uint32_t BinarySpacePartitionHandle::GetOutputPolyhedronsFacetsCount()
{
    return m_output->m_polyhedrons_facets_count;
}
uint32_t* BinarySpacePartitionHandle::GetOutputPolyhedronsFacets()
{
    return m_output->m_polyhedrons_facets;
}

extern "C" LIBRARY_EXPORT void* CreateBinarySpacePartitionHandle()
{
    return new BinarySpacePartitionHandle();
}
extern "C" LIBRARY_EXPORT void DisposeBinarySpacePartitionHandle(void* handle)
{
    ((BinarySpacePartitionHandle*)handle)->Dispose();
    delete (BinarySpacePartitionHandle*)handle;
}

extern "C" LIBRARY_EXPORT void AddBinarySpacePartitionInput(void* handle, uint32_t explicit_count, double* explicit_values, uint32_t tetrahedron_count, uint32_t* tetrahedrons, uint32_t constraints_count, uint32_t* constraints)
{
    ((BinarySpacePartitionHandle*)handle)->AddBinarySpacePartitionInput(explicit_count, explicit_values, tetrahedron_count, tetrahedrons, constraints_count, constraints);
}

extern "C" LIBRARY_EXPORT void CalculateBinarySpacePartition(void* handle)
{
    ((BinarySpacePartitionHandle*)handle)->CalculateBinarySpacePartition();
}

extern "C" LIBRARY_EXPORT uint32_t GetOutputInsertedVerticesCount(void* handle)
{
    return ((BinarySpacePartitionHandle*)handle)->GetOutputInsertedVerticesCount();
}
extern "C" LIBRARY_EXPORT uint32_t* GetOutputInsertedVertices(void* handle)
{
    return  ((BinarySpacePartitionHandle*)handle)->GetOutputInsertedVertices();
}

extern "C" LIBRARY_EXPORT uint32_t GetOutputPolyhedronsCount(void* handle)
{
    return ((BinarySpacePartitionHandle*)handle)->GetOutputPolyhedronsCount();
}
extern "C" LIBRARY_EXPORT uint32_t* GetOutputPolyhedrons(void* handle)
{
    return ((BinarySpacePartitionHandle*)handle)->GetOutputPolyhedrons();
}

extern "C" LIBRARY_EXPORT uint32_t GetOutputPolyhedronsFacetsCount(void* handle)
{
    return ((BinarySpacePartitionHandle*)handle)->GetOutputPolyhedronsFacetsCount();
}
extern "C" LIBRARY_EXPORT uint32_t* GetOutputPolyhedronsFacets(void* handle)
{
    return ((BinarySpacePartitionHandle*)handle)->GetOutputPolyhedronsFacets();
}

#pragma mark - Internal_Core

PolyhedronEdge::PolyhedronEdge(){}
PolyhedronEdge::PolyhedronEdge(PolyhedronEdge* other)
{
    this->e0 = other->e0;
    this->e1 = other->e1;
    this->p0 = other->p0;
    this->p1 = other->p1;
    this->p2 = other->p2;
    this->p3 = other->p3;
    this->p4 = other->p4;
    this->p5 = other->p5;
    this->f = other->f;
}

PolyhedronFacet::PolyhedronFacet(){}
PolyhedronFacet::PolyhedronFacet(PolyhedronFacet* other)
{
    this->edges = other->edges;
    this->p0 = other->p0;
    this->p1 = other->p1;
    this->p2 = other->p2;
    this->ip0 = other->ip0;
    this->ip1 = other->ip1;
}


void BinarySpacePartition::binary_space_partition(BinarySpacePartitionInput* input, BinarySpacePartitionOutput* output)
{
    m_vertices.clear();
    for(uint32_t i=0; i<input->m_vertices_count; i++)
    {
        m_vertices.push_back(input->m_vertices[i]);
    }
    m_tetrahedrons = input->m_tetrahedrons;
    m_tetrahedralization.assign_tetrahedrons(input->m_tetrahedrons, input->m_tetrahedrons_count);
    
    uint32_t removed_constraints_count = 0;
    m_constraints.clear();
    // only keep unique and non collinear constraints
    m_u_map_iii_i_0.clear();
    for(uint32_t i=0; i<3*input->m_constraints_count; i+=3)
    {
        uint32_t t0 = input->m_constraints[i+0];
        uint32_t t1 = input->m_constraints[i+1];
        uint32_t t2 = input->m_constraints[i+2];
        
        sort_ints(t0, t1, t2); // it is ok to reorder constraints in this case

        if(m_u_map_iii_i_0.end() != m_u_map_iii_i_0.find(make_tuple(t0,t1,t2)) || t0 == t1 || t0 == t2 || t1 == t2 || is_collinear(t0, t1, t2, m_vertices.data()))
        {
            removed_constraints_count++;
            m_u_map_iii_i_0[make_tuple(t0,t1,t2)] = 1;
            continue;
        }
        m_u_map_iii_i_0[make_tuple(t0,t1,t2)] = 1;
        m_constraints.push_back(t0);
        m_constraints.push_back(t1);
        m_constraints.push_back(t2);
    }
    input->m_constraints_count -= removed_constraints_count;

    // convert tetrahedrons to polyhedrons
    {
        m_polyhedrons.clear();
        m_polyhedrons_facets.clear();
        m_polyhedrons_edges.clear();
        m_vertices_incidents.resize(input->m_vertices_count);
        m_tetrahedrons_polyhedrons_mapping.resize(input->m_tetrahedrons_count);

        m_u_map_ii_i_0.clear();
        m_u_map_iii_i_0.clear();

        for(uint32_t i=0; i<4*input->m_tetrahedrons_count; i+=4)
        {
            m_vertices_incidents[m_tetrahedrons[i+0]] = i;
            m_vertices_incidents[m_tetrahedrons[i+1]] = i;
            m_vertices_incidents[m_tetrahedrons[i+2]] = i;
            m_vertices_incidents[m_tetrahedrons[i+3]] = i;

            uint32_t p = m_polyhedrons.size();
            m_tetrahedrons_polyhedrons_mapping[i/4] = p;

            m_polyhedrons.push_back(new Polyhedron());
            for(uint32_t f=0; f<4; f++)
            {
                uint32_t p0,p1,p2;
                uint32_t e0,e1,e2;
                uint32_t facet;
                m_tetrahedralization.get_tetrahedron_face(i,f,p0,p1,p2);
                
                e0 = find_or_add_edge(p0,p1);
                e1 = find_or_add_edge(p1,p2);
                e2 = find_or_add_edge(p2,p0);
                facet = find_or_add_facet(e0,e1,e2,p0,p1,p2);
                m_polyhedrons_edges[e0]->f = facet;
                m_polyhedrons_edges[e1]->f = facet;
                m_polyhedrons_edges[e2]->f = facet;
                if(UNDEFINED_VALUE == m_polyhedrons_facets[facet]->ip0)
                {
                    m_polyhedrons_facets[facet]->ip0 = p;
                }
                else
                {
                    m_polyhedrons_facets[facet]->ip1 = p;
                }
                m_polyhedrons[p]->facets.push_back(facet);
            }
        }
    }
    
    // create virtual constraints
    {
        m_virtual_constraints_count = 0;
        m_u_map_ii_vector_i_0.clear(); // key is an edge, value is the edge's incident constraints
        for(uint32_t i=0; i<3*input->m_constraints_count; i+=3)
        {
            uint32_t c0 = m_constraints[i+0];
            uint32_t c1 = m_constraints[i+1];
            uint32_t c2 = m_constraints[i+2];

            uint32_t t0 = c0;
            uint32_t t1 = c1;
            sort_ints(t0,t1);
            m_u_map_ii_vector_i_0[make_pair(t0,t1)].push_back(i);
            t0 = c1;
            t1 = c2;
            sort_ints(t0,t1);
            m_u_map_ii_vector_i_0[make_pair(t0,t1)].push_back(i);
            t0 = c2;
            t1 = c0;
            sort_ints(t0,t1);
            m_u_map_ii_vector_i_0[make_pair(t0,t1)].push_back(i);
        }
        
        for(auto& it : m_u_map_ii_vector_i_0)
        {
            uint32_t e0 = it.first.first;
            uint32_t e1 = it.first.second;

            if(1 == it.second.size())
            {
                add_virtual_constraint(e0,e1, it.second[0]);
                continue;
            }
            
            m_u_set_i_0.clear(); // stores the other vertex on every incident constraint
            for(uint32_t c : it.second)
            {
                m_u_set_i_0.insert(m_constraints[c+0]);
                m_u_set_i_0.insert(m_constraints[c+1]);
                m_u_set_i_0.insert(m_constraints[c+2]);
            }
            m_u_set_i_0.erase(e0);
            m_u_set_i_0.erase(e1);

            uint32_t e2 = *m_u_set_i_0.begin();
            m_u_set_i_0.erase(m_u_set_i_0.begin());
            bool is_on_boundary = true;
            for(uint32_t e3 : m_u_set_i_0)
            {
                if(0 != orient3d(e3,e0,e1,e2, m_vertices.data()))
                {
                    is_on_boundary = false;
                    break;
                }
            }
            if(!is_on_boundary)
            {
                continue;
            }
            
            int ignore_axis = max_component_in_triangle_normal(e0,e1,e2, m_vertices.data());
            int oe2 = orient3d_ignore_axis(e0,e1,e2,ignore_axis, m_vertices.data());
            for(uint32_t e3 : m_u_set_i_0)
            {
                int oe3 = orient3d_ignore_axis(e0,e1,e3,ignore_axis, m_vertices.data());
                if(0 == oe2)
                {
                    oe2 = oe3;
                }
                if(oe2 != oe3)
                {
                    add_virtual_constraint(e0,e1, it.second[0]);
                    break;
                }
            }
        }
    }


    // for every polyhedron, find the constraints that intersect it.
    // for every polyhedron facet, find the coplanar constraints that intersect it.
    {
        m_polyhedrons_intersect_constraints.clear();
        for(uint32_t i=0; i<3*(input->m_constraints_count+m_virtual_constraints_count); i+=3)
        {
            uint32_t c0 = m_constraints[i+0];
            uint32_t c1 = m_constraints[i+1];
            uint32_t c2 = m_constraints[i+2];

            m_u_set_i_0.clear(); // stores improperly intersected tetrahedrons
            m_u_set_i_1.clear(); // stores visited polyhedrons
            m_u_set_i_1.insert(UNDEFINED_VALUE);
            clear_queue(m_queue_i_0); // stores tetrahedrons to be visited
            m_queue_i_0.push(m_vertices_incidents[c0]);
            m_queue_i_0.push(m_vertices_incidents[c1]);
            m_queue_i_0.push(m_vertices_incidents[c2]);

            while(!m_queue_i_0.empty())
            {
                uint32_t t = m_queue_i_0.front();
                m_queue_i_0.pop();
                if(m_u_set_i_1.end() != m_u_set_i_1.find(t))
                {
                    continue;
                }
                m_u_set_i_1.insert(t);

                int int_type = tt_intersection(m_tetrahedrons[t+0], m_tetrahedrons[t+1], m_tetrahedrons[t+2], m_tetrahedrons[t+3], c0, c1, c2);
                if(0 == int_type)
                {
                    continue;
                }
                if(2 == int_type)
                {
                    m_u_set_i_0.insert(t);
                }
                
                uint32_t n;
                for(uint32_t f=0; f<4; f++)
                {
                    m_tetrahedralization.get_tetrahedron_neighbor(t, f, n);
                    m_queue_i_0.push(n);
                }
            }

            m_u_set_i_0.erase(UNDEFINED_VALUE);
            // if the constraint improperly intersects a tetrahedron, they are associated
            for(uint32_t t : m_u_set_i_0)
            {
                m_polyhedrons_intersect_constraints[m_tetrahedrons_polyhedrons_mapping[t/4]].push_back(i);
            }
        }
    }

    // slice each polyhedron with its list of improper constraints.
    {
        m_new_vertices_mappings.clear();
        for(auto it=m_polyhedrons_intersect_constraints.begin(); it!=m_polyhedrons_intersect_constraints.end(); it++)
        {
            while(!it->second.empty())
            {
                uint32_t i = it->first;

                uint32_t c = it->second.back();
                it->second.pop_back();
                uint32_t c0 = m_constraints[c+0];
                uint32_t c1 = m_constraints[c+1];
                uint32_t c2 = m_constraints[c+2];
                
                m_vector_i_0.clear(); // top polyhedron's facets
                m_vector_i_1.clear(); // bottom polyhedron's facets
                m_u_set_i_0.clear(); // edges on the constraint
                m_u_map_i_iii_0.clear(); // keys are processed edges, values are (intersection vertex, top edge, bottom edge)
                for(uint32_t f : m_polyhedrons[i]->facets)
                {
                    uint32_t i0(UNDEFINED_VALUE), i1(UNDEFINED_VALUE);
                    m_vector_i_2.clear(); // edges on top of the constraint
                    m_vector_i_3.clear(); // edges on bottom of the constraint
                    bool has_edge_on_constraint = false;
                    
                    for(uint32_t e : m_polyhedrons_facets[f]->edges)
                    {
                        if(m_u_map_i_iii_0.end() != m_u_map_i_iii_0.find(e))
                        {
                            auto [i_p,top_e,bottom_e] = m_u_map_i_iii_0[e];
                            if(UNDEFINED_VALUE == i_p && top_e == UNDEFINED_VALUE && bottom_e == UNDEFINED_VALUE)
                            {
                                m_u_set_i_0.insert(e); // e is on the constraint
                                has_edge_on_constraint = true;
                                continue;
                            }
                            
                            if(UNDEFINED_VALUE != i_p)
                            {
                                if(UNDEFINED_VALUE == i0 || i_p == i0)
                                {
                                    i0 = i_p;
                                }
                                else
                                {
                                    i1 = i_p;
                                }
                            }
                            if(UNDEFINED_VALUE != top_e)
                            {
                                m_vector_i_2.push_back(top_e);
                            }
                            if(UNDEFINED_VALUE != bottom_e)
                            {
                                m_vector_i_3.push_back(bottom_e);
                            }
                            continue;
                        }
                        
                        uint32_t e0(m_polyhedrons_edges[e]->e0), e1(m_polyhedrons_edges[e]->e1);
                        int o0(orient3d(c0,c1,c2,e0, m_vertices.data())), o1(orient3d(c0,c1,c2,e1, m_vertices.data()));
                        if(0 == o0)
                        {
                            if(0 == o1)
                            {
                                m_u_set_i_0.insert(e);
                                has_edge_on_constraint = true;
                                m_u_map_i_iii_0[e] = {UNDEFINED_VALUE,UNDEFINED_VALUE,UNDEFINED_VALUE};
                            }
                            else
                            {
                                if(1 == o1)
                                {
                                    m_vector_i_2.push_back(e);
                                    m_u_map_i_iii_0[e] = {e0,e,UNDEFINED_VALUE};
                                }
                                else
                                {
                                    m_vector_i_3.push_back(e);
                                    m_u_map_i_iii_0[e] = {e0,UNDEFINED_VALUE,e};
                                }
                                
                                if(UNDEFINED_VALUE == i0 || e0 == i0)
                                {
                                    i0 = e0;
                                }
                                else
                                {
                                    i1 = e0;
                                }
                            }
                        }
                        else if(1 == o0)
                        {
                            if(0 == o1)
                            {
                                m_vector_i_2.push_back(e);
                                m_u_map_i_iii_0[e] = {e1,e,UNDEFINED_VALUE};
                                if(UNDEFINED_VALUE == i0 || e1 == i0)
                                {
                                    i0 = e1;
                                }
                                else
                                {
                                    i1 = e1;
                                }
                            }
                            else if(1 == o1)
                            {
                                m_vector_i_2.push_back(e);
                                m_u_map_i_iii_0[e] = {UNDEFINED_VALUE,e,UNDEFINED_VALUE};
                            }
                            else
                            {
                                uint32_t new_i;
                                if(UNDEFINED_VALUE == m_polyhedrons_edges[e]->p2)
                                {
                                    uint32_t oe0(m_polyhedrons_edges[e]->p0), oe1(m_polyhedrons_edges[e]->p1);
                                    new_i = add_LPI(oe0,oe1,c0,c1,c2);
                                }
                                else
                                {
                                    new_i = add_TPI(c0,c1,c2,m_polyhedrons_edges[e]->p0,m_polyhedrons_edges[e]->p1,m_polyhedrons_edges[e]->p2,m_polyhedrons_edges[e]->p3,m_polyhedrons_edges[e]->p4,m_polyhedrons_edges[e]->p5);
                                }
                                
                                m_polyhedrons_edges[e]->e0 = e0;
                                m_polyhedrons_edges[e]->e1 = new_i;
                                m_vector_i_2.push_back(e);
                                uint32_t bottom_e = m_polyhedrons_edges.size();
                                m_polyhedrons_edges.push_back(new PolyhedronEdge(m_polyhedrons_edges[e]));
                                m_polyhedrons_edges[bottom_e]->e0 = new_i;
                                m_polyhedrons_edges[bottom_e]->e1 = e1;
                                m_vector_i_3.push_back(bottom_e);
                                
                                m_u_map_i_iii_0[e] = {new_i,e,bottom_e};
                                
                                if(UNDEFINED_VALUE == i0 || new_i == i0)
                                {
                                    i0 = new_i;
                                }
                                else
                                {
                                    i1 = new_i;
                                }
                            }
                        }
                        else
                        {
                            if(0 == o1)
                            {
                                m_vector_i_3.push_back(e);
                                m_u_map_i_iii_0[e] = {e1,UNDEFINED_VALUE,e};
                                if(UNDEFINED_VALUE == i0 || e1 == i0)
                                {
                                    i0 = e1;
                                }
                                else
                                {
                                    i1 = e1;
                                }
                            }
                            else if(1 == o1)
                            {
                                uint32_t new_i;
                                if(UNDEFINED_VALUE == m_polyhedrons_edges[e]->p2)
                                {
                                    uint32_t oe0(m_polyhedrons_edges[e]->p0), oe1(m_polyhedrons_edges[e]->p1);
                                    new_i = add_LPI(oe0,oe1,c0,c1,c2);
                                }
                                else
                                {
                                    new_i = add_TPI(c0,c1,c2,m_polyhedrons_edges[e]->p0,m_polyhedrons_edges[e]->p1,m_polyhedrons_edges[e]->p2,m_polyhedrons_edges[e]->p3,m_polyhedrons_edges[e]->p4,m_polyhedrons_edges[e]->p5);
                                }
                                
                                m_polyhedrons_edges[e]->e0 = e1;
                                m_polyhedrons_edges[e]->e1 = new_i;
                                m_vector_i_2.push_back(e);
                                uint32_t bottom_e = m_polyhedrons_edges.size();
                                m_polyhedrons_edges.push_back(new PolyhedronEdge(m_polyhedrons_edges[e]));
                                m_polyhedrons_edges[bottom_e]->e0 = new_i;
                                m_polyhedrons_edges[bottom_e]->e1 = e0;
                                m_vector_i_3.push_back(bottom_e);
                                
                                m_u_map_i_iii_0[e] = {new_i,e,bottom_e};
                                if(UNDEFINED_VALUE == i0 || new_i == i0)
                                {
                                    i0 = new_i;
                                }
                                else
                                {
                                    i1 = new_i;
                                }
                            }
                            else
                            {
                                m_vector_i_3.push_back(e);
                                m_u_map_i_iii_0[e] = {UNDEFINED_VALUE,UNDEFINED_VALUE,e};
                            }
                        }
                    }
                    
                    if(has_edge_on_constraint || i1 == UNDEFINED_VALUE) // constraint intersects on one or more edges of the facet or does not intersect the facet
                    {
                        if(!m_vector_i_2.empty())
                        {
                            m_vector_i_0.push_back(f);
                        }
                        else if(!m_vector_i_3.empty())
                        {
                            m_vector_i_1.push_back(f);
                        }
                    }
                    else
                    {
                        uint32_t i_e = m_polyhedrons_edges.size();
                        m_polyhedrons_edges.push_back(new PolyhedronEdge());
                        m_polyhedrons_edges[i_e]->e0 = i0;
                        m_polyhedrons_edges[i_e]->e1 = i1;
                        m_polyhedrons_edges[i_e]->p0 = m_polyhedrons_facets[f]->p0;
                        m_polyhedrons_edges[i_e]->p1 = m_polyhedrons_facets[f]->p1;
                        m_polyhedrons_edges[i_e]->p2 = m_polyhedrons_facets[f]->p2;
                        m_polyhedrons_edges[i_e]->p3 = c0;
                        m_polyhedrons_edges[i_e]->p4 = c1;
                        m_polyhedrons_edges[i_e]->p5 = c2;
                        m_u_set_i_0.insert(i_e);
                        
                        m_polyhedrons_facets[f]->edges.clear();
                        uint32_t bottom_f = m_polyhedrons_facets.size();
                        m_polyhedrons_facets.push_back(new PolyhedronFacet(m_polyhedrons_facets[f]));
                        m_polyhedrons_facets[f]->edges = m_vector_i_2;
                        m_polyhedrons_facets[f]->edges.push_back(i_e);
                        m_polyhedrons_facets[bottom_f]->edges = m_vector_i_3;
                        m_polyhedrons_facets[bottom_f]->edges.push_back(i_e);
                        m_vector_i_0.push_back(f);
                        m_vector_i_1.push_back(bottom_f);
                    }
                }
                
                if(0 == m_vector_i_0.size() || 0 == m_vector_i_1.size())
                {
                    continue;
                }
                if(m_vector_i_0.size()<3 || m_vector_i_1.size()<3 || m_u_set_i_0.size()<3)
                {
                    cout << "wtf" << endl;
                }
                
                uint32_t bottom_polyhedron = m_polyhedrons.size();
                m_polyhedrons.push_back(new Polyhedron());
                
                uint32_t common_facet = m_polyhedrons_facets.size();
                m_polyhedrons_facets.push_back(new PolyhedronFacet());
                m_polyhedrons_facets[common_facet]->p0 = c0;
                m_polyhedrons_facets[common_facet]->p1 = c1;
                m_polyhedrons_facets[common_facet]->p2 = c2;
                m_polyhedrons_facets[common_facet]->ip0 = i;
                m_polyhedrons_facets[common_facet]->ip1 = bottom_polyhedron;
                for(uint32_t e : m_u_set_i_0)
                {
                    m_polyhedrons_facets[common_facet]->edges.push_back(e);
                }
                
                m_polyhedrons[i]->facets = m_vector_i_0;
                m_polyhedrons[i]->facets.push_back(common_facet);
                m_polyhedrons[bottom_polyhedron]->facets = m_vector_i_1;
                m_polyhedrons[bottom_polyhedron]->facets.push_back(common_facet);
                
                for(uint32_t f : m_polyhedrons[i]->facets)
                {
                    for(uint32_t e : m_polyhedrons_facets[f]->edges)
                    {
                        m_polyhedrons_edges[e]->f = f;
                    }
                }
                for(uint32_t f : m_polyhedrons[bottom_polyhedron]->facets)
                {
                    for(uint32_t e : m_polyhedrons_facets[f]->edges)
                    {
                        m_polyhedrons_edges[e]->f = f;
                    }
                    if(f == common_facet)
                    {
                        continue;
                    }
                    uint32_t n;
                    if(i == m_polyhedrons_facets[f]->ip0)
                    {
                        m_polyhedrons_facets[f]->ip0 = bottom_polyhedron;
                        n = m_polyhedrons_facets[f]->ip1;
                    }
                    else
                    {
                        m_polyhedrons_facets[f]->ip1 = bottom_polyhedron;
                        n = m_polyhedrons_facets[f]->ip0;
                    }
                    if(n != UNDEFINED_VALUE)
                    {
                        if(m_polyhedrons[n]->facets.end() == find(m_polyhedrons[n]->facets.begin(), m_polyhedrons[n]->facets.end(), f))
                        {
                            m_polyhedrons[n]->facets.push_back(f);
                        }
                    }
                }
                
                
                for(auto [k, v] : m_u_map_i_iii_0)
                {
                    uint32_t original_e = k;
                    uint32_t i_e = get<0>(v);
                    uint32_t top_e = get<1>(v);
                    uint32_t bottom_e = get<2>(v);
                    
                    if(UNDEFINED_VALUE == i_e || UNDEFINED_VALUE == top_e || UNDEFINED_VALUE == bottom_e)
                    {
                        continue;
                    }
                    
                    if(original_e == bottom_e)
                    {
                        swap(top_e,bottom_e);
                    }
                    m_u_set_i_0.clear(); // visited polyhedrons
                    m_u_set_i_0.insert(UNDEFINED_VALUE);
                    clear_queue(m_queue_i_0); // polyhedrons to be visited
                    m_queue_i_0.push(i);
                    while(!m_queue_i_0.empty())
                    {
                        uint32_t cur = m_queue_i_0.front();
                        m_queue_i_0.pop();
                        
                        if(m_u_set_i_0.end() != m_u_set_i_0.find(cur))
                        {
                            continue;
                        }
                        m_u_set_i_0.insert(cur);
                        
                        bool search_neighbor = false;
                        for(uint32_t f : m_polyhedrons[cur]->facets)
                        {
                            if(m_polyhedrons_facets[f]->edges.end() != find(m_polyhedrons_facets[f]->edges.begin(),m_polyhedrons_facets[f]->edges.end(), original_e))
                            {
                                search_neighbor = true;
                                if(m_polyhedrons[i]->facets.end() != find(m_polyhedrons[i]->facets.begin(), m_polyhedrons[i]->facets.end(), f))
                                {
                                    continue;
                                }
                                if(m_polyhedrons[bottom_polyhedron]->facets.end() != find(m_polyhedrons[bottom_polyhedron]->facets.begin(), m_polyhedrons[bottom_polyhedron]->facets.end(), f))
                                {
                                    continue;
                                }
                                if(m_polyhedrons_facets[f]->edges.end() == find(m_polyhedrons_facets[f]->edges.begin(),m_polyhedrons_facets[f]->edges.end(), bottom_e))
                                {
                                    m_polyhedrons_facets[f]->edges.push_back(bottom_e);
                                }
                            }
                        }
                        
                        if(search_neighbor)
                        {
                            for(uint32_t f : m_polyhedrons[cur]->facets)
                            {
                                if(cur == m_polyhedrons_facets[f]->ip0)
                                {
                                    m_queue_i_0.push(m_polyhedrons_facets[f]->ip1);
                                }
                                else
                                {
                                    m_queue_i_0.push(m_polyhedrons_facets[f]->ip0);
                                }
                            }
                        }
                    }
                }
                
                m_vector_i_0.clear(); // store constraints that intersect top polyhedron
                for(uint32_t j=0; j<it->second.size(); j++)
                {
                    uint32_t nc0 = m_constraints[it->second[j]+0];
                    uint32_t nc1 = m_constraints[it->second[j]+1];
                    uint32_t nc2 = m_constraints[it->second[j]+2];
                    int o0(orient3d(c0,c1,c2,nc0, m_vertices.data())), o1(orient3d(c0,c1,c2,nc1, m_vertices.data())), o2(orient3d(c0,c1,c2,nc2, m_vertices.data()));

                    if(1 == o0 || 1 == o1 || 1 == o2)
                    {
                        m_vector_i_0.push_back(it->second[j]);
                    }
                    if(-1 == o0 || -1 == o1 || -1 == o2)
                    {
                        m_polyhedrons_intersect_constraints[bottom_polyhedron].push_back(it->second[j]);
                    }
                }
                it->second = m_vector_i_0;
            }
        }
    }

    // produce output
    {
        m_vector_i_0.clear();
        for(uint32_t i=0; i<m_new_vertices_mappings.size(); i+=9)
        {
            uint32_t n = UNDEFINED_VALUE == m_new_vertices_mappings[i+5] ? 5 : 9;

            m_vector_i_0.push_back(n);
            for(uint32_t j=0; j<n; j++)
            {
                m_vector_i_0.push_back(m_new_vertices_mappings[i+j]);
            }
        }
        output->m_inserted_vertices_count = m_new_vertices_mappings.size() / 9;
        output->m_inserted_vertices = new uint32_t[m_vector_i_0.size()];
        for(uint32_t i=0; i<m_vector_i_0.size(); i++)
        {
            output->m_inserted_vertices[i] = m_vector_i_0[i];
        }

        m_vector_i_0.clear();
        for(uint32_t i=0; i<m_polyhedrons.size(); i++)
        {
            m_vector_i_0.push_back(m_polyhedrons[i]->facets.size());
            for(uint32_t j=0; j<m_polyhedrons[i]->facets.size(); j++)
            {
                m_vector_i_0.push_back(m_polyhedrons[i]->facets[j]);
            }
        }
        output->m_polyhedrons = new uint32_t[m_vector_i_0.size()];
        output->m_polyhedrons_count = m_polyhedrons.size();
        for(uint32_t i=0; i<m_vector_i_0.size(); i++)
        {
            output->m_polyhedrons[i] = m_vector_i_0[i];
        }

        m_vector_i_0.clear();
        for(uint32_t i=0; i<m_polyhedrons_facets.size(); i++)
        {
            sort_polyhedron_facet(i);
        }
        for(uint32_t i=0; i<m_polyhedrons_facets.size(); i++)
        {
            get_polyhedron_facet_vertices(i, m_deque_i_0);
            m_vector_i_0.push_back(m_deque_i_0.size());
            for(uint32_t j=0; j<m_deque_i_0.size(); j++)
            {
                m_vector_i_0.push_back(m_deque_i_0[j]);
            }
        }
        output->m_polyhedrons_facets = new uint32_t[m_vector_i_0.size()];
        output->m_polyhedrons_facets_count = m_polyhedrons_facets.size();
        for(uint32_t i=0; i<m_vector_i_0.size(); i++)
        {
            output->m_polyhedrons_facets[i] = m_vector_i_0[i];
        }
    }

    // clean up
    {
        // just delete newly added vertices
        for(uint32_t i=input->m_vertices_count; i<m_vertices.size(); i++)
        {
            delete_vertex(m_vertices[i]);
        }
        for(uint32_t i=0; i<m_polyhedrons.size(); i++)
        {
            delete m_polyhedrons[i];
        }
        for(uint32_t i=0; i<m_polyhedrons_facets.size(); i++)
        {
            delete m_polyhedrons_facets[i];
        }
        for(uint32_t i=0; i<m_polyhedrons_edges.size(); i++)
        {
            delete m_polyhedrons_edges[i];
        }
    }

}

#pragma mark - Internal_Helper

uint32_t BinarySpacePartition::find_or_add_edge(uint32_t p0, uint32_t p1)
{
    sort_ints(p0,p1);
    if(m_u_map_ii_i_0.end() != m_u_map_ii_i_0.find(make_pair(p0,p1)))
    {
        return m_u_map_ii_i_0[make_pair(p0,p1)];
    }

    m_u_map_ii_i_0[make_pair(p0,p1)] = m_polyhedrons_edges.size();

    PolyhedronEdge* e = new PolyhedronEdge();
    e->e0 = p0;
    e->e1 = p1;
    e->p0 = p0;
    e->p1 = p1;
    e->p2 = e->p3 = e->p4 = e->p5 = e->f = UNDEFINED_VALUE;
    m_polyhedrons_edges.push_back(e);
    return m_polyhedrons_edges.size()-1;
}
uint32_t BinarySpacePartition::find_or_add_facet(uint32_t e0, uint32_t e1, uint32_t e2, uint32_t p0, uint32_t p1,  uint32_t p2)
{
    sort_ints(e0,e1,e2);
    if(m_u_map_iii_i_0.end() != m_u_map_iii_i_0.find(make_tuple(e0,e1,e2)))
    {
        return m_u_map_iii_i_0[make_tuple(e0,e1,e2)];
    }
    m_u_map_iii_i_0[make_tuple(e0,e1,e2)] = m_polyhedrons_facets.size();
    PolyhedronFacet* f = new PolyhedronFacet();
    f->edges.push_back(e0);
    f->edges.push_back(e1);
    f->edges.push_back(e2);
    f->p0 = p0;
    f->p1 = p1;
    f->p2 = p2;
    f->ip0 = UNDEFINED_VALUE;
    f->ip1 = UNDEFINED_VALUE;
    m_polyhedrons_facets.push_back(f);
    return m_polyhedrons_facets.size()-1;
}

void BinarySpacePartition::add_virtual_constraint(uint32_t e0, uint32_t e1, uint32_t c) // e0 and e1 incident the constraint c
{
    uint32_t t = m_vertices_incidents[e0];
    uint32_t c0 = m_constraints[c+0];
    uint32_t c1 = m_constraints[c+1];
    uint32_t c2 = m_constraints[c+2];
    for(uint64_t i=0; i<4; i++)
    {
        if(c0 == m_tetrahedrons[t+i] || c1 == m_tetrahedrons[t+i] || c2 == m_tetrahedrons[t+i] || 0 == orient3d(m_tetrahedrons[t+i],c0,c1,c2, m_vertices.data()))
        {
            continue;
        }
        m_constraints.push_back(e0);
        m_constraints.push_back(e1);
        m_constraints.push_back(m_tetrahedrons[t+i]);
        m_virtual_constraints_count++;
        return;
    }
    throw "can't add virtual constraint";
}


void BinarySpacePartition::get_polyhedron_facet_vertices(uint32_t f, uint32_t& f0,uint32_t& f1,uint32_t& f2)
{
    f0 = f1 = f2 = UNDEFINED_VALUE;
    f0 = m_polyhedrons_edges[m_polyhedrons_facets[f]->edges[0]]->e0;
    f1 = m_polyhedrons_edges[m_polyhedrons_facets[f]->edges[0]]->e1;
    
    if(f0 == m_polyhedrons_edges[m_polyhedrons_facets[f]->edges[1]]->e0 || f1 == m_polyhedrons_edges[m_polyhedrons_facets[f]->edges[1]]->e0)
    {
        f2 = m_polyhedrons_edges[m_polyhedrons_facets[f]->edges[1]]->e1;
    }
    else
    {
        f2 = m_polyhedrons_edges[m_polyhedrons_facets[f]->edges[1]]->e0;
    }
}

// 0 no intersection, 1 intersection in exterior, 2 int interior
uint32_t BinarySpacePartition::tt_intersection(uint32_t t0, uint32_t t1, uint32_t t2, uint32_t t3, uint32_t c0, uint32_t c1, uint32_t c2)
{
    // neighbors order: 0,1,2  1,0,3  0,2,3,  2,1,3
    // check a constraint vertex is inside the tetrahedron
    {
        int oc0t012 = orient3d(t0, t1, t2, c0, m_vertices.data());
        int oc1t012 = orient3d(t0, t1, t2, c1, m_vertices.data());
        int oc2t012 = orient3d(t0, t1, t2, c2, m_vertices.data());
        int oc0t103 = orient3d(t1, t0, t3, c0, m_vertices.data());
        int oc1t103 = orient3d(t1, t0, t3, c1, m_vertices.data());
        int oc2t103 = orient3d(t1, t0, t3, c2, m_vertices.data());
        int oc0t023 = orient3d(t0, t2, t3, c0, m_vertices.data());
        int oc1t023 = orient3d(t0, t2, t3, c1, m_vertices.data());
        int oc2t023 = orient3d(t0, t2, t3, c2, m_vertices.data());
        int oc0t213 = orient3d(t2, t1, t3, c0, m_vertices.data());
        int oc1t213 = orient3d(t2, t1, t3, c1, m_vertices.data());
        int oc2t213 = orient3d(t2, t1, t3, c2, m_vertices.data());
        
        bool int_in =
        (1==oc0t012 && oc0t012==oc0t103 && oc0t012==oc0t023 && oc0t012==oc0t213) ||
        (1==oc1t012 && oc1t012==oc1t103 && oc1t012==oc1t023 && oc1t012==oc1t213) ||
        (1==oc2t012 && oc2t012==oc2t103 && oc2t012==oc2t023 && oc2t012==oc2t213) ||
        inner_segment_cross_inner_triangle(t0,t1,c0,c1,c2,m_vertices.data()) ||
        inner_segment_cross_inner_triangle(t0,t2,c0,c1,c2,m_vertices.data()) ||
        inner_segment_cross_inner_triangle(t0,t3,c0,c1,c2,m_vertices.data()) ||
        inner_segment_cross_inner_triangle(t1,t2,c0,c1,c2,m_vertices.data()) ||
        inner_segment_cross_inner_triangle(t1,t3,c0,c1,c2,m_vertices.data()) ||
        inner_segment_cross_inner_triangle(t2,t3,c0,c1,c2,m_vertices.data()) ||
        inner_segment_cross_inner_triangle(c0,c1,t1,t2,t3,m_vertices.data()) ||
        inner_segment_cross_inner_triangle(c1,c2,t1,t2,t3,m_vertices.data()) ||
        inner_segment_cross_inner_triangle(c2,c0,t1,t2,t3,m_vertices.data()) ||
        inner_segment_cross_inner_triangle(c0,c1,t0,t2,t3,m_vertices.data()) ||
        inner_segment_cross_inner_triangle(c1,c2,t0,t2,t3,m_vertices.data()) ||
        inner_segment_cross_inner_triangle(c2,c0,t0,t2,t3,m_vertices.data()) ||
        inner_segment_cross_inner_triangle(c0,c1,t0,t1,t3,m_vertices.data()) ||
        inner_segment_cross_inner_triangle(c1,c2,t0,t1,t3,m_vertices.data()) ||
        inner_segment_cross_inner_triangle(c2,c0,t0,t1,t3,m_vertices.data()) ||
        inner_segment_cross_inner_triangle(c0,c1,t0,t1,t2,m_vertices.data()) ||
        inner_segment_cross_inner_triangle(c1,c2,t0,t1,t2,m_vertices.data()) ||
        inner_segment_cross_inner_triangle(c2,c0,t0,t1,t2,m_vertices.data());
        if(int_in)
        {
            return 2;
        }
    }
    
    
    //check if constraint and tetrahedron share vertices
    uint32_t shared_vertices_count = tt_intersection_helper(t0,t1,t2,t3,c0,c1,c2);
    
    if(2 == shared_vertices_count)
    {
        int oc2t012 = orient3d(t0, t1, t2, c2, m_vertices.data());
        int oc2t103 = orient3d(t1, t0, t3, c2, m_vertices.data());
        if(1 == oc2t012 && 1 == oc2t103)
        {
            return 2;
        }
    }
    else if(1 == shared_vertices_count)
    {
        int oc1t012 = orient3d(t0, t1, t2, c1, m_vertices.data());
        int oc2t012 = orient3d(t0, t1, t2, c2, m_vertices.data());
        int oc1t103 = orient3d(t1, t0, t3, c1, m_vertices.data());
        int oc2t103 = orient3d(t1, t0, t3, c2, m_vertices.data());
        int oc1t023 = orient3d(t0, t2, t3, c1, m_vertices.data());
        int oc2t023 = orient3d(t0, t2, t3, c2, m_vertices.data());
        
        uint32_t c1_cover = -1;
        uint32_t c2_cover = -1;
        if(1 == oc1t012 && 1 == oc1t023)
        {
            c1_cover = 0;
        }
        if(1 == oc1t103 && 1 == oc1t023)
        {
            c1_cover = 1;
        }
        if(1 == oc1t012 && 1 == oc1t103)
        {
            c1_cover = 2;
        }
        if(1 == oc2t012 && 1 == oc2t023)
        {
            c2_cover = 0;
        }
        if(1 == oc2t103 && 1 == oc2t023)
        {
            c2_cover = 1;
        }
        if(1 == oc2t012 && 1 == oc2t103)
        {
            c2_cover = 2;
        }
        if(c1_cover != c2_cover)
        {
            return 2;
        }
    }
    
    if(0 != shared_vertices_count)
    {
        return 1;
    }
    
    // check point just touching
    bool int_no =
    segment_cross_triangle(t0,t1,c0,c1,c2,m_vertices.data()) ||
    segment_cross_triangle(t0,t2,c0,c1,c2,m_vertices.data()) ||
    segment_cross_triangle(t0,t3,c0,c1,c2,m_vertices.data()) ||
    segment_cross_triangle(t1,t2,c0,c1,c2,m_vertices.data()) ||
    segment_cross_triangle(t1,t3,c0,c1,c2,m_vertices.data()) ||
    segment_cross_triangle(t2,t3,c0,c1,c2,m_vertices.data()) ||
    segment_cross_triangle(c0,c1,t1,t2,t3,m_vertices.data()) ||
    segment_cross_triangle(c1,c2,t1,t2,t3,m_vertices.data()) ||
    segment_cross_triangle(c2,c0,t1,t2,t3,m_vertices.data()) ||
    segment_cross_triangle(c0,c1,t0,t2,t3,m_vertices.data()) ||
    segment_cross_triangle(c1,c2,t0,t2,t3,m_vertices.data()) ||
    segment_cross_triangle(c2,c0,t0,t2,t3,m_vertices.data()) ||
    segment_cross_triangle(c0,c1,t0,t1,t3,m_vertices.data()) ||
    segment_cross_triangle(c1,c2,t0,t1,t3,m_vertices.data()) ||
    segment_cross_triangle(c2,c0,t0,t1,t3,m_vertices.data()) ||
    segment_cross_triangle(c0,c1,t0,t1,t2,m_vertices.data()) ||
    segment_cross_triangle(c1,c2,t0,t1,t2,m_vertices.data()) ||
    segment_cross_triangle(c2,c0,t0,t1,t2,m_vertices.data());
    
    if(!int_no)
    {
        return 0;
    }

    return 1;
}

uint32_t BinarySpacePartition::tt_intersection_helper(uint32_t& t0, uint32_t& t1, uint32_t& t2, uint32_t& t3, uint32_t& c0, uint32_t& c1, uint32_t& c2)
{
    uint32_t shared_vertices_count = 0;
    
    shared_vertices_count += t0==c0 || t1==c0 || t2==c0 || t3==c0;
    shared_vertices_count += t0==c1 || t1==c1 || t2==c1 || t3==c1;
    shared_vertices_count += t0==c2 || t1==c2 || t2==c2 || t3==c2;
    
    uint32_t swap_count = 0;
    if(2 == shared_vertices_count)
    {
        if(!(t0==c0 || t1==c0 || t2==c0 || t3==c0))
        {
            swap(c0,c2);
        }
        else if(!(t0==c1 || t1==c1 || t2==c1 || t3==c1))
        {
            swap(c1,c2);
        }
        
        if(t2==c0 || t2==c1)
        {
            swap(t2,t0);
            swap_count++;
        }
        else if(t3==c0 || t3==c1)
        {
            swap(t3,t0);
            swap_count++;
        }
        if(t2==c0 || t2==c1)
        {
            swap(t3,t1);
            swap_count++;
        }
        else if(t3==c0 || t3==c1)
        {
            swap(t3,t1);
            swap_count++;
        }
        
        if(0 != swap_count%2)
        {
            swap(t2,t3);
        }
    }
    else if(1 == shared_vertices_count)
    {
        if(t0==c1 || t1==c1 || t2==c1 || t3==c1)
        {
            swap(c0,c1);
        }
        else if(t0==c2 || t1==c2 || t2==c2 || t3==c2)
        {
            swap(c0,c2);
        }
        
        if(t1==c0)
        {
            swap(t1,t0);
            swap_count++;
        }
        else if(t2==c0)
        {
            swap(t2,t0);
            swap_count++;
        }
        else if(t3==c0)
        {
            swap(t3,t0);
            swap_count++;
        }
        
        if(0 != swap_count%2)
        {
            swap(t2,t3);
        }
    }
    
    return shared_vertices_count;
}

uint32_t BinarySpacePartition::add_LPI(uint32_t e0, uint32_t e1, uint32_t p0,uint32_t p1,uint32_t p2)
{
    uint32_t res = m_vertices.size();
    m_vertices.push_back(new implicitPoint3D_LPI(
        m_vertices[e0]->toExplicit3D(),m_vertices[e1]->toExplicit3D(),
        m_vertices[p0]->toExplicit3D(),m_vertices[p1]->toExplicit3D(),m_vertices[p2]->toExplicit3D()));

    m_new_vertices_mappings.push_back(e0);
    m_new_vertices_mappings.push_back(e1);
    m_new_vertices_mappings.push_back(p0);
    m_new_vertices_mappings.push_back(p1);
    m_new_vertices_mappings.push_back(p2);
    m_new_vertices_mappings.push_back(UNDEFINED_VALUE);
    m_new_vertices_mappings.push_back(UNDEFINED_VALUE);
    m_new_vertices_mappings.push_back(UNDEFINED_VALUE);
    m_new_vertices_mappings.push_back(UNDEFINED_VALUE);
    return res;
}
uint32_t BinarySpacePartition::add_TPI(uint32_t p0,uint32_t p1,uint32_t p2,uint32_t p3,uint32_t p4,uint32_t p5,uint32_t p6,uint32_t p7,uint32_t p8)
{
    uint32_t res = m_vertices.size();
    m_vertices.push_back(new implicitPoint3D_TPI(
        m_vertices[p0]->toExplicit3D(),m_vertices[p1]->toExplicit3D(),m_vertices[p2]->toExplicit3D(),
        m_vertices[p3]->toExplicit3D(),m_vertices[p4]->toExplicit3D(),m_vertices[p5]->toExplicit3D(),
        m_vertices[p6]->toExplicit3D(),m_vertices[p7]->toExplicit3D(),m_vertices[p8]->toExplicit3D()));

    m_new_vertices_mappings.push_back(p0);
    m_new_vertices_mappings.push_back(p1);
    m_new_vertices_mappings.push_back(p2);
    m_new_vertices_mappings.push_back(p3);
    m_new_vertices_mappings.push_back(p4);
    m_new_vertices_mappings.push_back(p5);
    m_new_vertices_mappings.push_back(p6);
    m_new_vertices_mappings.push_back(p7);
    m_new_vertices_mappings.push_back(p8);
    return res;
}


void BinarySpacePartition::sort_polyhedron_facet(uint32_t facet)
{
    vector<uint32_t>& edges = m_polyhedrons_facets[facet]->edges;
    for(uint32_t i=1; i<edges.size(); i++)
    {
        uint32_t last = edges[i-1];
        for(uint32_t j=i; j<edges.size(); j++)
        {
            uint32_t next = edges[j];
            if(m_polyhedrons_edges[last]->e0 == m_polyhedrons_edges[next]->e0 ||
               m_polyhedrons_edges[last]->e0 == m_polyhedrons_edges[next]->e1 ||
               m_polyhedrons_edges[last]->e1 == m_polyhedrons_edges[next]->e0 ||
               m_polyhedrons_edges[last]->e1 == m_polyhedrons_edges[next]->e1)
            {
                swap(edges[i],edges[j]);
                break;
            }
        }
    }
}

void BinarySpacePartition::get_polyhedron_facet_vertices(uint32_t facet, deque<uint32_t>& res)
{
    res.clear();
    for(uint32_t i=0; i<m_polyhedrons_facets[facet]->edges.size()-1; i++)
    {
        uint32_t e0 = m_polyhedrons_edges[m_polyhedrons_facets[facet]->edges[i]]->e0;
        uint32_t e1 = m_polyhedrons_edges[m_polyhedrons_facets[facet]->edges[i]]->e1;
        if(0 == i)
        {
            res.push_back(e0);
            res.push_back(e1);
            continue;
        }
        uint32_t be0 = res.front();
        uint32_t be1 = res.back();
     
        if(be0 == e0)
        {
            res.push_front(e1);
        }
        else if(be0 == e1)
        {
            res.push_front(e0);
        }
        else if(be1 == e0)
        {
            res.push_back(e1);
        }
        else if(be1 == e1)
        {
            res.push_back(e0);
        }
        else
        {
            throw "something is wrong";
        }
    }
    
    if(res.size() < 3)
    {
        throw "something is wrong";
    }
    for(uint32_t i=0; i<res.size(); i++)
    {
        for(uint32_t j=i+1; j<res.size(); j++)
        {
            if(res[i] == res[j])
            {
                throw "something is wrong";
            }
        }
    }
}
