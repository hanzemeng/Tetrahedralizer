#include "binary_space_partition.hpp"
using namespace std;

void BinarySpacePartitionHandle::Dispose()
{}
void BinarySpacePartitionHandle::AddInput(uint32_t explicit_count, double* explicit_values, uint32_t tetrahedrons_count, uint32_t* tetrahedrons, uint32_t constraints_count, uint32_t* constraints, bool aggressively_add_virtual_constraints)
{
    m_vertices = create_vertices(explicit_count, explicit_values, 0, nullptr);
    m_tetrahedralization.assign_tetrahedrons(tetrahedrons, tetrahedrons_count);
    m_constraints = create_constraints(constraints_count, constraints, m_vertices.data(), true);
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
    return m_facets.size();
}
void BinarySpacePartitionHandle::GetFacets(FacetInteropData* outArray)
{
    for(uint32_t i=0; i<m_facets.size(); i++)
    {
        outArray[i] = m_facets[i];
    }
}
uint32_t BinarySpacePartitionHandle::GetSegmentsCount()
{
    return m_segments.size();
}
void BinarySpacePartitionHandle::GetSegments(SegmentInteropData* outArray)
{
    for(uint32_t i=0; i<m_segments.size(); i++)
    {
        outArray[i] = m_segments[i];
    }
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
extern "C" LIBRARY_EXPORT void GetBinarySpacePartitionFacets(void* handle, FacetInteropData* outArray)
{
    ((BinarySpacePartitionHandle*)handle)->GetFacets(outArray);
}
extern "C" LIBRARY_EXPORT uint32_t GetBinarySpacePartitionSegmentsCount(void* handle)
{
    return ((BinarySpacePartitionHandle*)handle)->GetSegmentsCount();
}
extern "C" LIBRARY_EXPORT void GetBinarySpacePartitionSegments(void* handle, SegmentInteropData* outArray)
{
    ((BinarySpacePartitionHandle*)handle)->GetSegments(outArray);
}


void BinarySpacePartitionHandle::binary_space_partition()
{
    // convert tetrahedrons to polyhedrons
    {
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
                if(UNDEFINED_VALUE == m_facets[f].ip0)
                {
                    m_facets[f].ip0 = p;
                }
                else
                {
                    m_facets[f].ip1 = p;
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
            if(UNDEFINED_VALUE == c0)
            {
                continue;
            }

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

    // for every polyhedron, find the constraints that intersect it
    queue<pair<uint32_t,uint32_t>> polyhedrons_slice_order; // first is polyhedron, second is the root node of its slice tree
    {
        map<uint32_t, vector<tuple<uint32_t, vector<Segment>, vector<shared_ptr<genericPoint>>>>> polyhedrons_intersect_constraints;
        for(uint32_t i=0; i<m_constraints.size()/3; i++)
        {
            uint32_t c0 = m_constraints[3*i+0];
            uint32_t c1 = m_constraints[3*i+1];
            uint32_t c2 = m_constraints[3*i+2];
            if(UNDEFINED_VALUE == c0)
            {
                continue;
            }

            m_u_set_i_0.clear(); // stores visited polyhedrons
            m_u_set_i_0.insert(UNDEFINED_VALUE);
            clear_queue(m_queue_i_0); // stores tetrahedrons to be visited
            m_queue_i_0.push(m_tetrahedralization.get_vertex_incident_tetrahedron(c0));
            m_queue_i_0.push(m_tetrahedralization.get_vertex_incident_tetrahedron(c1));
            m_queue_i_0.push(m_tetrahedralization.get_vertex_incident_tetrahedron(c2));

            while(!m_queue_i_0.empty())
            {
                uint32_t t = m_queue_i_0.front();
                m_queue_i_0.pop();
                if(m_u_set_i_0.end() != m_u_set_i_0.find(t))
                {
                    continue;
                }
                m_u_set_i_0.insert(t);
                
                vector<shared_ptr<genericPoint>> vertices;
                vertices.push_back(m_vertices[c0]);
                vertices.push_back(m_vertices[c1]);
                vertices.push_back(m_vertices[c2]);
                vertices.push_back(m_vertices[m_tetrahedralization.get_tetrahedron_vertex(t,0)]);
                vertices.push_back(m_vertices[m_tetrahedralization.get_tetrahedron_vertex(t,1)]);
                vertices.push_back(m_vertices[m_tetrahedralization.get_tetrahedron_vertex(t,2)]);
                vertices.push_back(m_vertices[m_tetrahedralization.get_tetrahedron_vertex(t,3)]);
                auto [int_type, edges] = TriangleTetrahedronIntersection::triangle_tetrahedron_intersection(0,1,2,3,4,5,6, vertices);
                if(0 == int_type)
                {
                    continue;
                }
                if(2 == int_type)
                {
                    polyhedrons_intersect_constraints[t].push_back(make_tuple(i, edges, vertices));
                }
                
                for(uint32_t f=0; f<4; f++)
                {
                    m_queue_i_0.push(m_tetrahedralization.get_tetrahedron_neighbor(t, f));
                }
            }
        }
        
        for(auto& [p, slices] : polyhedrons_intersect_constraints)
        {
            polyhedrons_slice_order.push(make_pair(p, build_polyhedrons_slice_tree(slices)));
        }
    }
    

    m_inserted_vertices_count = 0;
    // slice each polyhedron with its list of improper constraints.
    while(!polyhedrons_slice_order.empty())
    {
        uint32_t p = polyhedrons_slice_order.front().first;
        PolyhedronConstraint& polyhedronConstraint = m_polyhedrons_slice_tree[polyhedrons_slice_order.front().second];
        polyhedrons_slice_order.pop();
        uint32_t c = polyhedronConstraint.c;
        uint32_t c0 = m_constraints[3*c+0];
        uint32_t c1 = m_constraints[3*c+1];
        uint32_t c2 = m_constraints[3*c+2];
        
        m_vector_i_0.clear(); // top polyhedron's facets
        m_vector_i_1.clear(); // bottom polyhedron's facets
        m_u_set_i_0.clear(); // edges on the constraint
        m_u_map_i_iii_0.clear(); // keys are processed edges, values are (intersection vertex, top edge, bottom edge)
        m_u_map_i_si_0.clear(); // orientation cache
        for(uint32_t f : m_polyhedrons[p])
        {
            m_vector_i_2.clear(); // edges on top of the constraint
            m_vector_i_3.clear(); // edges on bottom of the constraint
            bool has_edge_on_constraint = false;
            uint32_t i0(UNDEFINED_VALUE), i1(UNDEFINED_VALUE);
            for(uint32_t e : m_facets[f].segments)
            {
                if(m_u_map_i_iii_0.end() == m_u_map_i_iii_0.find(e))
                {
                    auto [i_p,top_e,bot_e,vs] = Segment::slice_segment_with_plane(e, c0, c1, c2, m_vertices, m_segments, m_u_map_i_si_0);
                    m_u_map_i_iii_0[e] = make_tuple(i_p,top_e,bot_e);
                    if(0 != vs.size())
                    {
                        m_inserted_vertices_count++;
                        m_inserted_vertices.push_back(vs.size());
                        for(uint32_t i=0; i<vs.size(); i++)
                        {
                            m_inserted_vertices.push_back(vs[i]);
                        }
                    }
                }
                
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
                uint32_t i_e = m_segments.size();
                m_segments.push_back(Segment());
                m_segments[i_e].e0 = i0;
                m_segments[i_e].e1 = i1;
                m_segments[i_e].p0 = m_facets[f].p0;
                m_segments[i_e].p1 = m_facets[f].p1;
                m_segments[i_e].p2 = m_facets[f].p2;
                m_segments[i_e].p3 = c0;
                m_segments[i_e].p4 = c1;
                m_segments[i_e].p5 = c2;
                m_u_set_i_0.insert(i_e);
                
                m_facets[f].segments.clear();
                uint32_t bottom_f = m_facets.size();
                m_facets.push_back(Facet(m_facets[f]));
                m_facets[f].segments = m_vector_i_2;
                m_facets[f].segments.push_back(i_e);
                m_facets[bottom_f].segments = m_vector_i_3;
                m_facets[bottom_f].segments.push_back(i_e);
                m_vector_i_0.push_back(f);
                m_vector_i_1.push_back(bottom_f);
            }
        }
        
        // constraint does not slice the polyhedron
        if(0 == m_vector_i_0.size() || 0 == m_vector_i_1.size())
        {
            if(0 != m_vector_i_0.size() && UNDEFINED_VALUE != polyhedronConstraint.top)
            {
                polyhedrons_slice_order.push(make_pair(p, polyhedronConstraint.top));
            }
            if(0 != m_vector_i_1.size() && UNDEFINED_VALUE != polyhedronConstraint.bot)
            {
                polyhedrons_slice_order.push(make_pair(p, polyhedronConstraint.bot));
            }
            continue;
        }
//        if(m_vector_i_0.size()<3 || m_vector_i_1.size()<3 || m_u_set_i_0.size()<3)
//        {
//            throw "wtf";
//        }
        
        uint32_t bottom_polyhedron = m_polyhedrons.size();
        m_polyhedrons.push_back(vector<uint32_t>());
        
        uint32_t common_facet = m_facets.size();
        m_facets.push_back(Facet());
        m_facets[common_facet].p0 = c0;
        m_facets[common_facet].p1 = c1;
        m_facets[common_facet].p2 = c2;
        m_facets[common_facet].ip0 = p;
        m_facets[common_facet].ip1 = bottom_polyhedron;
        for(uint32_t e : m_u_set_i_0)
        {
            m_facets[common_facet].segments.push_back(e);
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
            if(p == m_facets[f].ip0)
            {
                m_facets[f].ip0 = bottom_polyhedron;
                n = m_facets[f].ip1;
            }
            else
            {
                m_facets[f].ip1 = bottom_polyhedron;
                n = m_facets[f].ip0;
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
                    if(m_facets[f].contains_segment(original_e))
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
                        if(!m_facets[f].contains_segment(bottom_e))
                        {
                            m_facets[f].segments.push_back(bottom_e);
                        }
                    }
                }
                
                if(search_neighbor)
                {
                    for(uint32_t f : m_polyhedrons[cur])
                    {
                        if(cur == m_facets[f].ip0)
                        {
                            m_queue_i_0.push(m_facets[f].ip1);
                        }
                        else
                        {
                            m_queue_i_0.push(m_facets[f].ip0);
                        }
                    }
                }
            }
        }
        
        if(UNDEFINED_VALUE != polyhedronConstraint.top)
        {
            polyhedrons_slice_order.push(make_pair(p, polyhedronConstraint.top));
        }
        if(UNDEFINED_VALUE != polyhedronConstraint.bot)
        {
            polyhedrons_slice_order.push(make_pair(bottom_polyhedron, polyhedronConstraint.bot));
        }
    }
    
    // prepare output and calculate more facet info
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
        for(uint32_t i=0; i<m_facets.size(); i++)
        {
            vector<uint32_t> vs = m_facets[i].get_sorted_vertices(m_segments);
            double3 centroid = approximate_facet_centroid(vs, m_vertices.data());
            double3 weight;
            barycentric_weight(m_facets[i].p0,m_facets[i].p1,m_facets[i].p2,centroid,m_vertices.data(), weight);
            m_facets[i].w0 = weight.x;
            m_facets[i].w1 = weight.y;
        }
        
        vector<vector<uint32_t>> segments_incident_facets = vector<vector<uint32_t>>(m_segments.size());
        vector<vector<uint32_t>> vertices_incident_segments = vector<vector<uint32_t>>(m_vertices.size());
        for(uint32_t i=0; i<m_facets.size(); i++)
        {
            for(uint32_t j=0; j<m_facets[i].segments.size(); j++)
            {
                segments_incident_facets[m_facets[i].segments[j]].push_back(i);
            }
        }
        for(uint32_t i=0; i<m_segments.size(); i++)
        {
            uint32_t v0 = m_segments[i].e0;
            uint32_t v1 = m_segments[i].e1;
            vertices_incident_segments[v0].push_back(i);
            vertices_incident_segments[v1].push_back(i);
        }
        for(uint32_t i=0; i<m_constraints.size()/3; i++)
        {
            uint32_t c0 = m_constraints[3*i+0];
            uint32_t c1 = m_constraints[3*i+1];
            uint32_t c2 = m_constraints[3*i+2];
            if(UNDEFINED_VALUE == c0)
            {
                continue;
            }
            
            clear_queue(m_queue_i_0); // facets to be visited
            for(uint32_t s : vertices_incident_segments[c0])
            {
                for(uint32_t f : segments_incident_facets[s])
                {
                    m_queue_i_0.push(f);
                }
            }
            for(uint32_t s : vertices_incident_segments[c1])
            {
                for(uint32_t f : segments_incident_facets[s])
                {
                    m_queue_i_0.push(f);
                }
            }
            for(uint32_t s : vertices_incident_segments[c2])
            {
                for(uint32_t f : segments_incident_facets[s])
                {
                    m_queue_i_0.push(f);
                }
            }
            m_u_set_i_0.clear(); // visited facets
            
            while(!m_queue_i_0.empty())
            {
                uint32_t f = m_queue_i_0.front();
                m_queue_i_0.pop();
                if(m_u_set_i_0.end() != m_u_set_i_0.find(f))
                {
                    continue;
                }
                m_u_set_i_0.insert(f);
                if(!m_facets[f].is_coplanar_constraint(c0, c1, c2, m_vertices))
                {
                    continue;
                }
                m_facets[f].constrains.push_back(i);
                
                for(uint32_t j=0; j<m_facets[f].segments.size(); j++)
                {
                    for(uint32_t nf : segments_incident_facets[m_facets[f].segments[j]])
                    {
                        m_queue_i_0.push(nf);
                    }
                }
            }
        }
    }
    
}


uint32_t BinarySpacePartitionHandle::find_or_add_edge(uint32_t p0, uint32_t p1)
{
    sort_ints(p0,p1);
    if(m_u_map_ii_i_0.end() != m_u_map_ii_i_0.find(make_pair(p0,p1)))
    {
        return m_u_map_ii_i_0[make_pair(p0,p1)];
    }

    m_u_map_ii_i_0[make_pair(p0,p1)] = m_segments.size();

    uint32_t e = m_segments.size();
    m_segments.push_back(Segment());
    m_segments[e].e0 = p0;
    m_segments[e].e1 = p1;
    m_segments[e].p0 = p0;
    m_segments[e].p1 = p1;
    m_segments[e].p2 = m_segments[e].p3 = m_segments[e].p4 = m_segments[e].p5 = UNDEFINED_VALUE;
    return e;
}
uint32_t BinarySpacePartitionHandle::find_or_add_facet(uint32_t e0, uint32_t e1, uint32_t e2, uint32_t p0, uint32_t p1,  uint32_t p2)
{
    sort_ints(e0,e1,e2);
    if(m_u_map_iii_i_0.end() != m_u_map_iii_i_0.find(make_tuple(e0,e1,e2)))
    {
        return m_u_map_iii_i_0[make_tuple(e0,e1,e2)];
    }
    m_u_map_iii_i_0[make_tuple(e0,e1,e2)] = m_facets.size();

    uint32_t f = m_facets.size();
    m_facets.push_back(Facet());
    m_facets[f].segments.push_back(e0);
    m_facets[f].segments.push_back(e1);
    m_facets[f].segments.push_back(e2);
    m_facets[f].p0 = p0;
    m_facets[f].p1 = p1;
    m_facets[f].p2 = p2;
    m_facets[f].ip0 = UNDEFINED_VALUE;
    m_facets[f].ip1 = UNDEFINED_VALUE;
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

uint32_t BinarySpacePartitionHandle::build_polyhedrons_slice_tree(vector<tuple<uint32_t,vector<Segment>,vector<shared_ptr<genericPoint>>>>& slices)
{
    if(slices.empty())
    {
        return UNDEFINED_VALUE;
    }
    
    uint32_t res = m_polyhedrons_slice_tree.size();
    m_polyhedrons_slice_tree.push_back(PolyhedronConstraint());
    
    uint32_t slice = UNDEFINED_VALUE;
    vector<uint32_t> best_top;
    vector<uint32_t> best_bot;
    vector<uint32_t> best_both;
    for(uint32_t i=0; i<slices.size(); i++)
    {
        uint32_t c = get<0>(slices[i]);
        shared_ptr<genericPoint> c0 = m_vertices[m_constraints[3*c+0]];
        shared_ptr<genericPoint> c1 = m_vertices[m_constraints[3*c+1]];
        shared_ptr<genericPoint> c2 = m_vertices[m_constraints[3*c+2]];
        
        vector<uint32_t> top;
        vector<uint32_t> bot;
        vector<uint32_t> both;
        for(uint32_t j=0; j<slices.size(); j++)
        {
            if(i==j)
            {
                continue;
            }
            const vector<Segment>& segments = get<1>(slices[j]);
            const vector<shared_ptr<genericPoint>>& vertices = get<2>(slices[j]);
            bool has_top = false;
            bool has_bot = false;
            for(const Segment& segment : segments)
            {
                int o0 = orient3d(c0,c1,c2,vertices[segment.e0]);
                int o1 = orient3d(c0,c1,c2,vertices[segment.e1]);
                has_top |= 1==o0 || 1==o1;
                has_bot |= -1==o0 || -1==o1;
                if(has_top && has_bot)
                {
                    break;
                }
            }
            if(has_top && has_bot)
            {
                both.push_back(j);
            }
            else if(has_top && !has_bot)
            {
                top.push_back(j);
            }
            else if(!has_top && has_bot)
            {
                bot.push_back(j);
            }
        }
        if(UNDEFINED_VALUE==slice || best_both.size() > both.size())
        {
            slice = i;
            best_top = std::move(top);
            best_bot = std::move(bot);
            best_both = std::move(both);
        }
    }
    m_polyhedrons_slice_tree[res].c = get<0>(slices[slice]);
    vector<tuple<uint32_t,vector<Segment>,vector<shared_ptr<genericPoint>>>> top_slices;
    vector<tuple<uint32_t,vector<Segment>,vector<shared_ptr<genericPoint>>>> bot_slices;
    for(uint32_t i : best_top)
    {
        top_slices.push_back(slices[i]);
    }
    for(uint32_t i : best_bot)
    {
        bot_slices.push_back(slices[i]);
    }
    for(uint32_t i : best_both)
    {
        uint32_t c = get<0>(slices[slice]);
        const vector<Segment>& segments = get<1>(slices[i]);
        vector<shared_ptr<genericPoint>> vertices = get<2>(slices[i]);
        vertices.push_back(m_vertices[m_constraints[3*c+0]]);
        vertices.push_back(m_vertices[m_constraints[3*c+1]]);
        vertices.push_back(m_vertices[m_constraints[3*c+2]]);
        auto [vertex, top_segments, bot_segments] = Segment::slice_segments_with_plane(segments, 0, 1, 2, vertices.size()-3, vertices.size()-2, vertices.size()-1, vertices);
        
        top_slices.push_back(make_tuple(get<0>(slices[i]),top_segments,vertices));
        bot_slices.push_back(make_tuple(get<0>(slices[i]),bot_segments,vertices));
    }
    m_polyhedrons_slice_tree[res].top = build_polyhedrons_slice_tree(top_slices);
    m_polyhedrons_slice_tree[res].bot = build_polyhedrons_slice_tree(bot_slices);
    return res;
}
