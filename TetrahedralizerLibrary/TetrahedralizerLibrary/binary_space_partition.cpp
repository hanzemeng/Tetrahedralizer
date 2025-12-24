#include "binary_space_partition.hpp"
using namespace std;

void BinarySpacePartitionHandle::Dispose()
{}
void BinarySpacePartitionHandle::AddInput(uint32_t explicit_count, double* explicit_values, uint32_t tetrahedrons_count, uint32_t* tetrahedrons, uint32_t constraints_count, uint32_t* constraints, bool aggressively_add_virtual_constraints)
{
    m_vertices = create_vertices(explicit_count, explicit_values, 0, nullptr);
    m_polyhedralization.m_vertices = m_vertices;
    m_tetrahedralization.assign_tetrahedrons(tetrahedrons, tetrahedrons_count);
    m_constraints = create_constraints(constraints_count, constraints, m_vertices.data(), true);
}
void BinarySpacePartitionHandle::Calculate()
{
    binary_space_partition();
}

uint32_t BinarySpacePartitionHandle::GetInsertedVerticesCount()
{
    return count_nested_vector_size(m_polyhedralization.m_inserted_vertices);
}
void BinarySpacePartitionHandle::GetInsertedVertices(uint32_t* out)
{
    vector<uint32_t> temp = nested_vector_to_flat_vector(m_polyhedralization.m_inserted_vertices);
    write_buffer_with_vector(out, temp);
}

uint32_t BinarySpacePartitionHandle::GetPolyhedronsCount()
{
    return count_nested_vector_size(m_polyhedralization.m_polyhedrons);
}
void BinarySpacePartitionHandle::GetPolyhedrons(uint32_t* out)
{
    vector<uint32_t> temp = nested_vector_to_flat_vector(m_polyhedralization.m_polyhedrons);
    write_buffer_with_vector(out, temp);
}

uint32_t BinarySpacePartitionHandle::GetFacetsCount()
{
    return m_polyhedralization.m_facets.size();
}
void BinarySpacePartitionHandle::GetFacets(FacetInteropData* out)
{
    for(uint32_t i=0; i<m_polyhedralization.m_facets.size(); i++)
    {
        out[i] = m_polyhedralization.m_facets[i];
    }
}
uint32_t BinarySpacePartitionHandle::GetSegmentsCount()
{
    return m_polyhedralization.m_segments.size();
}
void BinarySpacePartitionHandle::GetSegments(SegmentInteropData* out)
{
    for(uint32_t i=0; i<m_polyhedralization.m_segments.size(); i++)
    {
        out[i] = m_polyhedralization.m_segments[i];
    }
}

uint32_t BinarySpacePartitionHandle::GetCoplanarTrianglesCount()
{
    return count_nested_vector_size(m_coplanar_triangles);
}
void BinarySpacePartitionHandle::GetCoplanarTriangles(uint32_t* out)
{
    vector<uint32_t> temp = nested_vector_to_flat_vector(m_coplanar_triangles);
    write_buffer_with_vector(out, temp);
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
extern "C" LIBRARY_EXPORT void GetBinarySpacePartitionInsertedVertices(void* handle, uint32_t* out)
{
    return  ((BinarySpacePartitionHandle*)handle)->GetInsertedVertices(out);
}

extern "C" LIBRARY_EXPORT uint32_t GetBinarySpacePartitionPolyhedronsCount(void* handle)
{
    return ((BinarySpacePartitionHandle*)handle)->GetPolyhedronsCount();
}
extern "C" LIBRARY_EXPORT void GetBinarySpacePartitionPolyhedrons(void* handle, uint32_t* out)
{
    return ((BinarySpacePartitionHandle*)handle)->GetPolyhedrons(out);
}

extern "C" LIBRARY_EXPORT uint32_t GetBinarySpacePartitionFacetsCount(void* handle)
{
    return ((BinarySpacePartitionHandle*)handle)->GetFacetsCount();
}
extern "C" LIBRARY_EXPORT void GetBinarySpacePartitionFacets(void* handle, FacetInteropData* out)
{
    ((BinarySpacePartitionHandle*)handle)->GetFacets(out);
}
extern "C" LIBRARY_EXPORT uint32_t GetBinarySpacePartitionSegmentsCount(void* handle)
{
    return ((BinarySpacePartitionHandle*)handle)->GetSegmentsCount();
}
extern "C" LIBRARY_EXPORT void GetBinarySpacePartitionSegments(void* handle, SegmentInteropData* out)
{
    ((BinarySpacePartitionHandle*)handle)->GetSegments(out);
}
extern "C" LIBRARY_EXPORT uint32_t GetBinarySpacePartitionCoplanarTrianglesCount(void* handle)
{
    return ((BinarySpacePartitionHandle*)handle)->GetCoplanarTrianglesCount();
}
extern "C" LIBRARY_EXPORT void GetBinarySpacePartitionCoplanarTriangles(void* handle, uint32_t* out)
{
    ((BinarySpacePartitionHandle*)handle)->GetCoplanarTriangles(out);
}


void BinarySpacePartitionHandle::binary_space_partition()
{
    // find a triangle to represent all coplanar triangles
    unordered_map<tuple<uint32_t, uint32_t, uint32_t>, uint32_t, trio_iii_hash> triangles_coplanar_groups;
    vector<uint32_t> coplanar_groups_triangles;
    auto get_coplanar_group = [&](uint32_t p0,uint32_t p1,uint32_t p2) -> uint32_t
    {
        sort_ints(p0,p1,p2);
        if(triangles_coplanar_groups.end() == triangles_coplanar_groups.find(make_tuple(p0,p1,p2)))
        {
            return UNDEFINED_VALUE;
        }
        return triangles_coplanar_groups[make_tuple(p0,p1,p2)];
    };
    auto get_coplanar_group_triangle = [&](uint32_t p0,uint32_t p1,uint32_t p2) -> tuple<uint32_t, uint32_t, uint32_t>
    {
        uint32_t c = get_coplanar_group(p0,p1,p2);
        return make_tuple(coplanar_groups_triangles[3*c+0],coplanar_groups_triangles[3*c+1],coplanar_groups_triangles[3*c+2]);
    };
    
    // calculate coplanar groups
    {
        vector<uint32_t> triangles = m_constraints;
        for(uint32_t i=0; i<m_tetrahedralization.get_tetrahedrons_count(); i++)
        {
            for(uint32_t j=0; j<4; j++)
            {
                auto [p0,p1,p2] = m_tetrahedralization.get_tetrahedron_facet(i, j);
                triangles.push_back(p0);
                triangles.push_back(p1);
                triangles.push_back(p2);
            }
        }
        vector<vector<uint32_t>> coplanar_groups = group_coplanar_triangles(triangles, m_vertices);
        for(uint32_t i=0; i<coplanar_groups.size(); i++)
        {
            uint32_t c0 = triangles[3*coplanar_groups[i][0]+0];
            uint32_t c1 = triangles[3*coplanar_groups[i][0]+1];
            uint32_t c2 = triangles[3*coplanar_groups[i][0]+2];
            uint32_t cg = coplanar_groups_triangles.size()/3;
            coplanar_groups_triangles.push_back(c0);
            coplanar_groups_triangles.push_back(c1);
            coplanar_groups_triangles.push_back(c2);
            for(uint32_t j=0; j<coplanar_groups[i].size(); j++)
            {
                uint32_t nc0 = triangles[3*coplanar_groups[i][j]+0];
                uint32_t nc1 = triangles[3*coplanar_groups[i][j]+1];
                uint32_t nc2 = triangles[3*coplanar_groups[i][j]+2];
                sort_ints(nc0, nc1, nc2);
                triangles_coplanar_groups[make_tuple(nc0,nc1,nc2)] = cg;
            }
        }
    }
    // convert tetrahedrons to polyhedrons
    {
        unordered_map<pair<uint32_t, uint32_t>, uint32_t, pair_ii_hash> segments_cache;
        unordered_map<tuple<uint32_t, uint32_t, uint32_t>, uint32_t, trio_iii_hash> facets_cache;
        for(uint32_t i=0; i<m_tetrahedralization.get_tetrahedrons_count(); i++)
        {
            uint32_t p = m_polyhedralization.m_polyhedrons.size();
            m_polyhedralization.m_polyhedrons.push_back(vector<uint32_t>());
            
            for(uint32_t j=0; j<4; j++)
            {
                auto find_segment = [&](uint32_t p0, uint32_t p1) -> uint32_t
                {
                    sort_ints(p0,p1);
                    if(segments_cache.end() == segments_cache.find(make_pair(p0,p1)))
                    {
                        uint32_t s = m_polyhedralization.m_segments.size();
                        segments_cache[make_pair(p0,p1)] = s;
                        m_polyhedralization.m_segments.push_back(Segment(p0,p1));
                    }
                    return segments_cache[make_pair(p0,p1)];
                };
                
                auto [p0,p1,p2] = m_tetrahedralization.get_tetrahedron_facet(i,j);
                sort_ints(p0,p1,p2);
                uint32_t s0 = find_segment(p0, p1);
                uint32_t s1 = find_segment(p1, p2);
                uint32_t s2 = find_segment(p2, p0);
                
                if(facets_cache.end() == facets_cache.find(make_tuple(p0,p1,p2)))
                {
                    auto [cp0,cp1,cp2] = get_coplanar_group_triangle(p0,p1,p2);
                    uint32_t f = m_polyhedralization.m_facets.size();
                    m_polyhedralization.m_facets.push_back(Facet(s0,s1,s2,cp0,cp1,cp2,UNDEFINED_VALUE,UNDEFINED_VALUE));
                    facets_cache[make_tuple(p0,p1,p2)] = f;
                }
                uint32_t f = facets_cache[make_tuple(p0,p1,p2)];
                if(UNDEFINED_VALUE == m_polyhedralization.m_facets[f].ip0)
                {
                    m_polyhedralization.m_facets[f].ip0 = p;
                }
                else
                {
                    m_polyhedralization.m_facets[f].ip1 = p;
                }
                m_polyhedralization.m_polyhedrons[p].push_back(f);
            }
        }
    }
    
    // create virtual constraints
    {
        vector<uint32_t> virtual_constraints;
        auto add_virtual_constraint = [&](uint32_t s0, uint32_t s1, uint32_t c) // e0 and e1 incident the constraint c
        {
            uint32_t t = m_tetrahedralization.get_vertex_incident_tetrahedron(s0);
            uint32_t c0 = m_constraints[3*c+0];
            uint32_t c1 = m_constraints[3*c+1];
            uint32_t c2 = m_constraints[3*c+2];
            auto [cg0,cg1,cg2] = get_coplanar_group_triangle(c0, c1, c2);
            for(uint64_t i=0; i<4; i++)
            {
                uint32_t v = m_tetrahedralization.get_tetrahedron_vertex(t, i);
                if(c0 == v || c1 == v || c2 == v || 0 == orient3d(cg0,cg1,cg2,v,m_vertices.data()))
                {
                    continue;
                }
                if(UNDEFINED_VALUE != get_coplanar_group(s0, s1, v)) // the virtual constraints is already a constraint or tetrahedron facet
                {
                    return;
                }
                virtual_constraints.push_back(s0);
                virtual_constraints.push_back(s1);
                virtual_constraints.push_back(v);
                return;
            }
            throw "can't add virtual constraint";
        };
        
        unordered_map<pair<uint32_t,uint32_t>, vector<uint32_t>, pair_ii_hash> segments_incident_constraints;
        for(uint32_t i=0; i<m_constraints.size()/3; i++)
        {
            uint32_t c0 = m_constraints[3*i+0];
            uint32_t c1 = m_constraints[3*i+1];
            uint32_t c2 = m_constraints[3*i+2];
            if(UNDEFINED_VALUE == c0)
            {
                continue;
            }
            
            auto associate_segments_incident_constraints = [&](uint32_t p0, uint32_t p1)
            {
                sort_ints(p0,p1);
                segments_incident_constraints[make_pair(p0,p1)].push_back(i);
            };
            associate_segments_incident_constraints(c0,c1);
            associate_segments_incident_constraints(c1,c2);
            associate_segments_incident_constraints(c2,c0);
        }
        
        for(auto& [k, constraints] : segments_incident_constraints)
        {
            auto [s0, s1] = k;
            if(1 == constraints.size())
            {
                add_virtual_constraint(s0, s1, constraints[0]);
                continue;
            }
            
            uint32_t c = constraints[0];
            uint32_t c0 = m_constraints[3*c+0];
            uint32_t c1 = m_constraints[3*c+1];
            uint32_t c2 = m_constraints[3*c+2];
            uint32_t cg = get_coplanar_group(c0, c1, c2);
            bool constrains_are_coplanar = true;
            for(uint32_t i=1; i<constraints.size(); i++)
            {
                uint32_t nc = constraints[i];
                uint32_t nc0 = m_constraints[3*nc+0];
                uint32_t nc1 = m_constraints[3*nc+1];
                uint32_t nc2 = m_constraints[3*nc+2];
                uint32_t ncg = get_coplanar_group(nc0, nc1, nc2);
                if(cg != ncg)
                {
                    constrains_are_coplanar = false;
                    break;
                }
            }
            if(!constrains_are_coplanar)
            {
                continue;
            }
            
            unordered_set<uint32_t> vertices; // stores the vertices on incident constraints
            for(uint32_t c : constraints)
            {
                vertices.insert(m_constraints[3*c+0]);
                vertices.insert(m_constraints[3*c+1]);
                vertices.insert(m_constraints[3*c+2]);
            }
            vertices.erase(s0);
            vertices.erase(s1);
            uint32_t s2 = *vertices.begin();
            vertices.erase(vertices.begin());
            
            auto [cc0,cc1,cc2] = get_coplanar_group_triangle(c0,c1,c2);
            int ignore_axis = max_component_in_triangle_normal(cc0,cc1,cc2, m_vertices.data());
            int os2 = orient3d_ignore_axis(s0,s1,s2,ignore_axis, m_vertices.data());
            bool all_vertices_on_same_side = true;
            for(uint32_t s3 : vertices)
            {
                int os3 = orient3d_ignore_axis(s0,s1,s3,ignore_axis, m_vertices.data());
                if(0 == os3)
                {
                    continue;
                }
                if(0 == os2)
                {
                    os2 = os3;
                }
                if(os2 != os3)
                {
                    all_vertices_on_same_side = false;
                    break;
                }
            }
            if(0 != os2 && all_vertices_on_same_side)
            {
                add_virtual_constraint(s0,s1, constraints[0]);
            }
        }
        
        // combine coplanar group
        vector<vector<uint32_t>> virtual_constraints_coplanar_group = group_coplanar_triangles(virtual_constraints, m_vertices);
        for(uint32_t i=0; i<virtual_constraints_coplanar_group.size(); i++)
        {
            uint32_t vcg = UNDEFINED_VALUE;
            for(uint32_t j=0; j<virtual_constraints_coplanar_group[i].size(); j++)
            {
                uint32_t vc = virtual_constraints_coplanar_group[i][j];
                uint32_t vc0 = virtual_constraints[3*vc+0];
                uint32_t vc1 = virtual_constraints[3*vc+1];
                uint32_t vc2 = virtual_constraints[3*vc+2];
                
                for(auto [k,v] : triangles_coplanar_groups)
                {
                    auto [c0,c1,c2] = k;
                    if(0==orient3d(vc0,vc1,vc2,c0,m_vertices.data()) && 0==orient3d(vc0,vc1,vc2,c1,m_vertices.data()) && 0==orient3d(vc0,vc1,vc2,c2,m_vertices.data()))
                    {
                        vcg = v;
                        break;
                    }
                }
                if(UNDEFINED_VALUE != vcg)
                {
                    break;
                }
            }
            if(UNDEFINED_VALUE == vcg)
            {
                uint32_t vc = virtual_constraints_coplanar_group[i][0];
                uint32_t vc0 = virtual_constraints[3*vc+0];
                uint32_t vc1 = virtual_constraints[3*vc+1];
                uint32_t vc2 = virtual_constraints[3*vc+2];
                vcg = coplanar_groups_triangles.size()/3;
                coplanar_groups_triangles.push_back(vc0);
                coplanar_groups_triangles.push_back(vc1);
                coplanar_groups_triangles.push_back(vc2);
            }
            for(uint32_t j=0; j<virtual_constraints_coplanar_group[i].size(); j++)
            {
                uint32_t vc = virtual_constraints_coplanar_group[i][j];
                uint32_t vc0 = virtual_constraints[3*vc+0];
                uint32_t vc1 = virtual_constraints[3*vc+1];
                uint32_t vc2 = virtual_constraints[3*vc+2];
                sort_ints(vc0, vc1, vc2);
                triangles_coplanar_groups[make_tuple(vc0, vc1, vc2)] = vcg;
            }
        }
        
        m_constraints.insert(m_constraints.end(), virtual_constraints.begin(), virtual_constraints.end());
    }

    // for every polyhedron, find the constraints that intersect it
    unordered_map<uint32_t, pair<vector<FacetOrder>, vector<uint32_t>>> polyhedrons_slice_order; // first is polyhedron, second is slice order and coplanar group
    {
        unordered_map<uint32_t, vector<tuple<vector<shared_ptr<genericPoint>>, vector<Segment>, vector<Facet>>>> polyhedrons_intersect_constraints;
        for(uint32_t i=0; i<m_constraints.size()/3; i++)
        {
            uint32_t c0 = m_constraints[3*i+0];
            uint32_t c1 = m_constraints[3*i+1];
            uint32_t c2 = m_constraints[3*i+2];
            if(UNDEFINED_VALUE == c0)
            {
                continue;
            }
            
            unordered_set<uint32_t> visited_tetrahedrons;
            visited_tetrahedrons.insert(UNDEFINED_VALUE);
            queue<uint32_t> visit_tetrahedrons;
            visit_tetrahedrons.push(m_tetrahedralization.get_vertex_incident_tetrahedron(c0));
            visit_tetrahedrons.push(m_tetrahedralization.get_vertex_incident_tetrahedron(c1));
            visit_tetrahedrons.push(m_tetrahedralization.get_vertex_incident_tetrahedron(c2));

            while(!visit_tetrahedrons.empty())
            {
                uint32_t t = visit_tetrahedrons.front();
                visit_tetrahedrons.pop();
                if(visited_tetrahedrons.end() != visited_tetrahedrons.find(t))
                {
                    continue;
                }
                visited_tetrahedrons.insert(t);
                
                uint32_t t0 = m_tetrahedralization.get_tetrahedron_vertex(t,0);
                uint32_t t1 = m_tetrahedralization.get_tetrahedron_vertex(t,1);
                uint32_t t2 = m_tetrahedralization.get_tetrahedron_vertex(t,2);
                uint32_t t3 = m_tetrahedralization.get_tetrahedron_vertex(t,3);
                
                vector<shared_ptr<genericPoint>> vertices;
                vector<Segment> segments;
                vector<Facet> facets;
                auto add_facet = [&](uint32_t p0,uint32_t p1,uint32_t p2,uint32_t p3) // p3 is the oppsite vertex of a tetrahedron facet
                {
                    uint32_t vn = vertices.size();
                    auto [cp0, cp1, cp2] = get_coplanar_group_triangle(p0, p1, p2);
                    vertices.push_back(m_vertices[p0]);
                    vertices.push_back(m_vertices[p1]);
                    vertices.push_back(m_vertices[p2]);
                    if(UNDEFINED_VALUE != p3 && -1 == orient3d(cp0,cp1,cp2,p3,m_vertices.data()))
                    {
                        vertices.push_back(m_vertices[cp0]);
                        vertices.push_back(m_vertices[cp2]);
                        vertices.push_back(m_vertices[cp1]);
                    }
                    else
                    {
                        vertices.push_back(m_vertices[cp0]);
                        vertices.push_back(m_vertices[cp1]);
                        vertices.push_back(m_vertices[cp2]);
                    }
                    
                    uint32_t sn = segments.size();
                    segments.push_back(Segment(vn+0,vn+1));
                    segments.push_back(Segment(vn+1,vn+2));
                    segments.push_back(Segment(vn+2,vn+0));
                    
                    facets.push_back(Facet(sn+0,sn+1,sn+2,vn+3,vn+4,vn+5,get_coplanar_group(p0, p1, p2)));
                };
                add_facet(c0,c1,c2,UNDEFINED_VALUE);
                add_facet(t0,t1,t2,t3);
                add_facet(t1,t0,t3,t2);
                add_facet(t0,t2,t3,t1);
                add_facet(t2,t1,t3,t0);

                int int_type = triangle_tetrahedron_intersection(vertices, segments, facets);
                if(0 == int_type)
                {
                    continue;
                }
                if(2 == int_type)
                {
                    polyhedrons_intersect_constraints[t].push_back(make_tuple(vertices, segments, facets));
                }
                
                for(uint32_t f=0; f<4; f++)
                {
                    visit_tetrahedrons.push(m_tetrahedralization.get_tetrahedron_neighbor(t, f));
                }
            }
        }
        
        for(auto& [p, slices] : polyhedrons_intersect_constraints)
        {
            vector<shared_ptr<genericPoint>> all_vertices;
            vector<Segment> all_segments;
            vector<Facet> all_facets;
            for(auto& [vertices, segments, facets] : slices)
            {
                uint32_t vn = all_vertices.size();
                uint32_t sn = all_segments.size();
                
                all_vertices.insert(all_vertices.end(), vertices.begin(), vertices.end());
                for(uint32_t i=0; i<segments.size(); i++)
                {
                    segments[i].increase_vertices_indexes(vn);
                }
                all_segments.insert(all_segments.end(), segments.begin(), segments.end());
                facets[0].increase_segments_indexes(sn);
                facets[0].p0 += vn;
                facets[0].p1 += vn;
                facets[0].p2 += vn;
                all_facets.push_back(facets[0]);
            }
            
            vector<FacetOrder> facets_order = order_facets(all_vertices,all_segments,all_facets);
            vector<uint32_t> facets_coplanar_groups;
            for(uint32_t i=0; i<all_facets.size(); i++)
            {
                facets_coplanar_groups.push_back(all_facets[i].ip0);
            }
            polyhedrons_slice_order[p] = make_pair(facets_order, facets_coplanar_groups);
        }
    }
    
    // slice polyhedrons
    for(auto& [k, v] : polyhedrons_slice_order)
    {
        auto& [facets_order, facets_coplanar_groups] = v;
        queue<pair<uint32_t, uint32_t>> slice_order; // polyhedron, facets_order index
        slice_order.push(make_pair(k, 0));
        
        while(!slice_order.empty())
        {
            auto [p, o] = slice_order.front();
            slice_order.pop();
            if(UNDEFINED_VALUE == o)
            {
                continue;
            }
            FacetOrder facet_order = facets_order[o];
            
            uint32_t cg = facets_coplanar_groups[facet_order.f];
            uint32_t c0 = coplanar_groups_triangles[3*cg+0];
            uint32_t c1 = coplanar_groups_triangles[3*cg+1];
            uint32_t c2 = coplanar_groups_triangles[3*cg+2];
            
            int slice_res = m_polyhedralization.slice_polyhedron_with_plane(p, c0, c1, c2);
            if(1 == slice_res)
            {
                slice_order.push(make_pair(p, facet_order.top));
            }
            else if(-1 == slice_res)
            {
                slice_order.push(make_pair(p, facet_order.bot));
            }
            else
            {
                slice_order.push(make_pair(p, facet_order.top));
                slice_order.push(make_pair(m_polyhedralization.m_polyhedrons.size()-1, facet_order.bot));
            }
        }
    }
    
    m_polyhedralization.calculate_facets_centroids();
    
    for(auto [k,v] : triangles_coplanar_groups)
    {
        auto [p0,p1,p2] = k;
        while(m_coplanar_triangles.size() <= v)
        {
            m_coplanar_triangles.push_back(vector<uint32_t>());
        }
        m_coplanar_triangles[v].push_back(p0);
        m_coplanar_triangles[v].push_back(p1);
        m_coplanar_triangles[v].push_back(p2);
    }
}
