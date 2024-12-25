#include "interior_characterization.h"


void interior_characterization()
{
    for(uint32_t i=0; i<m_polyhedrons_facets.size(); i++)
    {
        sort_polyhedron_facet(i);
    }
    
    // estimate some numbers
    {
        // vertices positions
        m_vertices_approximate_positions.resize(3*m_vertices.size());
        for(uint32_t i=0; i<m_vertices.size(); i++)
        {
            double x,y,z;
            m_vertices[i]->getApproxXYZCoordinates(x, y, z, true);
            m_vertices_approximate_positions[3*i+0] = x;
            m_vertices_approximate_positions[3*i+1] = y;
            m_vertices_approximate_positions[3*i+2] = z;
        }
        
        // facets areas
        double total_area = 0;
        m_polyhedrons_facets_approximate_areas.resize(m_polyhedrons_facets.size());
        for(uint32_t i=0; i<m_polyhedrons_facets.size(); i++)
        {
            double area = 0;
            get_polyhedron_facet_vertices(i, m_deque_i_0);
            double a11,a12,a13,a21,a22,a23,a31,a32,a33, r1,r2,r3;
            a11 = m_vertices_approximate_positions[3*m_deque_i_0[0]+0];
            a12 = m_vertices_approximate_positions[3*m_deque_i_0[0]+1];
            a13 = m_vertices_approximate_positions[3*m_deque_i_0[0]+2];
            
            for(uint32_t j=2; j<m_deque_i_0.size(); j++)
            {
//                a21 = m_vertices_approximate_positions[3*m_deque_i_0[j-1]+0];
//                a22 = m_vertices_approximate_positions[3*m_deque_i_0[j-1]+1];
//                a23 = m_vertices_approximate_positions[3*m_deque_i_0[j-1]+2];
//                a31 = m_vertices_approximate_positions[3*m_deque_i_0[j]+0];
//                a32 = m_vertices_approximate_positions[3*m_deque_i_0[j]+1];
//                a33 = m_vertices_approximate_positions[3*m_deque_i_0[j]+2];
//                area += ((a11)*((a22)*(a33) - (a23)*(a32)) - (a12)*((a21)*(a33) - (a23)*(a31)) + (a13)*((a21)*(a32) - (a22)*(a31)));
                
                a21 = m_vertices_approximate_positions[3*m_deque_i_0[j-1]+0] - a11;
                a22 = m_vertices_approximate_positions[3*m_deque_i_0[j-1]+1] - a12;
                a23 = m_vertices_approximate_positions[3*m_deque_i_0[j-1]+2] - a13;
                a31 = m_vertices_approximate_positions[3*m_deque_i_0[j]+0] - a11;
                a32 = m_vertices_approximate_positions[3*m_deque_i_0[j]+1] - a12;
                a33 = m_vertices_approximate_positions[3*m_deque_i_0[j]+2] - a13;

                r1 = a22*a33 - a23*a32;
                r2 = -a21*a33 + a23*a31;
                r3 = a21*a32 - a22*a31;

                area += r1*r1 + r2*r2 + r3*r3;
            }
//            area = fabs(area);
            total_area += area;
            m_polyhedrons_facets_approximate_areas[i] = area;
        }
        for(uint32_t i=0; i<m_polyhedrons_facets.size(); i++)
        {
            m_polyhedrons_facets_approximate_areas[i] /= total_area;
        }
        
        // facets centers
        m_polyhedrons_facets_approximate_centers.resize(3*m_polyhedrons_facets.size());
        for(uint32_t i=0; i<m_polyhedrons_facets.size(); i++)
        {
            get_polyhedron_facet_vertices(i, m_deque_i_0);
            double cx(0),cy(0),cz(0);
            
            for(uint32_t v : m_deque_i_0)
            {
                cx += m_vertices_approximate_positions[3*v+0];
                cy += m_vertices_approximate_positions[3*v+1];
                cz += m_vertices_approximate_positions[3*v+2];
            }
            cx /= m_deque_i_0.size();
            cy /= m_deque_i_0.size();
            cz /= m_deque_i_0.size();
            m_polyhedrons_facets_approximate_centers[3*i+0] = cx;
            m_polyhedrons_facets_approximate_centers[3*i+1] = cy;
            m_polyhedrons_facets_approximate_centers[3*i+2] = cz;
        }
        
        // polyhedrons winding numbers
        m_polyhedrons_winding_numbers.resize(m_polyhedrons.size());
        for(uint32_t i=0; i<m_polyhedrons.size(); i++)
        {
            double px(0),py(0),pz(0);
            for(uint32_t f : m_polyhedrons[i]->facets)
            {
                px += m_polyhedrons_facets_approximate_centers[3*f+0];
                py += m_polyhedrons_facets_approximate_centers[3*f+1];
                pz += m_polyhedrons_facets_approximate_centers[3*f+2];
            }
            px /= m_polyhedrons[i]->facets.size();
            py /= m_polyhedrons[i]->facets.size();
            pz /= m_polyhedrons[i]->facets.size();
            
            double wind = 0;
            for(uint32_t j=0; j<3*m_constraints_count; j+=3)
            {
                uint32_t c0(m_constraints[j+0]),c1(m_constraints[j+1]),c2(m_constraints[j+2]);
                
                double
                ax(m_vertices_approximate_positions[3*c0+0] - px),
                ay(m_vertices_approximate_positions[3*c0+1] - py),
                az(m_vertices_approximate_positions[3*c0+2] - pz),
                bx(m_vertices_approximate_positions[3*c1+0] - px),
                by(m_vertices_approximate_positions[3*c1+1] - py),
                bz(m_vertices_approximate_positions[3*c1+2] - pz),
                cx(m_vertices_approximate_positions[3*c2+0] - px),
                cy(m_vertices_approximate_positions[3*c2+1] - py),
                cz(m_vertices_approximate_positions[3*c2+2] - pz);
                
                double
                aa(sqrt(ax*ax + ay*ay + az*az)),
                ba(sqrt(bx*bx + by*by + bz*bz)),
                ca(sqrt(cx*cx + cy*cy + cz*cz));
                
                double y = ax*(by*cz-bz*cy) - ay*(bx*cz-bz*cx) + az*(bx*cy-by*cx);
                double x = aa*ba*ca + (ax*bx+ay*by+az*bz)*ca + (bx*cx+by*cy+bz*cz)*aa + (cx*ax+cy*ay+cz*az)*ba;
                double cur_wind = atan2(y, x);
                cur_wind /= (2*numbers::pi);
                
                wind += cur_wind;
            }
            m_polyhedrons_winding_numbers[i] = wind;
        }
    }
    
    
    
    // build the graph for min cut
    {
        GCoptimizationGeneralGraph gc((GCoptimization::SiteID)(m_polyhedrons.size()+1), 2);
        
//        double gamma(1), sigma(1); // parameters that the paper does not explain how to choose their values
//        double multiplier = gamma/(2*2*sigma*sigma);
        double multiplier = 0.1;
        for(uint32_t i=0; i<m_polyhedrons.size(); i++)
        {
            gc.setDataCost((GCoptimization::SiteID)i, 0, max(m_polyhedrons_winding_numbers[i],(double)0));
            gc.setDataCost((GCoptimization::SiteID)i, 1, max(1-m_polyhedrons_winding_numbers[i],(double)0));
            
            for(uint32_t f : m_polyhedrons[i]->facets)
            {
                uint32_t n = UNDEFINED_VALUE;
                if(i == m_polyhedrons_facets[f]->ip0)
                {
                    n = m_polyhedrons_facets[f]->ip1;
                }
                else
                {
                    n = m_polyhedrons_facets[f]->ip0;
                }
                if(UNDEFINED_VALUE == n)
                {
                    continue;
                }
                double w = m_polyhedrons_winding_numbers[i] - m_polyhedrons_winding_numbers[n];
                w = m_polyhedrons_facets_approximate_areas[f] * exp(-(w*w));
                gc.setNeighbors((GCoptimization::SiteID)i, (GCoptimization::SiteID)n, multiplier*w);
            }
        }
        
        gc.swap();
        m_graph_label.resize(m_polyhedrons.size());
        for(uint32_t i=0; i<m_polyhedrons.size(); i++)
        {
            m_graph_label[i] = gc.whatLabel((GCoptimization::SiteID)i);
        }
    }
}

void sort_polyhedron_facet(uint32_t facet)
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

void get_polyhedron_facet_vertices(uint32_t facet, deque<uint32_t>& res)
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
