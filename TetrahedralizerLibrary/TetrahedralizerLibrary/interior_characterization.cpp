#include "interior_characterization.hpp"
using namespace std;

void InteriorCharacterizationHandle::interior_characterization()
{
    auto get_coplanar_group = [&](uint32_t p0,uint32_t p1,uint32_t p2) -> uint32_t
    {
        sort_ints(p0,p1,p2);
        return m_triangles_coplanar_groups[make_tuple(p0,p1,p2)];
    };
    
    vector<double> facets_approximated_areas;
    {
        vector<double3> vertices_approximated_positions;
        approximate_verteices(vertices_approximated_positions, m_vertices);
        double total_area = 0.0;
        for(uint32_t i=0; i<m_facets.size(); i++)
        {
            vector<uint32_t> vs = m_facets[i].get_sorted_vertices(m_segments);
            double area = 0.0;
            double3 p0 = vertices_approximated_positions[vs[0]];
            for(uint32_t j=1; j<vs.size()-1; j++)
            {
                double3 p1 = vertices_approximated_positions[vs[j]];
                double3 p2 = vertices_approximated_positions[vs[j+1]];
                area += (p1-p0).cross(p2-p0).length();
            }
            facets_approximated_areas.push_back(area);
            total_area += area;
        }
        for(uint32_t i=0; i<facets_approximated_areas.size(); i++)
        {
            facets_approximated_areas[i] /= total_area;
        }
    }
    
    vector<vector<uint32_t>> constraints_coplanar_groups;
    for(uint32_t i=0; i<m_constraints.size()/3; i++)
    {
        uint32_t c0 = m_constraints[3*i+0];
        uint32_t c1 = m_constraints[3*i+1];
        uint32_t c2 = m_constraints[3*i+2];
        if(UNDEFINED_VALUE == c0)
        {
            continue;
        }
        uint32_t cg = get_coplanar_group(c0,c1,c2);
        while(constraints_coplanar_groups.size() <= cg)
        {
            constraints_coplanar_groups.push_back(vector<uint32_t>());
        }
        constraints_coplanar_groups[cg].push_back(c0);
        constraints_coplanar_groups[cg].push_back(c1);
        constraints_coplanar_groups[cg].push_back(c2);
    }
    
    vector<uint32_t> facets_centroids_incident_constraints = vector<uint32_t>(3*m_facets.size(), UNDEFINED_VALUE);
    for(uint32_t i=0; i<m_facets.size(); i++)
    {
        shared_ptr<genericPoint> centroid = make_shared<implicitPoint3D_BPT>
        (m_vertices[m_facets[i].p0]->toExplicit3D(), m_vertices[m_facets[i].p1]->toExplicit3D(), m_vertices[m_facets[i].p2]->toExplicit3D(),
         m_facets[i].w0, m_facets[i].w1);
        
        uint32_t cg = get_coplanar_group(m_facets[i].p0, m_facets[i].p1, m_facets[i].p2);
        if(cg >= constraints_coplanar_groups.size())
        {
            continue;
        }
        for(uint32_t j=0; j<constraints_coplanar_groups[cg].size()/3; j++)
        {
            uint32_t c0 = constraints_coplanar_groups[cg][3*j+0];
            uint32_t c1 = constraints_coplanar_groups[cg][3*j+1];
            uint32_t c2 = constraints_coplanar_groups[cg][3*j+2];
            if(genericPoint::pointInTriangle(*centroid,*m_vertices[c0],*m_vertices[c1],*m_vertices[c2]))
            {
                facets_centroids_incident_constraints[3*i+0] = c0;
                facets_centroids_incident_constraints[3*i+1] = c1;
                facets_centroids_incident_constraints[3*i+2] = c2;
                break;
            }
        }
    }

    GCoptimizationGeneralGraph gc((GCoptimization::SiteID)m_polyhedrons.size()+1, 2);
    {
        // neighbor cost and neighbor graph
        vector<vector<uint32_t>> neighbor_graph = vector<vector<uint32_t>>(m_polyhedrons.size()+1);
        vector<double> polyhedrons_to_ghost_weight = vector<double>(m_polyhedrons.size(), -1.0); // total area of facets connected to the ghost polyhedron, negative means not connectd
        for(uint32_t i=0; i<m_facets.size(); i++)
        {
            if(UNDEFINED_VALUE != facets_centroids_incident_constraints[3*i+0])
            {
                continue;
            }

            uint32_t n0 = m_facets[i].ip0;
            uint32_t n1 = m_facets[i].ip1;
            if(n0 == UNDEFINED_VALUE)
            {
                swap(n0,n1);
            }

            if(UNDEFINED_VALUE == n1)
            {
                neighbor_graph[n0].push_back(m_polyhedrons.size());
                neighbor_graph[m_polyhedrons.size()].push_back(n0);
                
                if(polyhedrons_to_ghost_weight[n0] < 0.0)
                {
                    polyhedrons_to_ghost_weight[n0] = facets_approximated_areas[i];
                }
                else
                {
                    polyhedrons_to_ghost_weight[n0] += facets_approximated_areas[i];
                }
                continue;
            }
            neighbor_graph[n0].push_back(n1);
            neighbor_graph[n1].push_back(n0);
            gc.setNeighbors((GCoptimization::SiteID)n0, (GCoptimization::SiteID)n1, facets_approximated_areas[i]);
        }
        for(uint32_t i=0; i<m_polyhedrons.size(); i++)
        {
            if(polyhedrons_to_ghost_weight[i] > 0.0)
            {
                gc.setNeighbors((GCoptimization::SiteID)i, (GCoptimization::SiteID)m_polyhedrons.size(), polyhedrons_to_ghost_weight[i]);
            }
        }
        
        // check polygons that can be reached from ghost
        vector<bool> reachable_from_ghost = vector<bool>(m_polyhedrons.size()+1,false);
        {
            queue<uint32_t> visit_polyhedrons;
            visit_polyhedrons.push(m_polyhedrons.size());
            while(!visit_polyhedrons.empty())
            {
                uint32_t p = visit_polyhedrons.front();
                visit_polyhedrons.pop();
                if(reachable_from_ghost[p])
                {
                    continue;
                }
                reachable_from_ghost[p] = true;
                for(uint32_t n : neighbor_graph[p])
                {
                    visit_polyhedrons.push(n);
                }
            }
        }
        
        // polyhedron cost
        gc.setDataCost((GCoptimization::SiteID)m_polyhedrons.size(), 1, 1.0); // ghost polyhedron
        for(uint32_t i=0; i<m_polyhedrons.size(); i++)
        {
            double out_area = 0.0; // area of facets point out
            double in_area = 0.0;
            
            for(uint32_t j=0; j<m_polyhedrons[i].size(); j++)
            {
                uint32_t f = m_polyhedrons[i][j];
                uint32_t t0 = facets_centroids_incident_constraints[3*f+0];
                uint32_t t1 = facets_centroids_incident_constraints[3*f+1];
                uint32_t t2 = facets_centroids_incident_constraints[3*f+2];
                if(UNDEFINED_VALUE == t0)
                {
                    continue;
                }
                
                int orient = 0;
                for(uint32_t k=0; k<m_polyhedrons[i].size(); k++)
                {
                    if(k == j)
                    {
                        continue;
                    }
                    uint32_t nf = m_polyhedrons[i][k];
                    for(uint32_t v : m_facets[nf].get_vertices(m_segments))
                    {
                        if(0 != (orient=orient3d(t0,t1,t2,v, m_vertices.data())))
                        {
                            break;
                        }
                    }
                }
                if(orient > 0)
                {
                    in_area += facets_approximated_areas[f];
                }
                else if(orient < 0)
                {
                    out_area += facets_approximated_areas[f];
                }
//                else
//                {
//                    throw "wtf";
//                }
            }
            
            if(reachable_from_ghost[i])
            {
                gc.setDataCost((GCoptimization::SiteID)i, 0, out_area);
                gc.setDataCost((GCoptimization::SiteID)i, 1, m_polyhedron_in_multiplier * in_area);
            }
            else
            {
                gc.setDataCost((GCoptimization::SiteID)i, 0, 1.0); // the polyhedron has to be in if it can't be reached from the ghost
                gc.setDataCost((GCoptimization::SiteID)i, 1, 0.0);
            }
        }
        
        gc.swap();
    }
    
    for(uint32_t i=0; i<m_polyhedrons.size(); i++)
    {
        m_polyhedrons_labels.push_back(gc.whatLabel((GCoptimization::SiteID)i));
    }
}


void InteriorCharacterizationHandle::Dispose()
{}

void InteriorCharacterizationHandle::AddInput(uint32_t explicit_count, double* explicit_values, uint32_t implicit_count, uint32_t* implicit_values,
                                              uint32_t polyhedrons_count, uint32_t* polyhedrons, uint32_t facets_count, FacetInteropData* facets, uint32_t segments_count, SegmentInteropData* segments,
                                              uint32_t coplanar_triangles_count, uint32_t* coplanar_triangles, uint32_t constraints_count, uint32_t* constraints, double polyhedron_in_multiplier)
{
    m_vertices = create_vertices(explicit_count, explicit_values, implicit_count, implicit_values);
    m_polyhedrons = flat_array_to_nested_vector(polyhedrons, polyhedrons_count);
    for(uint32_t i=0; i<facets_count; i++)
    {
        m_facets.push_back(facets[i].to_facet());
    }
    for(uint32_t i=0; i<segments_count; i++)
    {
        m_segments.push_back(segments[i].to_segment());
    }
    vector<vector<uint32_t>> temp = flat_array_to_nested_vector(coplanar_triangles, coplanar_triangles_count);
    for(uint32_t i=0; i<temp.size(); i++)
    {
        for(uint32_t j=0; j<temp[i].size()/3; j++)
        {
            uint32_t c0 = temp[i][3*j+0];
            uint32_t c1 = temp[i][3*j+1];
            uint32_t c2 = temp[i][3*j+2];
            sort_ints(c0, c1, c2);
            m_triangles_coplanar_groups[make_tuple(c0,c1,c2)] = i;
        }
    }
    
    m_constraints = create_constraints(constraints_count, constraints, m_vertices.data(), true);
    m_polyhedron_in_multiplier = polyhedron_in_multiplier;
}

void InteriorCharacterizationHandle::Calculate()
{
    interior_characterization();
}

void InteriorCharacterizationHandle::GetPolyhedronsLabels(uint32_t* out)
{
    write_buffer_with_vector(out, m_polyhedrons_labels);
}

extern "C" LIBRARY_EXPORT void* CreateInteriorCharacterizationHandle()
{
    return new InteriorCharacterizationHandle();
}
extern "C" LIBRARY_EXPORT void DisposeInteriorCharacterizationHandle(void* handle)
{
    ((InteriorCharacterizationHandle*)handle)->Dispose();
    delete (InteriorCharacterizationHandle*)handle;
}

extern "C" LIBRARY_EXPORT void AddInteriorCharacterizationInput(void* handle, uint32_t explicit_count, double* explicit_values, uint32_t implicit_count, uint32_t* implicit_values,
                                                                uint32_t polyhedrons_count, uint32_t* polyhedrons, uint32_t facets_count, FacetInteropData* facets, uint32_t segments_count, SegmentInteropData* segments,
                                                                uint32_t coplanar_triangles_count, uint32_t* coplanar_triangles, uint32_t constraints_count, uint32_t* constraints, double polyhedron_in_multiplier)
{
    ((InteriorCharacterizationHandle*)handle)->AddInput(explicit_count, explicit_values, implicit_count, implicit_values,
                                                        polyhedrons_count, polyhedrons, facets_count, facets, segments_count, segments,
                                                        coplanar_triangles_count, coplanar_triangles, constraints_count, constraints, polyhedron_in_multiplier);
}

extern "C" LIBRARY_EXPORT void CalculateInteriorCharacterization(void* handle)
{
    ((InteriorCharacterizationHandle*)handle)->Calculate();
}

extern "C" LIBRARY_EXPORT void GetInteriorCharacterizationPolyhedronsLabels(void* handle, uint32_t* out)
{
    ((InteriorCharacterizationHandle*)handle)->GetPolyhedronsLabels(out);
}
