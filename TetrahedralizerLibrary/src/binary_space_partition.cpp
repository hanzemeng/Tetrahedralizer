#include "binary_space_partition.hpp"
using namespace std;

void BinarySpacePartitionHandle::Dispose()
{}
void BinarySpacePartitionHandle::AddInput(uint32_t explicit_count, double* explicit_values, uint32_t tetrahedrons_count, uint32_t* tetrahedrons, uint32_t constraints_count, uint32_t* constraints)
{
//    m_polyhedralization.m_vertices = create_vertices(explicit_count, explicit_values, 0, nullptr);
//    approximate_verteices(m_approximated_vertices, m_polyhedralization.m_vertices);
//    m_tetrahedralization.assign_tetrahedrons(tetrahedrons, tetrahedrons_count);
//    m_constraints = create_constraints(constraints_count, constraints, m_polyhedralization.m_vertices.data(), true);
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

extern "C" LIBRARY_EXPORT void AddBinarySpacePartitionInput(void* handle, uint32_t explicit_count, double* explicit_values, uint32_t tetrahedron_count, uint32_t* tetrahedrons, uint32_t constraints_count, uint32_t* constraints)
{
    ((BinarySpacePartitionHandle*)handle)->AddInput(explicit_count, explicit_values, tetrahedron_count, tetrahedrons, constraints_count, constraints);
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
    unordered_map<tuple<uint32_t, uint32_t, uint32_t>, uint32_t, iii32_hash> triangles_coplanar_groups;
    vector<uint32_t> coplanar_groups_triangles;
    auto get_coplanar_group_triangle = [&](uint32_t p0,uint32_t p1,uint32_t p2) -> tuple<uint32_t, uint32_t, uint32_t>
    {
        uint32_t c = search_int(p0,p1,p2,triangles_coplanar_groups);
        return make_tuple(coplanar_groups_triangles[3*c+0],coplanar_groups_triangles[3*c+1],coplanar_groups_triangles[3*c+2]);
    };
    
    // create virtual constraints and coplanar groups
    {
        // calculate coplanar groups for tetrahedralization and constraints facets
        vector<uint32_t> triangles = m_tetrahedralization.get_all_facets();
        triangles.insert(triangles.end(),m_constraints.begin(),m_constraints.end());
        auto [coplanar_groups, coplanar_planes] = group_coplanar_triangles(triangles, m_polyhedralization.m_vertices, m_approximated_vertices);
        for(uint32_t i=0; i<coplanar_groups.size(); i++)
        {
            double max_demon = -1.0;
            uint32_t cg = coplanar_groups_triangles.size()/3;
            uint32_t c0,c1,c2;
            for(uint32_t j=0; j<coplanar_groups[i].size(); j++)
            {
                uint32_t nc0 = triangles[3*coplanar_groups[i][j]+0];
                uint32_t nc1 = triangles[3*coplanar_groups[i][j]+1];
                uint32_t nc2 = triangles[3*coplanar_groups[i][j]+2];
                double demon = fabs(barycentric_weight_denom(m_approximated_vertices[nc0],m_approximated_vertices[nc1],m_approximated_vertices[nc2]));
                if(demon > max_demon) // search for the largest triangle to define the plane
                {
                    max_demon = demon;
                    c0 = nc0;
                    c1 = nc1;
                    c2 = nc2;
                }
                assign_int(nc0,nc1,nc2,cg,triangles_coplanar_groups);
            }
            coplanar_groups_triangles.push_back(c0);
            coplanar_groups_triangles.push_back(c1);
            coplanar_groups_triangles.push_back(c2);
        }

        // create virtual constraints
        vector<uint32_t> virtual_constraints;
        auto add_virtual_constraint = [&](uint32_t s0, uint32_t s1, uint32_t c) // e0 and e1 incident the constraint c
        {
            uint32_t t = m_tetrahedralization.get_incident_tetrahedron(s0);
            uint32_t c0 = m_constraints[3*c+0];
            uint32_t c1 = m_constraints[3*c+1];
            uint32_t c2 = m_constraints[3*c+2];
            auto [cg0,cg1,cg2] = get_coplanar_group_triangle(c0, c1, c2);
            for(uint64_t i=0; i<4; i++)
            {
                uint32_t v = m_tetrahedralization.get_tetrahedron_vertex(t, i);
                if(c0 == v || c1 == v || c2 == v || 0 == orient3d(cg0,cg1,cg2,v,m_polyhedralization.m_vertices.data()))
                {
                    continue;
                }
                if(UNDEFINED_VALUE != search_int(s0,s1,v,triangles_coplanar_groups)) // the virtual constraints is already a constraint or tetrahedron facet
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
        
        unordered_map<pair<uint32_t,uint32_t>, vector<uint32_t>, ii32_hash> segments_incident_constraints;
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
            uint32_t cg = search_int(c0, c1, c2, triangles_coplanar_groups);
            bool constrains_are_coplanar = true;
            for(uint32_t i=1; i<constraints.size(); i++)
            {
                uint32_t nc = constraints[i];
                uint32_t nc0 = m_constraints[3*nc+0];
                uint32_t nc1 = m_constraints[3*nc+1];
                uint32_t nc2 = m_constraints[3*nc+2];
                uint32_t ncg = search_int(nc0, nc1, nc2, triangles_coplanar_groups);
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
            int ignore_axis = max_component_in_triangle_normal(cc0,cc1,cc2, m_polyhedralization.m_vertices.data());
            int os2 = orient3d_ignore_axis(s0,s1,s2,ignore_axis, m_polyhedralization.m_vertices.data());
            bool all_vertices_on_same_side = true;
            for(uint32_t s3 : vertices)
            {
                int os3 = orient3d_ignore_axis(s0,s1,s3,ignore_axis, m_polyhedralization.m_vertices.data());
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
        auto [vc_coplanar_groups, vc_coplanar_planes] = group_coplanar_triangles(virtual_constraints, m_polyhedralization.m_vertices, m_approximated_vertices);
        for(uint32_t i=0; i<vc_coplanar_groups.size(); i++)
        {
            uint32_t vcg = UNDEFINED_VALUE;
            for(uint32_t j=0; j<coplanar_groups.size(); j++)
            {
                if(vc_coplanar_planes[4*i+0] != coplanar_planes[4*j+0] ||
                   vc_coplanar_planes[4*i+1] != coplanar_planes[4*j+1] ||
                   vc_coplanar_planes[4*i+2] != coplanar_planes[4*j+2] ||
                   vc_coplanar_planes[4*i+3] != coplanar_planes[4*j+3])
                {
                    continue;
                }
                uint32_t vc = vc_coplanar_groups[i][0];
                uint32_t vc0 = virtual_constraints[3*vc+0];
                uint32_t vc1 = virtual_constraints[3*vc+1];
                uint32_t vc2 = virtual_constraints[3*vc+2];
                uint32_t c = coplanar_groups[j][0];
                uint32_t c0 = triangles[3*c+0];
                uint32_t c1 = triangles[3*c+1];
                uint32_t c2 = triangles[3*c+2];
                if(0==orient3d(vc0,vc1,vc2,c0,m_polyhedralization.m_vertices.data()) && 0==orient3d(vc0,vc1,vc2,c1,m_polyhedralization.m_vertices.data()) && 0==orient3d(vc0,vc1,vc2,c2,m_polyhedralization.m_vertices.data()))
                {
                    vcg = search_int(c0, c1, c2, triangles_coplanar_groups);
                    break;
                }
            }
            
            if(UNDEFINED_VALUE == vcg)
            {
                uint32_t vc = vc_coplanar_groups[i][0];
                uint32_t vc0 = virtual_constraints[3*vc+0];
                uint32_t vc1 = virtual_constraints[3*vc+1];
                uint32_t vc2 = virtual_constraints[3*vc+2];
                vcg = coplanar_groups_triangles.size()/3;
                coplanar_groups_triangles.push_back(vc0);
                coplanar_groups_triangles.push_back(vc1);
                coplanar_groups_triangles.push_back(vc2);
            }
            for(uint32_t vc : vc_coplanar_groups[i])
            {
                uint32_t vc0 = virtual_constraints[3*vc+0];
                uint32_t vc1 = virtual_constraints[3*vc+1];
                uint32_t vc2 = virtual_constraints[3*vc+2];
                assign_int(vc0, vc1, vc2, vcg, triangles_coplanar_groups);
            }
        }
        
        m_constraints.insert(m_constraints.end(), virtual_constraints.begin(), virtual_constraints.end());
    }
    
    // convert tetrahedrons to polyhedrons
    {
        unordered_map<pair<uint32_t, uint32_t>, uint32_t, ii32_hash> segments_cache;
        unordered_map<tuple<uint32_t, uint32_t, uint32_t>, uint32_t, iii32_hash> facets_cache;
        for(uint32_t i=0; i<m_tetrahedralization.get_tetrahedrons_count(); i++)
        {
            uint32_t p = m_polyhedralization.m_polyhedrons.size();
            m_polyhedralization.m_polyhedrons.push_back(vector<uint32_t>());
            
            for(uint32_t j=0; j<4; j++)
            {
                auto find_segment = [&](uint32_t p0, uint32_t p1) -> uint32_t
                {
                    uint32_t s = search_int(p0,p1,segments_cache);
                    if(UNDEFINED_VALUE == s)
                    {
                        s = m_polyhedralization.m_segments.size();
                        m_polyhedralization.m_segments.push_back(Segment(p0,p1));
                        assign_int(p0, p1, s, segments_cache);
                    }
                    return s;
                };
                
                auto [p0,p1,p2] = m_tetrahedralization.get_tetrahedron_facet(i,j);
                uint32_t s0 = find_segment(p0, p1);
                uint32_t s1 = find_segment(p1, p2);
                uint32_t s2 = find_segment(p2, p0);
                
                uint32_t f = search_int(p0,p1,p2,facets_cache);
                if(UNDEFINED_VALUE == f)
                {
                    auto [cp0,cp1,cp2] = get_coplanar_group_triangle(p0,p1,p2);
                    f = m_polyhedralization.m_facets.size();
                    m_polyhedralization.m_facets.push_back(Facet(s0,s1,s2,cp0,cp1,cp2,UNDEFINED_VALUE,UNDEFINED_VALUE));
                    assign_int(p0, p1, p2, f, facets_cache);
                }
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

    // for every polyhedron, find the constraints that intersect it
    unordered_map<uint32_t, vector<uint32_t>> polyhedrons_slice_order; // first is polyhedron, second is slice order and coplanar group
    {
        unordered_map<uint32_t, tuple<vector<shared_ptr<genericPoint>>, vector<Segment>, vector<Facet>>> polyhedrons_intersect_constraints;
        vector<uint32_t> visited_tetrahedrons = vector<uint32_t>(m_tetrahedralization.get_tetrahedrons_count(), UNDEFINED_VALUE);
        queue<uint32_t> visit_tetrahedrons;
        for(uint32_t i=0; i<m_constraints.size()/3; i++)
        {
            uint32_t c0 = m_constraints[3*i+0];
            uint32_t c1 = m_constraints[3*i+1];
            uint32_t c2 = m_constraints[3*i+2];
            if(UNDEFINED_VALUE == c0 || UNDEFINED_VALUE != m_tetrahedralization.get_incident_tetrahedron(c0, c1, c2))
            {
                continue;
            }
            auto [cc0, cc1, cc2] = get_coplanar_group_triangle(c0, c1, c2);
            uint32_t cg = search_int(c0, c1, c2, triangles_coplanar_groups);
            
            visit_tetrahedrons.push(m_tetrahedralization.get_incident_tetrahedron(c0));
            visit_tetrahedrons.push(m_tetrahedralization.get_incident_tetrahedron(c1));
            visit_tetrahedrons.push(m_tetrahedralization.get_incident_tetrahedron(c2));
            while(!visit_tetrahedrons.empty())
            {
                uint32_t t = visit_tetrahedrons.front();
                visit_tetrahedrons.pop();
                if(UNDEFINED_VALUE==t || i==visited_tetrahedrons[t])
                {
                    continue;
                }
                visited_tetrahedrons[t] = i;

                auto [t0,t1,t2,t3] = m_tetrahedralization.get_tetrahedron_vertices(t);
                vector<Segment> segments = {Segment(c0,c1),Segment(c1,c2),Segment(c2,c0)};
                vector<uint32_t> planes;
                vector<uint32_t> planes_groups;
                planes.push_back(cc0);
                planes.push_back(cc1);
                planes.push_back(cc2);
                planes_groups.push_back(cg);
                auto add_facet = [&](uint32_t p0,uint32_t p1,uint32_t p2,uint32_t p3) // p3 is the oppsite vertex of a tetrahedron facet
                {
                    auto [cp0, cp1, cp2] = get_coplanar_group_triangle(p0, p1, p2);
                    if(-1 == orient3d(cp0,cp1,cp2,p3,m_polyhedralization.m_vertices.data()))
                    {
                        planes.push_back(cp0);
                        planes.push_back(cp2);
                        planes.push_back(cp1);
                    }
                    else
                    {
                        planes.push_back(cp0);
                        planes.push_back(cp1);
                        planes.push_back(cp2);
                    }
                    planes_groups.push_back(search_int(p0, p1, p2, triangles_coplanar_groups));
                };
                add_facet(t0,t1,t2,t3);
                add_facet(t1,t0,t3,t2);
                add_facet(t0,t2,t3,t1);
                add_facet(t2,t1,t3,t0);
                
                uint32_t original_vertices_count = m_polyhedralization.m_vertices.size();
                auto [int_type, top_segments] = triangle_tetrahedron_intersection(m_polyhedralization.m_vertices, segments, planes, planes_groups);

                if(2 == int_type)
                {
                    auto& [all_vertices, all_segments, all_facets] = polyhedrons_intersect_constraints[t];
                    uint32_t vn = all_vertices.size();
                    uint32_t sn = all_segments.size();
                    all_vertices.insert(all_vertices.end(), m_polyhedralization.m_vertices.begin()+original_vertices_count, m_polyhedralization.m_vertices.end());
                    for(uint32_t s : top_segments)
                    {
                        segments[s].increase_vertices_indexes(original_vertices_count, vn);
                        all_segments.push_back(segments[s]);
                    }
                    all_facets.push_back(Facet(sn, top_segments.size(), cc0,cc1,cc2,cg));
                }
                
                m_polyhedralization.m_vertices.resize(original_vertices_count);
                if(0 == int_type)
                {
                    continue;
                }
                for(uint32_t f=0; f<4; f++)
                {
                    visit_tetrahedrons.push(m_tetrahedralization.get_tetrahedron_neighbor(t, f).first);
                }
            }
        }
        for(auto& [p, v] : polyhedrons_intersect_constraints)
        {
            auto& [all_vertices, all_segments, all_facets] = v;
            uint32_t original_vertices_count = m_polyhedralization.m_vertices.size();
            m_polyhedralization.m_vertices.insert(m_polyhedralization.m_vertices.end(), all_vertices.begin(), all_vertices.end());
            approximate_verteices(m_approximated_vertices, m_polyhedralization.m_vertices);
            polyhedrons_slice_order[p] = order_facets(m_polyhedralization.m_vertices,m_approximated_vertices, all_segments,all_facets);
            m_polyhedralization.m_vertices.resize(original_vertices_count);
            m_approximated_vertices.resize(original_vertices_count);
        }
    }
    
    // slice polyhedrons
    for(auto& [p, facets_order] : polyhedrons_slice_order)
    {
        queue<pair<uint32_t, uint32_t>> slice_order; // polyhedron, facets_order index
        slice_order.push(make_pair(p, 0));
        
        while(!slice_order.empty())
        {
            auto [p, o] = slice_order.front();
            slice_order.pop();
            if(UNDEFINED_VALUE == o)
            {
                continue;
            }
            uint32_t cg = facets_order[o];
            uint32_t c0 = coplanar_groups_triangles[3*cg+0];
            uint32_t c1 = coplanar_groups_triangles[3*cg+1];
            uint32_t c2 = coplanar_groups_triangles[3*cg+2];
            
            int slice_res = m_polyhedralization.slice_polyhedron_with_plane(p, c0, c1, c2);
            if(1 == slice_res)
            {
                slice_order.push(make_pair(p, facets_order[o+1]));
            }
            else if(-1 == slice_res)
            {
                slice_order.push(make_pair(p, facets_order[o+2]));
            }
            else
            {
                slice_order.push(make_pair(p, facets_order[o+1]));
                slice_order.push(make_pair(m_polyhedralization.m_polyhedrons.size()-1, facets_order[o+2]));
            }
        }
    }

    approximate_verteices(m_approximated_vertices, m_polyhedralization.m_vertices);
    for(uint32_t i=0; i<m_polyhedralization.m_facets.size(); i++)
    {
        m_polyhedralization.m_facets[i].calculate_implicit_centroid(m_approximated_vertices, m_polyhedralization.m_segments);
    }
    m_coplanar_triangles = vector<vector<uint32_t>>(coplanar_groups_triangles.size()/3);
    for(uint32_t i=0; i<coplanar_groups_triangles.size()/3; i++)
    {
        m_coplanar_triangles[i].push_back(coplanar_groups_triangles[3*i+0]);
        m_coplanar_triangles[i].push_back(coplanar_groups_triangles[3*i+1]);
        m_coplanar_triangles[i].push_back(coplanar_groups_triangles[3*i+2]);
    }
    for(auto [k,v] : triangles_coplanar_groups)
    {
        auto [p0,p1,p2] = k;
        if((p0==m_coplanar_triangles[v][0] || p1==m_coplanar_triangles[v][0] || p2==m_coplanar_triangles[v][0]) &&
           (p0==m_coplanar_triangles[v][1] || p1==m_coplanar_triangles[v][1] || p2==m_coplanar_triangles[v][1]) &&
           (p0==m_coplanar_triangles[v][2] || p1==m_coplanar_triangles[v][2] || p2==m_coplanar_triangles[v][2]))
        {
            continue;
        }
        m_coplanar_triangles[v].push_back(p0);
        m_coplanar_triangles[v].push_back(p1);
        m_coplanar_triangles[v].push_back(p2);
    }
}
