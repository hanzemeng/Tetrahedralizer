#include "facet_association.h"

void facet_association()
{
    // calculate tetrahedron neighbors and vertices incidents
    {
        m_map_iii_i_0.clear();
        m_neighbors.resize(m_tetrahedrons.size());
        m_vertices_incidents.resize(m_vertices_count);
        for(uint32_t i=0; i<m_neighbors.size(); i++)
        {
            m_neighbors[i] = UNDEFINED_VALUE;
        }
        for(uint32_t i=0; i<m_tetrahedrons.size(); i++)
        {
            uint32_t p0,p1,p2;
            get_tetrahedron_face(i, p0, p1, p2);
            sort_ints(p0,p1,p2);
            auto it = m_map_iii_i_0.find({p0,p1,p2});
            if(m_map_iii_i_0.end() != it)
            {
                uint32_t n = it->second;
                m_neighbors[i] = n;
                m_neighbors[n] = i;
                m_map_iii_i_0.erase(it);
            }
            else
            {
                m_map_iii_i_0[{p0,p1,p2}] = i;
            }
            
            m_vertices_incidents[m_tetrahedrons[i]] = i&0xfffffffc;
        }
    }
    
    // calculate constraint neighbors
    {
        m_map_ii_i_0.clear();
        m_vector_i_0.resize(3*m_constraints_count); // m_vector_i_0 is constraint neighbors
        for(uint32_t i=0; i<3*m_constraints_count; i++)
        {
            m_vector_i_0[i] = UNDEFINED_VALUE;
        }
        for(uint32_t i=0; i<m_constraints.size(); i+=3)
        {
            for(uint32_t j=0; j<3; j++)
            {
                uint32_t p0 = m_constraints[i+j];
                uint32_t p1 = m_constraints[i+(j+1)%3];
                sort_ints(p0,p1);
                
                auto it = m_map_ii_i_0.find({p0,p1});
                if(m_map_ii_i_0.end() != it)
                {
                    uint32_t n = it->second;
                    m_vector_i_0[i+j] = n;
                    m_vector_i_0[n] = i+j;
                    m_map_ii_i_0.erase(it);
                }
                else
                {
                    m_map_ii_i_0[{p0,p1}] = i+j;
                }
            }
        }
    }

    // calculate coplanar constraints
    {
        m_vector_i_1.resize(m_constraints_count);
        for(uint32_t i=0; i<m_constraints_count; i++)
        {
            m_vector_i_1[i] = UNDEFINED_VALUE;
        }
        
        uint32_t facet_count = 0;
        for(uint32_t i=0; i<m_constraints_count; i++)
        {
            if(UNDEFINED_VALUE != m_vector_i_1[i])
            {
                continue;
            }

            clear_queue(m_queue_i_0);
            m_queue_i_0.push(i);
            while(!m_queue_i_0.empty())
            {
                uint32_t c = m_queue_i_0.front();
                m_queue_i_0.pop();
                
                if(UNDEFINED_VALUE != m_vector_i_1[c])
                {
                    continue;
                }
                m_vector_i_1[c] = facet_count;
                
                uint32_t p0(m_constraints[3*c+0]),p1(m_constraints[3*c+1]),p2(m_constraints[3*c+2]);
                for(uint32_t j=0; j<3; j++)
                {
                    uint32_t n = m_vector_i_0[3*c+j];
                    if(UNDEFINED_VALUE == n)
                    {
                        continue;
                    }
                    uint32_t r = n%3;
                    uint32_t p3 = m_constraints[n-r + (r+2)%3];
                    if(0 != orient3d(p0,p1,p2,p3))
                    {
                        continue;
                    }
                    m_queue_i_0.push(n/3);
                }
            }
            facet_count++;
        }
        
        m_coplanar_constraints.resize(facet_count);
        for(uint32_t i=0; i<facet_count; i++)
        {
            m_coplanar_constraints[i].clear();
        }
        for(uint32_t i=0; i<m_constraints_count; i++)
        {
            uint32_t t0(m_constraints[3*i+0]),t1(m_constraints[3*i+1]),t2(m_constraints[3*i+2]);
            if(t0 == t1 || t0 == t2 || t1 == t2 || is_collinear(t0, t1, t2))
            {
                continue;
            }
            m_coplanar_constraints[m_vector_i_1[i]].push_back(i);
        }
    }
    
    {
        m_polyhedrons_facets_associations.resize(3*m_tetrahedrons.size());
        for(uint32_t i=0; i<m_polyhedrons_facets_associations.size(); i++)
        {
            m_polyhedrons_facets_associations[i] = UNDEFINED_VALUE;
        }
        
        for(uint32_t i=0; i<m_coplanar_constraints.size(); i++)
        {
            m_map_i_i_0.clear(); // stores vertices orients
            for(uint32_t j=0; j<m_coplanar_constraints[i].size(); j++)
            {
                uint32_t c = m_coplanar_constraints[i][j];
                
                uint32_t c0 = m_constraints[3*c+0];
                uint32_t c1 = m_constraints[3*c+1];
                uint32_t c2 = m_constraints[3*c+2];
                int ignore_axis = max_component_in_triangle_normal(c0,c1,c2);
                        
                // m_u_set_i_0 stores the polyhedrons that incident at one of the point of the cth constraint
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

                m_u_set_i_1.clear(); // stores visited polyhedrons
                m_u_set_i_1.insert(UNDEFINED_VALUE);
                for(uint32_t t : m_u_set_i_0)
                {
                    m_queue_i_0.push(t); // stores polyhedrons to be visited
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
                    uint32_t t0 = m_tetrahedrons[t+0];
                    uint32_t t1 = m_tetrahedrons[t+1];
                    uint32_t t2 = m_tetrahedrons[t+2];
                    uint32_t t3 = m_tetrahedrons[t+3];
                    
                    if(m_map_i_i_0.end() == m_map_i_i_0.find(t0))
                    {
                        m_map_i_i_0[t0] = orient3d(c0,c1,c2,t0);
                    }
                    if(m_map_i_i_0.end() == m_map_i_i_0.find(t1))
                    {
                        m_map_i_i_0[t1] = orient3d(c0,c1,c2,t1);
                    }
                    if(m_map_i_i_0.end() == m_map_i_i_0.find(t2))
                    {
                        m_map_i_i_0[t2] = orient3d(c0,c1,c2,t2);
                    }
                    if(m_map_i_i_0.end() == m_map_i_i_0.find(t3))
                    {
                        m_map_i_i_0[t3] = orient3d(c0,c1,c2,t3);
                    }
                    
                    int o0 = m_map_i_i_0[t0];
                    int o1 = m_map_i_i_0[t1];
                    int o2 = m_map_i_i_0[t2];
                    int o3 = m_map_i_i_0[t3];
                    
                    if(o0 == o1 && o1 == o2 && o2 == o3 && 0 != o0)
                    {
                        continue;
                    }
                    
                    for(uint32_t k=0; k<4; k++)
                    {
                        uint32_t n;
                        get_tetrahedron_neighbor(t, k, n);
                        m_queue_i_0.push(n);
                    }
                    m_u_set_i_0.insert(t);
                }
                
//                for(uint32_t i=0; i<m_tetrahedrons.size(); i+=4)
//                {
//                    m_u_set_i_0.insert(i);
//                }
                m_u_set_i_0.erase(UNDEFINED_VALUE);
                for(uint32_t t : m_u_set_i_0)
                {
                    for(uint32_t k=0; k<4; k++)
                    {
                        uint32_t ps[3];
                        get_tetrahedron_face(t, k, ps[0], ps[1], ps[2]);
                        bool is_coplanar_face = true;
                        for(uint32_t l=0; l<3; l++)
                        {
                            if(m_map_i_i_0.end() == m_map_i_i_0.find(ps[l]))
                            {
                                m_map_i_i_0[ps[l]] = orient3d(c0,c1,c2,ps[l]);
                            }
                            
                            if(0 != m_map_i_i_0[ps[l]])
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
                            if(UNDEFINED_VALUE != m_polyhedrons_facets_associations[3*t+3*k+l])
                            {
                                continue;
                            }
                            if(ps[l] == c0 || ps[l] == c1 || ps[l] == c2 || 
                               vertex_in_segment(ps[l], c0, c1) ||
                               vertex_in_segment(ps[l], c1, c2) ||
                               vertex_in_segment(ps[l], c2, c0) ||
                               vertex_in_segment(ps[l], c0, c1, ignore_axis) ||
                               vertex_in_segment(ps[l], c1, c2, ignore_axis) ||
                               vertex_in_segment(ps[l], c2, c0, ignore_axis) ||
                               vertex_in_triangle(ps[l], c0, c1, c2) ||
                               vertex_in_triangle(ps[l], c0, c1, c2, ignore_axis)
                               )
                            {
                                m_polyhedrons_facets_associations[3*t+3*k+l] = c;
                            }
                        }
                    }
                }
            }
        }
    }
}
