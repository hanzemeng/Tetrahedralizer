#include "binary_space_partition.hpp"

#pragma mark - External_Handle

BinarySpacePartitionHandle::BinarySpacePartitionHandle()
{}
void BinarySpacePartitionHandle::Dispose()
{
    delete_vertices(m_vertices);
}

void BinarySpacePartitionHandle::AddInput(uint32_t explicit_count, double* explicit_values, uint32_t tetrahedrons_count, uint32_t* tetrahedrons, uint32_t constraints_count, uint32_t* constraints, bool aggressively_add_virtual_constraints)
{
    m_vertices = create_vertices(explicit_count, explicit_values, 0, nullptr);
    m_tetrahedralization.assign_tetrahedrons(tetrahedrons, tetrahedrons_count);
    m_constraints = create_constraints(constraints_count, constraints, m_vertices.data(), false);
    m_aggressively_add_virtual_constraints = aggressively_add_virtual_constraints;
}
void BinarySpacePartitionHandle::Calculate()
{
    binary_space_partition();
}

uint32_t BinarySpacePartitionHandle::GetInsertedVerticesCount()
{
    return m_inserted_vertices_count;
}
uint32_t* BinarySpacePartitionHandle::GetInsertedVertices()
{
    return m_inserted_vertices.data();
}

uint32_t BinarySpacePartitionHandle::GetPolyhedronsCount()
{
    return m_output_polyhedrons_count;
}
uint32_t* BinarySpacePartitionHandle::GetPolyhedrons()
{
    return m_output_polyhedrons.data();
}

uint32_t BinarySpacePartitionHandle::GetFacetsCount()
{
    return m_output_facets_count;
}
uint32_t* BinarySpacePartitionHandle::GetFacets()
{
    return m_output_facets.data();
}
uint32_t* BinarySpacePartitionHandle::GetFacetsCentroids()
{
    return m_output_facets_centroids.data();
}
double* BinarySpacePartitionHandle::GetFacetsCentroidsWeights()
{
    return m_output_facets_centroids_weights.data();
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

extern "C" LIBRARY_EXPORT void AddBinarySpacePartitionInput(void* handle, uint32_t explicit_count, double* explicit_values, uint32_t tetrahedron_count, uint32_t* tetrahedrons, uint32_t constraints_count, uint32_t* constraints, bool aggressively_add_virtual_constraints)
{
    ((BinarySpacePartitionHandle*)handle)->AddInput(explicit_count, explicit_values, tetrahedron_count, tetrahedrons, constraints_count, constraints, aggressively_add_virtual_constraints);
}

extern "C" LIBRARY_EXPORT void CalculateBinarySpacePartition(void* handle)
{
    ((BinarySpacePartitionHandle*)handle)->Calculate();
}

extern "C" LIBRARY_EXPORT uint32_t GetBinarySpacePartitionInsertedVerticesCount(void* handle)
{
    return ((BinarySpacePartitionHandle*)handle)->GetInsertedVerticesCount();
}
extern "C" LIBRARY_EXPORT uint32_t* GetBinarySpacePartitionInsertedVertices(void* handle)
{
    return  ((BinarySpacePartitionHandle*)handle)->GetInsertedVertices();
}

extern "C" LIBRARY_EXPORT uint32_t GetBinarySpacePartitionPolyhedronsCount(void* handle)
{
    return ((BinarySpacePartitionHandle*)handle)->GetPolyhedronsCount();
}
extern "C" LIBRARY_EXPORT uint32_t* GetBinarySpacePartitionPolyhedrons(void* handle)
{
    return ((BinarySpacePartitionHandle*)handle)->GetPolyhedrons();
}

extern "C" LIBRARY_EXPORT uint32_t GetBinarySpacePartitionFacetsCount(void* handle)
{
    return ((BinarySpacePartitionHandle*)handle)->GetFacetsCount();
}
extern "C" LIBRARY_EXPORT uint32_t* GetBinarySpacePartitionFacets(void* handle)
{
    return ((BinarySpacePartitionHandle*)handle)->GetFacets();
}
extern "C" LIBRARY_EXPORT uint32_t* GetBinarySpacePartitionFacetsCentroids(void* handle)
{
    return ((BinarySpacePartitionHandle*)handle)->GetFacetsCentroids();
}
extern "C" LIBRARY_EXPORT double* GetBinarySpacePartitionFacetsCentroidsWeights(void* handle)
{
    return ((BinarySpacePartitionHandle*)handle)->GetFacetsCentroidsWeights();
}

#pragma mark - Internal_Core

PolyhedronEdge::PolyhedronEdge(){}
PolyhedronEdge::PolyhedronEdge(const PolyhedronEdge& other)
{
    this->e0 = other.e0;
    this->e1 = other.e1;
    this->p0 = other.p0;
    this->p1 = other.p1;
    this->p2 = other.p2;
    this->p3 = other.p3;
    this->p4 = other.p4;
    this->p5 = other.p5;
}

PolyhedronFacet::PolyhedronFacet(){}
PolyhedronFacet::PolyhedronFacet(const PolyhedronFacet& other)
{
    this->edges = other.edges;
    this->p0 = other.p0;
    this->p1 = other.p1;
    this->p2 = other.p2;
    this->ip0 = other.ip0;
    this->ip1 = other.ip1;
}


void BinarySpacePartitionHandle::binary_space_partition()
{
    // convert tetrahedrons to polyhedrons
    {
        m_polyhedrons.clear();
        m_polyhedrons_facets.clear();
        m_polyhedrons_edges.clear();
        m_u_map_ii_i_0.clear();
        m_u_map_iii_i_0.clear();
        for(uint32_t i=0; i<m_tetrahedralization.get_tetrahedrons_count(); i++)
        {
            uint32_t p = m_polyhedrons.size();
            m_polyhedrons.push_back(vector<uint32_t>());
            
            for(uint32_t j=0; j<4; j++)
            {
                uint32_t p0,p1,p2;
                uint32_t e0,e1,e2;
                uint32_t f;
                m_tetrahedralization.get_tetrahedron_facet(i,j,p0,p1,p2);
                
                e0 = find_or_add_edge(p0,p1);
                e1 = find_or_add_edge(p1,p2);
                e2 = find_or_add_edge(p2,p0);
                f = find_or_add_facet(e0,e1,e2,p0,p1,p2);
                if(UNDEFINED_VALUE == m_polyhedrons_facets[f].ip0)
                {
                    m_polyhedrons_facets[f].ip0 = p;
                }
                else
                {
                    m_polyhedrons_facets[f].ip1 = p;
                }
                m_polyhedrons[p].push_back(f);
            }
        }
    }
    
    // create virtual constraints
    {
        uint32_t input_constraints_count = m_constraints.size()/3;
        m_u_map_ii_vector_i_0.clear(); // key is an edge, value is the edge's incident constraints
        for(uint32_t i=0; i<input_constraints_count; i++)
        {
            uint32_t c0 = m_constraints[3*i+0];
            uint32_t c1 = m_constraints[3*i+1];
            uint32_t c2 = m_constraints[3*i+2];

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
                m_u_set_i_0.insert(m_constraints[3*c+0]);
                m_u_set_i_0.insert(m_constraints[3*c+1]);
                m_u_set_i_0.insert(m_constraints[3*c+2]);
            }
            m_u_set_i_0.erase(e0);
            m_u_set_i_0.erase(e1);

            uint32_t e2 = *m_u_set_i_0.begin();
            m_u_set_i_0.erase(m_u_set_i_0.begin());
            bool constrains_are_coplanar = true;
            for(uint32_t e3 : m_u_set_i_0)
            {
                if(0 != orient3d(e3,e0,e1,e2, m_vertices.data()))
                {
                    constrains_are_coplanar = false;
                    break;
                }
            }
            if(!constrains_are_coplanar)
            {
                continue;
            }
            
            if(m_aggressively_add_virtual_constraints)
            {
                add_virtual_constraint(e0,e1, it.second[0]);
                continue;
            }
            int ignore_axis = max_component_in_triangle_normal(e0,e1,e2, m_vertices.data());
            int oe2 = orient3d_ignore_axis(e0,e1,e2,ignore_axis, m_vertices.data());
            bool should_add_virtual_constraint = true;
            for(uint32_t e3 : m_u_set_i_0)
            {
                int oe3 = orient3d_ignore_axis(e0,e1,e3,ignore_axis, m_vertices.data());
                if(0 == oe3)
                {
                    continue;
                }
                if(0 == oe2)
                {
                    oe2 = oe3;
                }
                if(oe2 != oe3)
                {
                    should_add_virtual_constraint = false;
                    break;
                }
            }
            if(0 != oe2 && should_add_virtual_constraint)
            {
                add_virtual_constraint(e0,e1, it.second[0]);
            }
        }
    }

    map<uint32_t, vector<uint32_t>> polyhedrons_intersect_constraints;
    // for every polyhedron, find the constraints that intersect it.
    {
        for(uint32_t i=0; i<m_constraints.size()/3; i++)
        {
            uint32_t c0 = m_constraints[3*i+0];
            uint32_t c1 = m_constraints[3*i+1];
            uint32_t c2 = m_constraints[3*i+2];

            m_u_set_i_0.clear(); // stores improperly intersected tetrahedrons
            m_u_set_i_1.clear(); // stores visited polyhedrons
            m_u_set_i_1.insert(UNDEFINED_VALUE);
            clear_queue(m_queue_i_0); // stores tetrahedrons to be visited
            m_queue_i_0.push(m_tetrahedralization.get_vertex_incident_tetrahedron(c0));
            m_queue_i_0.push(m_tetrahedralization.get_vertex_incident_tetrahedron(c1));
            m_queue_i_0.push(m_tetrahedralization.get_vertex_incident_tetrahedron(c2));

            while(!m_queue_i_0.empty())
            {
                uint32_t t = m_queue_i_0.front();
                m_queue_i_0.pop();
                if(m_u_set_i_1.end() != m_u_set_i_1.find(t))
                {
                    continue;
                }
                m_u_set_i_1.insert(t);

                int int_type = triangle_tetrahedron_intersection(c0, c1, c2,
                                                                 m_tetrahedralization.get_tetrahedron_vertex(t,0),
                                                                 m_tetrahedralization.get_tetrahedron_vertex(t,1),
                                                                 m_tetrahedralization.get_tetrahedron_vertex(t,2),
                                                                 m_tetrahedralization.get_tetrahedron_vertex(t,3),
                                                                 m_vertices.data());
                if(0 == int_type)
                {
                    continue;
                }
                if(2 == int_type)
                {
                    m_u_set_i_0.insert(t);
                }
                
                for(uint32_t f=0; f<4; f++)
                {
                    m_queue_i_0.push(m_tetrahedralization.get_tetrahedron_neighbor(t, f));
                }
            }

            m_u_set_i_0.erase(UNDEFINED_VALUE);
            // if the constraint improperly intersects a tetrahedron, they are associated
            for(uint32_t t : m_u_set_i_0)
            {
                polyhedrons_intersect_constraints[t].push_back(i);
            }
        }
    }

    m_inserted_vertices_count = 0;
    // slice each polyhedron with its list of improper constraints.
    {
        for(auto it=polyhedrons_intersect_constraints.begin(); it!=polyhedrons_intersect_constraints.end(); it++)
        {
            while(!it->second.empty())
            {
                uint32_t p = it->first;
                uint32_t c = it->second.back();
                it->second.pop_back();
                uint32_t c0 = m_constraints[3*c+0];
                uint32_t c1 = m_constraints[3*c+1];
                uint32_t c2 = m_constraints[3*c+2];
                
                m_vector_i_0.clear(); // top polyhedron's facets
                m_vector_i_1.clear(); // bottom polyhedron's facets
                m_u_set_i_0.clear(); // edges on the constraint
                m_u_map_i_iii_0.clear(); // keys are processed edges, values are (intersection vertex, top edge, bottom edge)
                for(uint32_t f : m_polyhedrons[p])
                {
                    uint32_t i0(UNDEFINED_VALUE), i1(UNDEFINED_VALUE);
                    m_vector_i_2.clear(); // edges on top of the constraint
                    m_vector_i_3.clear(); // edges on bottom of the constraint
                    bool has_edge_on_constraint = false;
                    
                    for(uint32_t e : m_polyhedrons_facets[f].edges)
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
                        
                        uint32_t e0(m_polyhedrons_edges[e].e0), e1(m_polyhedrons_edges[e].e1);
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
                                if(UNDEFINED_VALUE == m_polyhedrons_edges[e].p2)
                                {
                                    uint32_t oe0(m_polyhedrons_edges[e].p0), oe1(m_polyhedrons_edges[e].p1);
                                    new_i = add_LPI(oe0,oe1,c0,c1,c2);
                                }
                                else
                                {
                                    new_i = add_TPI(c0,c1,c2,m_polyhedrons_edges[e].p0,m_polyhedrons_edges[e].p1,m_polyhedrons_edges[e].p2,m_polyhedrons_edges[e].p3,m_polyhedrons_edges[e].p4,m_polyhedrons_edges[e].p5);
                                }
                                
                                m_polyhedrons_edges[e].e0 = e0;
                                m_polyhedrons_edges[e].e1 = new_i;
                                m_vector_i_2.push_back(e);
                                uint32_t bottom_e = m_polyhedrons_edges.size();
                                m_polyhedrons_edges.push_back(PolyhedronEdge(m_polyhedrons_edges[e]));
                                m_polyhedrons_edges[bottom_e].e0 = new_i;
                                m_polyhedrons_edges[bottom_e].e1 = e1;
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
                                if(UNDEFINED_VALUE == m_polyhedrons_edges[e].p2)
                                {
                                    uint32_t oe0(m_polyhedrons_edges[e].p0), oe1(m_polyhedrons_edges[e].p1);
                                    new_i = add_LPI(oe0,oe1,c0,c1,c2);
                                }
                                else
                                {
                                    new_i = add_TPI(c0,c1,c2,m_polyhedrons_edges[e].p0,m_polyhedrons_edges[e].p1,m_polyhedrons_edges[e].p2,m_polyhedrons_edges[e].p3,m_polyhedrons_edges[e].p4,m_polyhedrons_edges[e].p5);
                                }
                                
                                m_polyhedrons_edges[e].e0 = e1;
                                m_polyhedrons_edges[e].e1 = new_i;
                                m_vector_i_2.push_back(e);
                                uint32_t bottom_e = m_polyhedrons_edges.size();
                                m_polyhedrons_edges.push_back(PolyhedronEdge(m_polyhedrons_edges[e]));
                                m_polyhedrons_edges[bottom_e].e0 = new_i;
                                m_polyhedrons_edges[bottom_e].e1 = e0;
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
                        m_polyhedrons_edges.push_back(PolyhedronEdge());
                        m_polyhedrons_edges[i_e].e0 = i0;
                        m_polyhedrons_edges[i_e].e1 = i1;
                        m_polyhedrons_edges[i_e].p0 = m_polyhedrons_facets[f].p0;
                        m_polyhedrons_edges[i_e].p1 = m_polyhedrons_facets[f].p1;
                        m_polyhedrons_edges[i_e].p2 = m_polyhedrons_facets[f].p2;
                        m_polyhedrons_edges[i_e].p3 = c0;
                        m_polyhedrons_edges[i_e].p4 = c1;
                        m_polyhedrons_edges[i_e].p5 = c2;
                        m_u_set_i_0.insert(i_e);
                        
                        m_polyhedrons_facets[f].edges.clear();
                        uint32_t bottom_f = m_polyhedrons_facets.size();
                        m_polyhedrons_facets.push_back(PolyhedronFacet(m_polyhedrons_facets[f]));
                        m_polyhedrons_facets[f].edges = m_vector_i_2;
                        m_polyhedrons_facets[f].edges.push_back(i_e);
                        m_polyhedrons_facets[bottom_f].edges = m_vector_i_3;
                        m_polyhedrons_facets[bottom_f].edges.push_back(i_e);
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
                m_polyhedrons.push_back(vector<uint32_t>());
                
                uint32_t common_facet = m_polyhedrons_facets.size();
                m_polyhedrons_facets.push_back(PolyhedronFacet());
                m_polyhedrons_facets[common_facet].p0 = c0;
                m_polyhedrons_facets[common_facet].p1 = c1;
                m_polyhedrons_facets[common_facet].p2 = c2;
                m_polyhedrons_facets[common_facet].ip0 = p;
                m_polyhedrons_facets[common_facet].ip1 = bottom_polyhedron;
                for(uint32_t e : m_u_set_i_0)
                {
                    m_polyhedrons_facets[common_facet].edges.push_back(e);
                }
                
                m_polyhedrons[p] = m_vector_i_0;
                m_polyhedrons[p].push_back(common_facet);
                m_polyhedrons[bottom_polyhedron] = m_vector_i_1;
                m_polyhedrons[bottom_polyhedron].push_back(common_facet);
                
                for(uint32_t f : m_polyhedrons[bottom_polyhedron])
                {
                    if(f == common_facet)
                    {
                        continue;
                    }
                    uint32_t n;
                    if(p == m_polyhedrons_facets[f].ip0)
                    {
                        m_polyhedrons_facets[f].ip0 = bottom_polyhedron;
                        n = m_polyhedrons_facets[f].ip1;
                    }
                    else
                    {
                        m_polyhedrons_facets[f].ip1 = bottom_polyhedron;
                        n = m_polyhedrons_facets[f].ip0;
                    }
                    if(n != UNDEFINED_VALUE)
                    {
                        if(m_polyhedrons[n].end() == find(m_polyhedrons[n].begin(), m_polyhedrons[n].end(), f))
                        {
                            m_polyhedrons[n].push_back(f);
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
                    m_queue_i_0.push(p);
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
                        for(uint32_t f : m_polyhedrons[cur])
                        {
                            if(m_polyhedrons_facets[f].edges.end() != find(m_polyhedrons_facets[f].edges.begin(),m_polyhedrons_facets[f].edges.end(), original_e))
                            {
                                search_neighbor = true;
                                if(m_polyhedrons[p].end() != find(m_polyhedrons[p].begin(), m_polyhedrons[p].end(), f))
                                {
                                    continue;
                                }
                                if(m_polyhedrons[bottom_polyhedron].end() != find(m_polyhedrons[bottom_polyhedron].begin(), m_polyhedrons[bottom_polyhedron].end(), f))
                                {
                                    continue;
                                }
                                if(m_polyhedrons_facets[f].edges.end() == find(m_polyhedrons_facets[f].edges.begin(),m_polyhedrons_facets[f].edges.end(), bottom_e))
                                {
                                    m_polyhedrons_facets[f].edges.push_back(bottom_e);
                                }
                            }
                        }
                        
                        if(search_neighbor)
                        {
                            for(uint32_t f : m_polyhedrons[cur])
                            {
                                if(cur == m_polyhedrons_facets[f].ip0)
                                {
                                    m_queue_i_0.push(m_polyhedrons_facets[f].ip1);
                                }
                                else
                                {
                                    m_queue_i_0.push(m_polyhedrons_facets[f].ip0);
                                }
                            }
                        }
                    }
                }
                
                m_vector_i_0.clear(); // store constraints that intersect top polyhedron
                for(uint32_t j=0; j<it->second.size(); j++)
                {
                    uint32_t nc0 = m_constraints[3*it->second[j]+0];
                    uint32_t nc1 = m_constraints[3*it->second[j]+1];
                    uint32_t nc2 = m_constraints[3*it->second[j]+2];
                    int o0(orient3d(c0,c1,c2,nc0, m_vertices.data())), o1(orient3d(c0,c1,c2,nc1, m_vertices.data())), o2(orient3d(c0,c1,c2,nc2, m_vertices.data()));

                    if(1 == o0 || 1 == o1 || 1 == o2) // constraints may not improperly intersect new polyhedrons, but the slicing algorithm can handle it
                    {
                        m_vector_i_0.push_back(it->second[j]);
                    }
                    if(-1 == o0 || -1 == o1 || -1 == o2)
                    {
                        polyhedrons_intersect_constraints[bottom_polyhedron].push_back(it->second[j]);
                    }
                }
                it->second = m_vector_i_0;
            }
        }
    }

    // produce output
    {
        m_output_polyhedrons_count = m_polyhedrons.size();
        for(uint32_t i=0; i<m_polyhedrons.size(); i++)
        {
            m_output_polyhedrons.push_back(m_polyhedrons[i].size());
            for(uint32_t j=0; j<m_polyhedrons[i].size(); j++)
            {
                m_output_polyhedrons.push_back(m_polyhedrons[i][j]);
            }
        }

        m_output_facets_count = m_polyhedrons_facets.size();
        for(uint32_t i=0; i<m_polyhedrons_facets.size(); i++)
        {
            sort_polyhedron_facet(i);
            get_polyhedron_facet_vertices(i, m_deque_i_0);
            m_output_facets.push_back(m_deque_i_0.size());
            for(uint32_t j=0; j<m_deque_i_0.size(); j++)
            {
                m_output_facets.push_back(m_deque_i_0[j]);
            }
            
            double3 centroid = approximate_facet_centroid(m_deque_i_0, m_vertices.data());
            double3 weight;
            barycentric_weight(m_polyhedrons_facets[i].p0,m_polyhedrons_facets[i].p1,m_polyhedrons_facets[i].p2,centroid,m_vertices.data(), weight);
            m_output_facets_centroids.push_back(m_polyhedrons_facets[i].p0);
            m_output_facets_centroids.push_back(m_polyhedrons_facets[i].p1);
            m_output_facets_centroids.push_back(m_polyhedrons_facets[i].p2);
            m_output_facets_centroids_weights.push_back(weight.x);
            m_output_facets_centroids_weights.push_back(weight.y);
        }
    }
}

#pragma mark - Internal_Helper

uint32_t BinarySpacePartitionHandle::find_or_add_edge(uint32_t p0, uint32_t p1)
{
    sort_ints(p0,p1);
    if(m_u_map_ii_i_0.end() != m_u_map_ii_i_0.find(make_pair(p0,p1)))
    {
        return m_u_map_ii_i_0[make_pair(p0,p1)];
    }

    m_u_map_ii_i_0[make_pair(p0,p1)] = m_polyhedrons_edges.size();

    uint32_t e = m_polyhedrons_edges.size();
    m_polyhedrons_edges.push_back(PolyhedronEdge());
    m_polyhedrons_edges[e].e0 = p0;
    m_polyhedrons_edges[e].e1 = p1;
    m_polyhedrons_edges[e].p0 = p0;
    m_polyhedrons_edges[e].p1 = p1;
    m_polyhedrons_edges[e].p2 = m_polyhedrons_edges[e].p3 = m_polyhedrons_edges[e].p4 = m_polyhedrons_edges[e].p5 = UNDEFINED_VALUE;
    return e;
}
uint32_t BinarySpacePartitionHandle::find_or_add_facet(uint32_t e0, uint32_t e1, uint32_t e2, uint32_t p0, uint32_t p1,  uint32_t p2)
{
    sort_ints(e0,e1,e2);
    if(m_u_map_iii_i_0.end() != m_u_map_iii_i_0.find(make_tuple(e0,e1,e2)))
    {
        return m_u_map_iii_i_0[make_tuple(e0,e1,e2)];
    }
    m_u_map_iii_i_0[make_tuple(e0,e1,e2)] = m_polyhedrons_facets.size();

    uint32_t f = m_polyhedrons_facets.size();
    m_polyhedrons_facets.push_back(PolyhedronFacet());
    m_polyhedrons_facets[f].edges.push_back(e0);
    m_polyhedrons_facets[f].edges.push_back(e1);
    m_polyhedrons_facets[f].edges.push_back(e2);
    m_polyhedrons_facets[f].p0 = p0;
    m_polyhedrons_facets[f].p1 = p1;
    m_polyhedrons_facets[f].p2 = p2;
    m_polyhedrons_facets[f].ip0 = UNDEFINED_VALUE;
    m_polyhedrons_facets[f].ip1 = UNDEFINED_VALUE;
    return f;
}

void BinarySpacePartitionHandle::add_virtual_constraint(uint32_t e0, uint32_t e1, uint32_t c) // e0 and e1 incident the constraint c
{
    uint32_t t = m_tetrahedralization.get_vertex_incident_tetrahedron(e0);
    uint32_t c0 = m_constraints[3*c+0];
    uint32_t c1 = m_constraints[3*c+1];
    uint32_t c2 = m_constraints[3*c+2];
    for(uint64_t i=0; i<4; i++)
    {
        uint32_t v = m_tetrahedralization.get_tetrahedron_vertex(t, i);
        if(c0 == v || c1 == v || c2 == v || 0 == orient3d(v,c0,c1,c2, m_vertices.data()))
        {
            continue;
        }
        m_constraints.push_back(e0);
        m_constraints.push_back(e1);
        m_constraints.push_back(v);
        return;
    }
    throw "can't add virtual constraint";
}


uint32_t BinarySpacePartitionHandle::add_LPI(uint32_t e0, uint32_t e1, uint32_t p0,uint32_t p1,uint32_t p2)
{
    uint32_t res = m_vertices.size();
    m_vertices.push_back(new implicitPoint3D_LPI(
        m_vertices[e0]->toExplicit3D(),m_vertices[e1]->toExplicit3D(),
        m_vertices[p0]->toExplicit3D(),m_vertices[p1]->toExplicit3D(),m_vertices[p2]->toExplicit3D()));

    m_inserted_vertices_count++;
    m_inserted_vertices.push_back(5);
    m_inserted_vertices.push_back(e0);
    m_inserted_vertices.push_back(e1);
    m_inserted_vertices.push_back(p0);
    m_inserted_vertices.push_back(p1);
    m_inserted_vertices.push_back(p2);
    return res;
}
uint32_t BinarySpacePartitionHandle::add_TPI(uint32_t p0,uint32_t p1,uint32_t p2,uint32_t p3,uint32_t p4,uint32_t p5,uint32_t p6,uint32_t p7,uint32_t p8)
{
    uint32_t res = m_vertices.size();
    m_vertices.push_back(new implicitPoint3D_TPI(
        m_vertices[p0]->toExplicit3D(),m_vertices[p1]->toExplicit3D(),m_vertices[p2]->toExplicit3D(),
        m_vertices[p3]->toExplicit3D(),m_vertices[p4]->toExplicit3D(),m_vertices[p5]->toExplicit3D(),
        m_vertices[p6]->toExplicit3D(),m_vertices[p7]->toExplicit3D(),m_vertices[p8]->toExplicit3D()));

    m_inserted_vertices_count++;
    m_inserted_vertices.push_back(9);
    m_inserted_vertices.push_back(p0);
    m_inserted_vertices.push_back(p1);
    m_inserted_vertices.push_back(p2);
    m_inserted_vertices.push_back(p3);
    m_inserted_vertices.push_back(p4);
    m_inserted_vertices.push_back(p5);
    m_inserted_vertices.push_back(p6);
    m_inserted_vertices.push_back(p7);
    m_inserted_vertices.push_back(p8);
    return res;
}


void BinarySpacePartitionHandle::sort_polyhedron_facet(uint32_t facet)
{
    vector<uint32_t>& edges = m_polyhedrons_facets[facet].edges;
    for(uint32_t i=1; i<edges.size(); i++)
    {
        uint32_t last = edges[i-1];
        for(uint32_t j=i; j<edges.size(); j++)
        {
            uint32_t next = edges[j];
            if(m_polyhedrons_edges[last].e0 == m_polyhedrons_edges[next].e0 ||
               m_polyhedrons_edges[last].e0 == m_polyhedrons_edges[next].e1 ||
               m_polyhedrons_edges[last].e1 == m_polyhedrons_edges[next].e0 ||
               m_polyhedrons_edges[last].e1 == m_polyhedrons_edges[next].e1)
            {
                swap(edges[i],edges[j]);
                break;
            }
        }
    }
}

void BinarySpacePartitionHandle::get_polyhedron_facet_vertices(uint32_t facet, deque<uint32_t>& res)
{
    res.clear();
    for(uint32_t i=0; i<m_polyhedrons_facets[facet].edges.size()-1; i++)
    {
        uint32_t e0 = m_polyhedrons_edges[m_polyhedrons_facets[facet].edges[i]].e0;
        uint32_t e1 = m_polyhedrons_edges[m_polyhedrons_facets[facet].edges[i]].e1;
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
    
    // make the first 3 points not collinear to help with future computation
    uint32_t n = res.size();
    while(n-->0 && is_collinear(res[0], res[1], res[2], m_vertices.data()))
    {
        uint32_t front = res[0];
        res.pop_front();
        res.push_back(front);
    }
//    if(n<0)
//    {
//        throw "something is wrong";
//    }
    
//    if(res.size() < 3)
//    {
//        throw "something is wrong";
//    }
//    for(uint32_t i=0; i<res.size(); i++)
//    {
//        for(uint32_t j=i+1; j<res.size(); j++)
//        {
//            if(res[i] == res[j])
//            {
//                throw "something is wrong";
//            }
//        }
//    }
}
