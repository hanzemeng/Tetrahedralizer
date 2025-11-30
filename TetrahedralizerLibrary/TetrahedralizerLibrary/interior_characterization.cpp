#include "interior_characterization.hpp"


void InteriorCharacterizationHandle::interior_characterization()
{
    Polyhedralization polyhedralization(m_polyhedrons);
    vector<vector<vector<uint32_t>>> facets_vertices_mapping;
    m_facets_centroids_mapping = vector<uint32_t>(m_facets.size(), UNDEFINED_VALUE);
    vector<double> facets_approximated_areas;
    unordered_map<pair<uint32_t,uint32_t>,vector<uint32_t>,pair_ii_hash> edges_incident_facets;
    vector<vector<pair<uint32_t,uint32_t>>> vertices_incident_edges = vector<vector<pair<uint32_t,uint32_t>>>(m_vertices.size());
    
    
    // calculate some data
    {
        vector<double3> vertices_approximated_positions;
        for(uint32_t i=0; i<m_vertices.size(); i++)
        {
            vertices_approximated_positions.push_back(approximate_point(m_vertices[i]));
        }
        double total_area = 0.0;
        for(uint32_t i=0; i<m_facets.size(); i++)
        {
            facets_vertices_mapping.push_back(vector<vector<uint32_t>>(m_facets[i].size()));
            
            double area = 0.0;
            double3 p0 = vertices_approximated_positions[m_facets[i][0]];
            for(uint32_t j=1; j<m_facets[i].size()-1; j++)
            {
                double3 p1 = vertices_approximated_positions[m_facets[i][j]];
                double3 p2 = vertices_approximated_positions[m_facets[i][j+1]];
                area += (p1-p0).cross(p2-p0).length();
            }
            facets_approximated_areas.push_back(area);
            total_area += area;
        }
        for(uint32_t i=0; i<facets_approximated_areas.size(); i++)
        {
            facets_approximated_areas[i] /= total_area;
        }
        
        for(uint32_t i=0; i<m_facets.size(); i++)
        {
            for(uint32_t j=0; j<m_facets[i].size(); j++)
            {
                uint32_t e0 = m_facets[i][j];
                uint32_t e1 = m_facets[i][(j+1)%m_facets[i].size()];
                sort_ints(e0, e1);
                edges_incident_facets[make_pair(e0, e1)].push_back(i);
                vertices_incident_edges[e0].push_back(make_pair(e0, e1));
                vertices_incident_edges[e1].push_back(make_pair(e0, e1));
            }
        }
    }

    // associate constraints to facets vertices
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
        for(pair<uint32_t,uint32_t> e : vertices_incident_edges[c0])
        {
            for(uint32_t f : edges_incident_facets[e])
            {
                m_queue_i_0.push(f);
            }
        }
        for(pair<uint32_t,uint32_t> e : vertices_incident_edges[c1])
        {
            for(uint32_t f : edges_incident_facets[e])
            {
                m_queue_i_0.push(f);
            }
        }
        for(pair<uint32_t,uint32_t> e : vertices_incident_edges[c2])
        {
            for(uint32_t f : edges_incident_facets[e])
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
            
            bool search_neighbor = true;
            uint32_t f0 = m_facets[f][0];
            uint32_t f1 = m_facets[f][1];
            uint32_t f2 = m_facets[f][2];
            bool is_coplanar = 0 == orient3d(c0,c1,c2,f0,m_vertices.data()) && 0 == orient3d(c0,c1,c2,f1,m_vertices.data()) && 0 == orient3d(c0,c1,c2,f2,m_vertices.data());
            if(is_coplanar)
            {
                search_neighbor = true;
                for(uint32_t j=0; j<m_facets[f].size(); j++)
                {
                    if(vertex_in_triangle(m_facets[f][j], c0, c1, c2, m_vertices.data()))
                    {
                        facets_vertices_mapping[f][j].push_back(i);
                    }
                }
                if(UNDEFINED_VALUE == m_facets_centroids_mapping[f] && genericPoint::pointInTriangle(*m_facets_centroids[f],*m_vertices[c0],*m_vertices[c1],*m_vertices[c2]))
                {
                    m_facets_centroids_mapping[f] = i;
                }
            }
            if(is_coplanar)
            {
                for(uint32_t j=0; j<m_facets[f].size(); j++)
                {
                    uint32_t e0 = m_facets[f][j];
                    uint32_t e1 = m_facets[f][(j+1)%m_facets[f].size()];
                    sort_ints(e0, e1);
                    for(uint32_t nf : edges_incident_facets[make_pair(e0, e1)])
                    {
                        m_queue_i_0.push(nf);
                    }
                }
            }
        }
    }

    GCoptimizationGeneralGraph gc((GCoptimization::SiteID)m_polyhedrons.size()+1, 2);
    // build graph and calculate min cut
    {
        // polyhedron cost
        gc.setDataCost((GCoptimization::SiteID)m_polyhedrons.size(), 1, 1.0); // ghost polyhedron
        for(uint32_t i=0; i<m_polyhedrons.size(); i++)
        {
            double out_area = 0.0; // area of facets point out
            double in_area = 0.0;
            
            for(uint32_t j=0; j<m_polyhedrons[i].size(); j++)
            {
                int orient = 0;
                uint32_t f = m_polyhedrons[i][j];
                uint32_t t = m_facets_centroids_mapping[f];
                if(UNDEFINED_VALUE == t)
                {
                    continue;
                }
                
                uint32_t t0 = m_constraints[3*t+0];
                uint32_t t1 = m_constraints[3*t+1];
                uint32_t t2 = m_constraints[3*t+2];
                for(uint32_t k=0; k<m_polyhedrons[i].size(); k++)
                {
                    if(k == j)
                    {
                        continue;
                    }
                    uint32_t nf = m_polyhedrons[i][k];
                    uint32_t f0 = m_facets[nf][0];
                    uint32_t f1 = m_facets[nf][1];
                    uint32_t f2 = m_facets[nf][2];
                    if(0 != (orient=orient3d(t0,t1,t2,f0, m_vertices.data())))
                    {
                        break;
                    }
                    if(0 != (orient=orient3d(t0,t1,t2,f1, m_vertices.data())))
                    {
                        break;
                    }
                    if(0 != (orient=orient3d(t0,t1,t2,f2, m_vertices.data())))
                    {
                        break;
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
            gc.setDataCost((GCoptimization::SiteID)i, 0, out_area);
            gc.setDataCost((GCoptimization::SiteID)i, 1, m_polyhedron_in_multiplier * in_area);
        }
        
        // neighbor cost
        vector<bool> polyhedrons_connected_to_ghost = vector<bool>(m_polyhedrons.size(), false);
        for(uint32_t i=0; i<m_facets.size(); i++)
        {
            if(UNDEFINED_VALUE != m_facets_centroids_mapping[i])
            {
                continue;
            }

            uint32_t n0,n1;
            polyhedralization.get_polyhedron_facet_neighbors(i, n0, n1);
            if(UNDEFINED_VALUE == n1)
            {
                if(polyhedrons_connected_to_ghost[n0])
                {
                    continue;
                }
                polyhedrons_connected_to_ghost[n0] = true;
                n1 = m_polyhedrons.size();
            }
            gc.setNeighbors((GCoptimization::SiteID)n0, (GCoptimization::SiteID)n1, facets_approximated_areas[i]);
        }
        
        gc.swap();
    }
    
    // produce output
    {
        for(uint32_t i=0; i<m_polyhedrons.size(); i++)
        {
            m_polyhedrons_labels.push_back(gc.whatLabel((GCoptimization::SiteID)i));
        }
        
        for(uint32_t i=0; i<facets_vertices_mapping.size(); i++)
        {
            for(uint32_t j=0; j<facets_vertices_mapping[i].size(); j++)
            {
                m_facets_vertices_mapping.push_back(facets_vertices_mapping[i][j].size());
                for(uint32_t k=0; k<facets_vertices_mapping[i][j].size(); k++)
                {
                    m_facets_vertices_mapping.push_back(facets_vertices_mapping[i][j][k]);
                }
            }
        }
    }
}


InteriorCharacterizationHandle::InteriorCharacterizationHandle()
{}
void InteriorCharacterizationHandle::Dispose()
{
    delete_vertices(m_vertices);
    delete_vertices(m_facets_centroids);
}

void InteriorCharacterizationHandle::AddInput(uint32_t explicit_count, double* explicit_values, uint32_t implicit_count, uint32_t* implicit_values, uint32_t polyhedrons_count, uint32_t* polyhedrons, uint32_t facets_count, uint32_t* facets, uint32_t* facets_centroids, double* facets_centroids_weights, uint32_t constraints_count, uint32_t* constraints, double polyhedron_in_multiplier)
{
    m_vertices = create_vertices(explicit_count, explicit_values, implicit_count, implicit_values);
    m_polyhedrons = flat_array_to_nested_vector(polyhedrons, polyhedrons_count);
    m_facets = flat_array_to_nested_vector(facets, facets_count);
    for(uint32_t i=0; i<facets_count; i++)
    {
        implicitPoint3D_BPT* p = new implicitPoint3D_BPT(m_vertices[facets_centroids[3*i+0]]->toExplicit3D(),
                                                         m_vertices[facets_centroids[3*i+1]]->toExplicit3D(),
                                                         m_vertices[facets_centroids[3*i+2]]->toExplicit3D(),
                                                         facets_centroids_weights[2*i+0],
                                                         facets_centroids_weights[2*i+1]);
        m_facets_centroids.push_back(p);
    }
    m_constraints = create_constraints(constraints_count, constraints, m_vertices.data(), true);
    m_polyhedron_in_multiplier = polyhedron_in_multiplier;
}

void InteriorCharacterizationHandle::Calculate()
{
    interior_characterization();
}

uint32_t* InteriorCharacterizationHandle::GetPolyhedronsLabels()
{
    return m_polyhedrons_labels.data();
}
uint32_t* InteriorCharacterizationHandle::GetFacetsVerticesMapping()
{
    return m_facets_vertices_mapping.data();
}
uint32_t* InteriorCharacterizationHandle::GetFacetsCentroidsMapping()
{
    return m_facets_centroids_mapping.data();
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

extern "C" LIBRARY_EXPORT void AddInteriorCharacterizationInput(void* handle, uint32_t explicit_count, double* explicit_values, uint32_t implicit_count, uint32_t* implicit_values, uint32_t polyhedrons_count, uint32_t* polyhedrons, uint32_t facets_count, uint32_t* facets, uint32_t* facets_centroids, double* facets_centroids_weights, uint32_t constraints_count, uint32_t* constraints, double polyhedron_in_multiplier)
{
    ((InteriorCharacterizationHandle*)handle)->AddInput(explicit_count, explicit_values, implicit_count, implicit_values, polyhedrons_count, polyhedrons, facets_count, facets, facets_centroids, facets_centroids_weights, constraints_count, constraints, polyhedron_in_multiplier);
}

extern "C" LIBRARY_EXPORT void CalculateInteriorCharacterization(void* handle)
{
    ((InteriorCharacterizationHandle*)handle)->Calculate();
}

extern "C" LIBRARY_EXPORT uint32_t* GetInteriorCharacterizationPolyhedronsLabels(void* handle)
{
    return ((InteriorCharacterizationHandle*)handle)->GetPolyhedronsLabels();
}
extern "C" LIBRARY_EXPORT uint32_t* GetInteriorCharacterizationFacetsVerticesMapping(void* handle)
{
    return ((InteriorCharacterizationHandle*)handle)->GetFacetsVerticesMapping();
}
extern "C" LIBRARY_EXPORT uint32_t* GetInteriorCharacterizationFacetsCentroidsMapping(void* handle)
{
    return ((InteriorCharacterizationHandle*)handle)->GetFacetsCentroidsMapping();
}
