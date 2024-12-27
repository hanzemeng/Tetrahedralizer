#include "binary_space_partition.h"

void binary_space_partition()
{
    // convert tetrahedrons to polyhedrons
    {
        m_polyhedrons.clear();
        m_polyhedrons_facets.clear();
        m_polyhedrons_edges.clear();
        m_vertices_incidents.resize(m_vertices_count);
        m_tetrahedrons_polyhedrons_mapping.resize(m_tetrahedrons.size()/4);

        m_map_ii_i_0.clear();
        m_map_iii_i_0.clear();
        
        for(uint32_t i=0; i<m_tetrahedrons.size(); i+=4)
        {
            if(UNDEFINED_VALUE == m_tetrahedrons[i] || INFINITE_VERTEX == m_tetrahedrons[i+3])
            {
                m_tetrahedrons_polyhedrons_mapping[i/4] = UNDEFINED_VALUE;
                continue;
            }

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
                get_tetrahedron_face(i,f,p0,p1,p2);
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
    
    m_virtual_constraints_count = 0;
    // create virtual constraints
    {
        m_map_ii_vector_i_0.clear(); // key is an edge, value is the edge's incident constraints
        for(uint32_t i=0; i<3*m_constraints_count; i+=3)
        {
            uint32_t c0 = m_constraints[i+0];
            uint32_t c1 = m_constraints[i+1];
            uint32_t c2 = m_constraints[i+2];

            uint32_t t0 = c0;
            uint32_t t1 = c1;
            sort_ints(t0,t1);
            m_map_ii_vector_i_0[{t0,t1}].push_back(i);
            t0 = c1;
            t1 = c2;
            sort_ints(t0,t1);
            m_map_ii_vector_i_0[{t0,t1}].push_back(i);
            t0 = c2;
            t1 = c0;
            sort_ints(t0,t1);
            m_map_ii_vector_i_0[{t0,t1}].push_back(i);
        }
        for(auto& it : m_map_ii_vector_i_0)
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
                if(0 != orient3d(e3,e0,e1,e2))
                {
                    is_on_boundary = false;
                    break;
                }
            }
            if(!is_on_boundary)
            {
                continue;
            }

            int ignore_axis = max_component_in_triangle_normal(e0,e1,e2);
            int oe2 = orient3d_ignore_axis(e0,e1,e2,ignore_axis);
            for(uint32_t e3 : m_u_set_i_0)
            {
                if(oe2 != orient3d_ignore_axis(e0,e1,e3,ignore_axis))
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
        for(uint32_t i=0; i<3*(m_constraints_count+m_virtual_constraints_count); i+=3)
        {
            uint32_t c0 = m_constraints[i+0];
            uint32_t c1 = m_constraints[i+1];
            uint32_t c2 = m_constraints[i+2];

            // m_u_set_i_0 stores the polyhedrons that incident at one of the point of the ith constraint
            // m_u_set_i_1 stores visited polyhedrons
            // m_queue_i_0 stores tetrahedrons to be visited
            m_u_set_i_0.clear();
            m_u_set_i_1.clear();
            m_u_set_i_1.insert(UNDEFINED_VALUE);
            clear_queue(m_queue_i_0);
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

                if(INFINITE_VERTEX == m_tetrahedrons[t+3])
                {
                    continue;
                }
                if(!has_vertex(t,c0) && !has_vertex(t,c1) && !has_vertex(t,c2))
                {
                    continue;
                }
                m_u_set_i_0.insert(t);
                uint32_t n;
                for(uint32_t f=0; f<4; f++)
                {
                    get_tetrahedron_neighbor(t, f, n);
                    m_queue_i_0.push(n);
                }
            }

            
            // m_u_set_i_1 stores visited polyhedrons
            // m_queue_i_0 stores polyhedrons to be visited
            m_u_set_i_1.clear();
            m_u_set_i_1.insert(UNDEFINED_VALUE);
            for(uint32_t t : m_u_set_i_0)
            {
                m_queue_i_0.push(t);
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

                if(INFINITE_VERTEX == m_tetrahedrons[t+3])
                {
                    continue;
                }

                for(uint32_t f=0; f<4; f++)
                {
                    uint32_t f0,f1,f2;
                    uint32_t o0;
                    get_tetrahedron_face(t,f,f0,f1,f2);
                    if(edge_intersects_triangle(c0,c1,f0,f1,f2,o0) || edge_intersects_triangle(c1,c2,f0,f1,f2,o0) || edge_intersects_triangle(c2,c0,f0,f1,f2,o0))
                    {
                        uint32_t n;
                        get_tetrahedron_neighbor(t, f, n);
                        m_u_set_i_0.insert(n);
                        m_queue_i_0.push(n);
                    }

                    if(edge_intersects_triangle(f0,f1,c0,c1,c2,o0))
                    {
                        m_u_set_i_0.insert(t);
                        if(UNDEFINED_VALUE == o0)
                        {
                            add_tetrahedron_incident(f0,f1,m_u_set_i_2,m_queue_i_1,m_u_set_i_0, m_queue_i_0);
                        }
                        else
                        {
                            add_tetrahedron_incident(o0,m_u_set_i_2,m_queue_i_1,m_u_set_i_0,m_queue_i_0);
                        }
                    }
                    if(edge_intersects_triangle(f1,f2,c0,c1,c2,o0))
                    {
                        m_u_set_i_0.insert(t);
                        if(UNDEFINED_VALUE == o0)
                        {
                            add_tetrahedron_incident(f1,f2,m_u_set_i_2,m_queue_i_1,m_u_set_i_0,m_queue_i_0);
                        }
                        else
                        {
                            add_tetrahedron_incident(o0,m_u_set_i_2,m_queue_i_1,m_u_set_i_0,m_queue_i_0);
                        }
                    }
                    if(edge_intersects_triangle(f2,f0,c0,c1,c2,o0))
                    {
                        m_u_set_i_0.insert(t);
                        if(UNDEFINED_VALUE == o0)
                        {
                            add_tetrahedron_incident(f2,f0,m_u_set_i_2,m_queue_i_1,m_u_set_i_0,m_queue_i_0);
                        }
                        else
                        {
                            add_tetrahedron_incident(o0,m_u_set_i_2,m_queue_i_1,m_u_set_i_0,m_queue_i_0);
                        }
                    }
                }
            }
            
            // if the constraint improperly intersects a tetrahedron, they are associated
            for(uint32_t t : m_u_set_i_0)
            {
                uint32_t t0 = m_tetrahedrons[t+0];
                uint32_t t1 = m_tetrahedrons[t+1];
                uint32_t t2 = m_tetrahedrons[t+2];
                uint32_t t3 = m_tetrahedrons[t+3];
                int ot0c012 = orient3d(t0,c0,c1,c2);
                int ot1c012 = orient3d(t1,c0,c1,c2);
                int ot2c012 = orient3d(t2,c0,c1,c2);
                int ot3c012 = orient3d(t3,c0,c1,c2);

                uint32_t zero_count;
                improper_intersection_helper_0(t0,t1,t2,t3,ot0c012,ot1c012,ot2c012,ot3c012,zero_count);
                if(4 == zero_count) // should not happen
                {
                    continue;
                }
                
                if(3 == zero_count)
                {
                    continue;
                }
                
                if(2 == zero_count)
                {
                    if(ot2c012 == ot3c012)
                    {
                        continue;
                    }
                    if(vertex_in_inner_triangle(t0,c0,c1,c2) || vertex_in_inner_triangle(t1,c0,c1,c2))
                    {
                        m_polyhedrons_intersect_constraints[m_tetrahedrons_polyhedrons_mapping[t/4]].push_back(i);
                        continue;
                    }

                    if(vertex_in_triangle(t0,c0,c1,c2) && vertex_in_triangle(t1,c0,c1,c2))
                    {
                        if( (vertex_in_segment(t0,c0,c1) && vertex_in_segment(t1,c0,c1)) ||
                            (vertex_in_segment(t0,c1,c2) && vertex_in_segment(t1,c1,c2)) ||
                            (vertex_in_segment(t0,c2,c0) && vertex_in_segment(t1,c2,c0)))
                        {
                            if(inner_segment_cross_triangle(t2,t3,c0,c1,c2))
                            {
                                m_polyhedrons_intersect_constraints[m_tetrahedrons_polyhedrons_mapping[t/4]].push_back(i);
                                continue;
                            }
                            else
                            {
                                if( inner_segment_cross_inner_triangle(c0,c1,t2,t3,t0) || 
                                    inner_segment_cross_inner_triangle(c1,c2,t2,t3,t0) ||
                                    inner_segment_cross_inner_triangle(c2,c0,t2,t3,t0) ||
                                    inner_segment_cross_inner_triangle(c0,c1,t2,t3,t1) ||
                                    inner_segment_cross_inner_triangle(c1,c2,t2,t3,t1) ||
                                    inner_segment_cross_inner_triangle(c2,c0,t2,t3,t1))
                                {
                                    m_polyhedrons_intersect_constraints[m_tetrahedrons_polyhedrons_mapping[t/4]].push_back(i);
                                }
                                else
                                {
                                    continue;
                                }
                            }
                        }
                        else
                        {
                            m_polyhedrons_intersect_constraints[m_tetrahedrons_polyhedrons_mapping[t/4]].push_back(i);
                            continue;
                        }
                    }
                    else if(vertex_in_triangle(t0,c0,c1,c2) || vertex_in_triangle(t1,c0,c1,c2))
                    {
                        if( inner_segment_cross_inner_segment(t0,t1,c0,c1) ||
                            inner_segment_cross_inner_segment(t0,t1,c1,c2) ||
                            inner_segment_cross_inner_segment(t0,t1,c2,c0))
                        {
                            m_polyhedrons_intersect_constraints[m_tetrahedrons_polyhedrons_mapping[t/4]].push_back(i);
                            continue;
                        }
                        if(inner_segment_cross_triangle(t2,t3,c0,c1,c2))
                        {
                            m_polyhedrons_intersect_constraints[m_tetrahedrons_polyhedrons_mapping[t/4]].push_back(i);
                            continue;
                        }
                        if( inner_segment_cross_inner_triangle(c0,c1,t2,t3,t0) ||
                            inner_segment_cross_inner_triangle(c1,c2,t2,t3,t0) ||
                            inner_segment_cross_inner_triangle(c2,c0,t2,t3,t0) ||
                            inner_segment_cross_inner_triangle(c0,c1,t2,t3,t1) ||
                            inner_segment_cross_inner_triangle(c1,c2,t2,t3,t1) ||
                            inner_segment_cross_inner_triangle(c2,c0,t2,t3,t1))
                        {
                            m_polyhedrons_intersect_constraints[m_tetrahedrons_polyhedrons_mapping[t/4]].push_back(i);
                            continue;
                        }
                    }
                    else
                    {
                        m_polyhedrons_intersect_constraints[m_tetrahedrons_polyhedrons_mapping[t/4]].push_back(i);
                        continue;
                    }
                }
                else if(1 == zero_count)
                {
                    if(ot1c012 == ot2c012 && ot2c012 == ot3c012)
                    {
                        continue;
                    }
                    if(!vertex_in_triangle(t0,c0,c1,c2))
                    {
                        m_polyhedrons_intersect_constraints[m_tetrahedrons_polyhedrons_mapping[t/4]].push_back(i);
                        continue;
                    }
                    if(vertex_in_inner_triangle(t0,c0,c1,c2))
                    {
                        m_polyhedrons_intersect_constraints[m_tetrahedrons_polyhedrons_mapping[t/4]].push_back(i);
                        continue;
                    }

                    uint32_t td, te1, te2;
                    if(ot2c012 == ot3c012 && ot1c012 != ot2c012)
                    {
                        td = t1;
                        te1 = t2;
                        te2 = t3;
                    }
                    else if(ot1c012 == ot3c012 && ot2c012 != ot1c012)
                    {
                        td = t2;
                        te1 = t1;
                        te2 = t3;
                    }
                    else if(ot1c012 == ot2c012 && ot3c012 != ot1c012)
                    {
                        td = t3;
                        te1 = t1;
                        te2 = t2;
                    }
                    else
                    {
                        throw "the paper does not account for this case";
                    }
                    if( inner_segment_cross_triangle(td,te1,c0,c1,c2) ||
                        inner_segment_cross_triangle(td,te2,c0,c1,c2) ||
                        inner_segment_cross_inner_triangle(c0,c1,t0,td,te1) ||
                        inner_segment_cross_inner_triangle(c1,c2,t0,td,te1) ||
                        inner_segment_cross_inner_triangle(c2,c0,t0,td,te1) ||
                        inner_segment_cross_inner_triangle(c0,c1,t0,td,te2) ||
                        inner_segment_cross_inner_triangle(c1,c2,t0,td,te2) ||
                        inner_segment_cross_inner_triangle(c2,c0,t0,td,te2) ||
                        inner_segment_cross_inner_triangle(c0,c1,td,te2,te1) ||
                        inner_segment_cross_inner_triangle(c1,c2,td,te2,te1) ||
                        inner_segment_cross_inner_triangle(c2,c0,td,te2,te1))
                    {
                        m_polyhedrons_intersect_constraints[m_tetrahedrons_polyhedrons_mapping[t/4]].push_back(i);
                        continue;
                    }
                }
                else
                {
                    uint32_t s = 0;
                    s += inner_segment_cross_triangle(t0,t1,c0,c1,c2);
                    s += inner_segment_cross_triangle(t0,t2,c0,c1,c2);
                    s += inner_segment_cross_triangle(t0,t3,c0,c1,c2);
                    s += inner_segment_cross_triangle(t1,t2,c0,c1,c2);
                    s += inner_segment_cross_triangle(t1,t3,c0,c1,c2);
                    s += inner_segment_cross_triangle(t2,t3,c0,c1,c2);
                    s += (inner_segment_cross_inner_triangle(c0,c1,t1,t2,t3) || inner_segment_cross_inner_triangle(c1,c2,t1,t2,t3) || inner_segment_cross_inner_triangle(c2,c0,t1,t2,t3));
                    s += (inner_segment_cross_inner_triangle(c0,c1,t0,t2,t3) || inner_segment_cross_inner_triangle(c1,c2,t0,t2,t3) || inner_segment_cross_inner_triangle(c2,c0,t0,t2,t3));
                    s += (inner_segment_cross_inner_triangle(c0,c1,t0,t1,t3) || inner_segment_cross_inner_triangle(c1,c2,t0,t1,t3) || inner_segment_cross_inner_triangle(c2,c0,t0,t1,t3));
                    s += (inner_segment_cross_inner_triangle(c0,c1,t0,t1,t2) || inner_segment_cross_inner_triangle(c1,c2,t0,t1,t2) || inner_segment_cross_inner_triangle(c2,c0,t0,t1,t2));
                    if(s>=3)
                    {
                        m_polyhedrons_intersect_constraints[m_tetrahedrons_polyhedrons_mapping[t/4]].push_back(i);
                        continue;
                    }
                }
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
                m_map_i_iii_0.clear(); // keys are processed edges, values are (intersection vertex, top edge, bottom edge)
                for(uint32_t f : m_polyhedrons[i]->facets)
                {
                    uint32_t i0(UNDEFINED_VALUE), i1(UNDEFINED_VALUE);
                    m_vector_i_2.clear(); // edges on top of the constraint
                    m_vector_i_3.clear(); // edges on bottom of the constraint
                    bool has_edge_on_constraint = false;
                    
                    for(uint32_t e : m_polyhedrons_facets[f]->edges)
                    {
                        if(m_map_i_iii_0.end() != m_map_i_iii_0.find(e))
                        {
                            auto [i_p,top_e,bottom_e] = m_map_i_iii_0[e];
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
                        int o0(orient3d(c0,c1,c2,e0)), o1 = orient3d(c0,c1,c2,e1);
                        if(0 == o0)
                        {
                            if(0 == o1)
                            {
                                m_u_set_i_0.insert(e);
                                has_edge_on_constraint = true;
                                m_map_i_iii_0[e] = {UNDEFINED_VALUE,UNDEFINED_VALUE,UNDEFINED_VALUE};
                            }
                            else
                            {
                                if(1 == o1)
                                {
                                    m_vector_i_2.push_back(e);
                                    m_map_i_iii_0[e] = {e0,e,UNDEFINED_VALUE};
                                }
                                else
                                {
                                    m_vector_i_3.push_back(e);
                                    m_map_i_iii_0[e] = {e0,UNDEFINED_VALUE,e};
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
                                m_map_i_iii_0[e] = {e1,e,UNDEFINED_VALUE};
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
                                m_map_i_iii_0[e] = {UNDEFINED_VALUE,e,UNDEFINED_VALUE};
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
                                
                                m_map_i_iii_0[e] = {new_i,e,bottom_e};
                                
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
                                m_map_i_iii_0[e] = {e1,UNDEFINED_VALUE,e};
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
                                
                                m_map_i_iii_0[e] = {new_i,e,bottom_e};
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
                                m_map_i_iii_0[e] = {UNDEFINED_VALUE,UNDEFINED_VALUE,e};
                            }
                        }
                    }
                    
                    if(has_edge_on_constraint || i1 == UNDEFINED_VALUE) // constraint intersects on one or more edges of the facet or does not intersect the facet
                    {
                        if(!m_vector_i_2.empty())
                        {
                            m_vector_i_0.push_back(f);
                        }
                        else
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
                
                
                for(auto [k, v] : m_map_i_iii_0)
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
                    int o0(orient3d(c0,c1,c2,nc0)), o1(orient3d(c0,c1,c2,nc1)), o2(orient3d(c0,c1,c2,nc2));

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
}

uint32_t find_or_add_edge(uint32_t p0, uint32_t p1)
{
    sort_ints(p0,p1);
    if(m_map_ii_i_0.end() != m_map_ii_i_0.find({p0,p1}))
    {
        return m_map_ii_i_0[{p0,p1}];
    }

    m_map_ii_i_0[{p0,p1}] = m_polyhedrons_edges.size();

    PolyhedronEdge* e = new PolyhedronEdge();
    e->e0 = p0;
    e->e1 = p1;
    e->p0 = p0;
    e->p1 = p1;
    e->p2 = e->p3 = e->p4 = e->p5 = e->f = UNDEFINED_VALUE;
    m_polyhedrons_edges.push_back(e);
    return m_polyhedrons_edges.size()-1;
}
uint32_t find_or_add_facet(uint32_t e0, uint32_t e1, uint32_t e2, uint32_t p0, uint32_t p1,  uint32_t p2)
{
    sort_ints(e0,e1,e2);
    if(m_map_iii_i_0.end() != m_map_iii_i_0.find({e0,e1,e2}))
    {
        return m_map_iii_i_0[{e0,e1,e2}];
    }
    m_map_iii_i_0[{e0,e1,e2}] = m_polyhedrons_facets.size();
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

void add_virtual_constraint(uint32_t e0, uint32_t e1, uint32_t c) // e0 and e1 incident the constraint c
{
    uint32_t t = m_vertices_incidents[e0];
    uint32_t c0 = m_constraints[c+0];
    uint32_t c1 = m_constraints[c+1];
    uint32_t c2 = m_constraints[c+2];
    for(uint64_t i=0; i<4; i++)
    {
        if(c0 == m_tetrahedrons[t+i] || c1 == m_tetrahedrons[t+i] || c2 == m_tetrahedrons[t+i] || 0 == orient3d(m_tetrahedrons[t+i],c0,c1,c2))
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

bool has_vertex(uint32_t t, uint32_t v)
{
    return v == m_tetrahedrons[t+0] || v == m_tetrahedrons[t+1] || v == m_tetrahedrons[t+2] || v == m_tetrahedrons[t+3];
}

bool edge_intersects_triangle(uint32_t e0,uint32_t e1,uint32_t t0,uint32_t t1,uint32_t t2, uint32_t& o0)
{
    int oe01t01 = orient3d(e0,e1,t0,t1);
    int oe01t12 = orient3d(e0,e1,t1,t2);
    int oe01t20 = orient3d(e0,e1,t2,t0);
    if(oe01t01 != oe01t12 || oe01t12 != oe01t20)
    {
        return false;
    }
    int oe0t012 = orient3d(e0,t0,t1,t2);
    int oe1t012 = orient3d(e1,t0,t1,t2);
    o0 = UNDEFINED_VALUE;
    if(0 == oe0t012)
    {
        o0 = e0;
    }
    if(0 == oe1t012)
    {
        o0 = e1;
    }
    return oe0t012 != oe1t012;
}

void add_tetrahedron_incident(uint32_t p0, unordered_set<uint32_t>& visited, queue<uint32_t>& to_be_visited, unordered_set<uint32_t>& res_set, queue<uint32_t>& res_queue)
{
    visited.clear();
    visited.insert(UNDEFINED_VALUE);
    clear_queue(to_be_visited);
    to_be_visited.push(m_vertices_incidents[p0]);
    while(!to_be_visited.empty())
    {
        uint32_t t = to_be_visited.front();
        to_be_visited.pop();
        if(visited.end() != visited.find(t))
        {
            continue;
        }
        visited.insert(t);

        if(INFINITE_VERTEX == m_tetrahedrons[t+3])
        {
            continue;
        }
        if(!has_vertex(t,p0))
        {
            continue;
        }
        res_set.insert(t);
        res_queue.push(t);
        uint32_t n;
        for(uint32_t f=0; f<4; f++)
        {
            get_tetrahedron_neighbor(t, f, n);
            to_be_visited.push(n);
        }
    }
}
void add_tetrahedron_incident(uint32_t p0,uint32_t p1, unordered_set<uint32_t>& visited, queue<uint32_t>& to_be_visited, unordered_set<uint32_t>& res_set, queue<uint32_t>& res_queue)
{
    visited.clear();
    visited.insert(UNDEFINED_VALUE);
    clear_queue(to_be_visited);
    to_be_visited.push(m_vertices_incidents[p0]);
    while(!to_be_visited.empty())
    {
        uint32_t t = to_be_visited.front();
        to_be_visited.pop();
        if(visited.end() != visited.find(t))
        {
            continue;
        }
        visited.insert(t);

        if(INFINITE_VERTEX == m_tetrahedrons[t+3])
        {
            continue;
        }
        bool b0 = has_vertex(t,p0);
        bool b1 = has_vertex(t,p1);
        if(!b0 && !b1)
        {
            continue;
        }
        if(b0 && b1)
        {
            res_set.insert(t);
            res_queue.push(t);
        }
        uint32_t n;
        for(uint32_t f=0; f<4; f++)
        {
            get_tetrahedron_neighbor(t, f, n);
            to_be_visited.push(n);
        }
    }
}
    
void get_polyhedron_facet_vertices(uint32_t f, uint32_t& f0,uint32_t& f1,uint32_t& f2)
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

void improper_intersection_helper_0(uint32_t& t0,uint32_t& t1,uint32_t& t2,uint32_t& t3,int& o0,int& o1,int& o2,int& o3, uint32_t& zero_count)
{
    zero_count = 0;
    zero_count += 0 == o0;
    zero_count += 0 == o1;
    zero_count += 0 == o2;
    zero_count += 0 == o3;
    if(0 == zero_count || 4 == zero_count) // 4 == zero_count should not happen
    {
        return;
    }

    if(1 == zero_count) // put the zero orient point as t0; while keeping 1 == orient3d(t0,t1,t2,t3)
    {
        if(0 == o1)
        {
            swap(t0,t1);
            swap(o0,o1);
            swap(t2,t3);
            swap(o2,o3);
        }
        else if(0 == o2)
        {
            swap(t0,t2);
            swap(o0,o2);
            swap(t1,t3);
            swap(o1,o3);
        }
        else if(0 == o3)
        {
            swap(t0,t3);
            swap(o0,o3);
            swap(t1,t2);
            swap(o1,o2);
        }
    }
    else if(2 == zero_count)// put the zero orient points as t0, t1; while keeping 1 == orient3d(t0,t1,t2,t3)
    {
        if(0 == o0)
        {
            if(0 == o2)
            {
                swap(t1,t2);
                swap(o1,o2);
                swap(t2,t3);
                swap(o2,o3);
            }
            else if(0 == o3)
            {
                swap(t1,t3);
                swap(o1,o3);
                swap(t2,t3);
                swap(o2,o3);
            }
        }
        else if(0 == o1)
        {
            if(0 == o2)
            {
                swap(t0,t1);
                swap(o0,o1);
                swap(t1,t2);
                swap(o1,o2);
            }
            else// if(0 == o3)
            {
                swap(t0,t1);
                swap(o0,o1);
                swap(t1,t3);
                swap(o1,o3);
            }
        }
        else// if(0 == o2)
        {
            swap(t0,t2);
            swap(o0,o2);
            swap(t1,t3);
            swap(o1,o3);
        }
    }
    else// if(3 == zero_count)// put the non-zero orient points as t3; while keeping 1 == orient3d(t0,t1,t2,t3)
    {
         if(0 != o0)
         {
             swap(t0,t3);
             swap(o0,o3);
             swap(t1,t2);
             swap(o1,o2);
            
         }
         else if(0 != o1)
         {
             swap(t1,t3);
             swap(o1,o3);
             swap(t0,t2);
             swap(o0,o2);
            
         }
         else if(0 != o2)
         {
             swap(t2,t3);
             swap(o2,o3);
             swap(t0,t1);
             swap(o0,o1);
         }
    }

    assert(1 == orient3d(t0,t1,t2,t3));
}

uint32_t add_LPI(uint32_t e0, uint32_t e1, uint32_t p0,uint32_t p1,uint32_t p2)
{
    uint32_t res = m_vertices.size();
    m_vertices.push_back(new implicitPoint3D_LPI(
        m_vertices[e0]->toExplicit3D(),m_vertices[e1]->toExplicit3D(),
        m_vertices[p0]->toExplicit3D(),m_vertices[p1]->toExplicit3D(),m_vertices[p2]->toExplicit3D()));

    m_new_vertices_mappings.push_back(NineUInt32(e0,e1,p0,p1,p2));
    return res;
}
uint32_t add_TPI(uint32_t p0,uint32_t p1,uint32_t p2,uint32_t p3,uint32_t p4,uint32_t p5,uint32_t p6,uint32_t p7,uint32_t p8)
{
//    if(p0 > m_vertices_count || p1 > m_vertices_count || p2 > m_vertices_count || p3 > m_vertices_count || p4 > m_vertices_count || p5 > m_vertices_count || p6 > m_vertices_count || p7 > m_vertices_count || p8 > m_vertices_count)
//    {
//        throw "wtf";
//    }
    uint32_t res = m_vertices.size();
    m_vertices.push_back(new implicitPoint3D_TPI(
        m_vertices[p0]->toExplicit3D(),m_vertices[p1]->toExplicit3D(),m_vertices[p2]->toExplicit3D(),
        m_vertices[p3]->toExplicit3D(),m_vertices[p4]->toExplicit3D(),m_vertices[p5]->toExplicit3D(),
        m_vertices[p6]->toExplicit3D(),m_vertices[p7]->toExplicit3D(),m_vertices[p8]->toExplicit3D()));

    m_new_vertices_mappings.push_back(NineUInt32(p0,p1,p2,p3,p4,p5,p6,p7,p8));
    return res;
}
