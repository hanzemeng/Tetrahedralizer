#include "interior_characterization.hpp"


void InteriorCharacterization::interior_characterization(InteriorCharacterizationInput* input, InteriorCharacterizationOutput* output)
{
    Polyhedralization polyhedralization(input->m_polyhedrons, input->m_polyhedrons_count, input->m_facets, input->m_facets_count);
    vector<vector<uint32_t>> polyhedrons = flat_array_to_nested_vector(input->m_polyhedrons, input->m_polyhedrons_count);
    vector<vector<uint32_t>> facets = flat_array_to_nested_vector(input->m_facets, input->m_facets_count);
    vector<vector<vector<uint32_t>>> facets_vertices_mapping;
    vector<uint32_t> facets_centroids_mapping = vector<uint32_t>(input->m_facets_count, UNDEFINED_VALUE);
    vector<double> facets_approximated_areas;
    unordered_map<pair<uint32_t,uint32_t>,vector<uint32_t>,pair_ii_hash> edges_incident_facets;
    vector<vector<pair<uint32_t,uint32_t>>> vertices_incident_edges = vector<vector<pair<uint32_t,uint32_t>>>(input->m_vertices_count);
    
    
    // calculate some data
    {
        vector<double3> vertices_approximated_positions;
        for(uint32_t i=0; i<input->m_constraints_count; i++)
        {
            uint32_t c0 = input->m_constraints[3*i+0];
            uint32_t c1 = input->m_constraints[3*i+1];
            uint32_t c2 = input->m_constraints[3*i+2];
            if(c0==c1||c0==c2||c1==c2||is_collinear(c0, c1, c2, input->m_vertices))
            {
                input->m_constraints[3*i+0] = UNDEFINED_VALUE;
            }
        }
        for(uint32_t i=0; i<input->m_vertices_count; i++)
        {
            vertices_approximated_positions.push_back(approximate_point(input->m_vertices[i]));
        }
        double total_area = 0.0;
        for(uint32_t i=0; i<facets.size(); i++)
        {
            facets_vertices_mapping.push_back(vector<vector<uint32_t>>(facets[i].size()));
            
            double area = 0.0;
            double3 p0 = vertices_approximated_positions[facets[i][0]];
            for(uint32_t j=1; j<facets[i].size()-1; j++)
            {
                double3 p1 = vertices_approximated_positions[facets[i][j]];
                double3 p2 = vertices_approximated_positions[facets[i][j+1]];
                area += (p1-p0).cross(p2-p0).length();
            }
            facets_approximated_areas.push_back(area);
            total_area += area;
        }
        for(uint32_t i=0; i<facets_approximated_areas.size(); i++)
        {
            facets_approximated_areas[i] /= total_area;
        }
        
        for(uint32_t i=0; i<facets.size(); i++)
        {
            for(uint32_t j=0; j<facets[i].size(); j++)
            {
                uint32_t e0 = facets[i][j];
                uint32_t e1 = facets[i][(j+1)%facets[i].size()];
                sort_ints(e0, e1);
                edges_incident_facets[make_pair(e0, e1)].push_back(i);
                vertices_incident_edges[e0].push_back(make_pair(e0, e1));
                vertices_incident_edges[e1].push_back(make_pair(e0, e1));
            }
        }
    }

    // associate constraints to facets vertices
    for(uint32_t i=0; i<input->m_constraints_count; i++)
    {
        uint32_t c0 = input->m_constraints[3*i+0];
        uint32_t c1 = input->m_constraints[3*i+1];
        uint32_t c2 = input->m_constraints[3*i+2];
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
            uint32_t f0 = facets[f][0];
            uint32_t f1 = facets[f][1];
            uint32_t f2 = facets[f][2];
            bool is_coplanar = 0 == orient3d(c0,c1,c2,f0,input->m_vertices) && 0 == orient3d(c0,c1,c2,f1,input->m_vertices) && 0 == orient3d(c0,c1,c2,f2,input->m_vertices);
            if(is_coplanar)
            {
                search_neighbor = true;
                for(uint32_t j=0; j<facets[f].size(); j++)
                {
                    if(vertex_in_triangle(facets[f][j], c0, c1, c2, input->m_vertices))
                    {
                        facets_vertices_mapping[f][j].push_back(i);
                    }
                }
                if(UNDEFINED_VALUE == facets_centroids_mapping[f] && genericPoint::pointInTriangle(*input->m_facets_centroids[f],*input->m_vertices[c0],*input->m_vertices[c1],*input->m_vertices[c2]))
                {
                    facets_centroids_mapping[f] = i;
                }
            }
            if(is_coplanar)
            {
                for(uint32_t j=0; j<facets[f].size(); j++)
                {
                    uint32_t e0 = facets[f][j];
                    uint32_t e1 = facets[f][(j+1)%facets[f].size()];
                    sort_ints(e0, e1);
                    for(uint32_t nf : edges_incident_facets[make_pair(e0, e1)])
                    {
                        m_queue_i_0.push(nf);
                    }
                }
            }
//            if(!search_neighbor)
//            {
//                f0 = facets[f][0];
//                for(uint32_t j=1; j<facets[f].size()-1; j++)
//                {
//                    if(is_coplanar)
//                    {
//                        f1 = facets[f][j];
//                        f2 = facets[f][j+1];
//                        if(vertex_in_triangle(c0, f0, f1, f2, input->m_vertices) ||
//                           vertex_in_triangle(c1, f0, f1, f2, input->m_vertices) ||
//                           vertex_in_triangle(c2, f0, f1, f2, input->m_vertices) ||
//                           segment_cross_segment(c0, c1, f0, f1, input->m_vertices) ||
//                           segment_cross_segment(c0, c1, f1, f2, input->m_vertices) ||
//                           segment_cross_segment(c0, c1, f2, f0, input->m_vertices) ||
//                           segment_cross_segment(c1, c2, f0, f1, input->m_vertices) ||
//                           segment_cross_segment(c1, c2, f1, f2, input->m_vertices) ||
//                           segment_cross_segment(c1, c2, f2, f0, input->m_vertices) ||
//                           segment_cross_segment(c2, c0, f0, f1, input->m_vertices) ||
//                           segment_cross_segment(c2, c0, f1, f2, input->m_vertices) ||
//                           segment_cross_segment(c2, c0, f2, f0, input->m_vertices))
//                        {
//                            search_neighbor = true;
//                            break;
//                        }
//                    }
//                    else
//                    {
//                        if(is_collinear(f0, f1, f2, input->m_vertices) || // I am just tired of handling the degenerate case
//                           segment_cross_triangle(c0, c1, f0, f1, f2, input->m_vertices) ||
//                           segment_cross_triangle(c1, c2, f0, f1, f2, input->m_vertices) ||
//                           segment_cross_triangle(c2, c0, f0, f1, f2, input->m_vertices) ||
//                           segment_cross_triangle(f0, f1, c0, c1, c2, input->m_vertices) ||
//                           segment_cross_triangle(f1, f2, c0, c1, c2, input->m_vertices) ||
//                           segment_cross_triangle(f2, f0, c0, c1, c2, input->m_vertices))
//                        {
//                            search_neighbor = true;
//                            break;
//                        }
//                    }
//                }
//            }
        }
    }

    GCoptimizationGeneralGraph gc((GCoptimization::SiteID)polyhedrons.size()+1, 2);
    // build graph and calculate min cut
    {
        // polyhedron cost
        gc.setDataCost((GCoptimization::SiteID)polyhedrons.size(), 1, 1.0); // ghost polyhedron
        for(uint32_t i=0; i<polyhedrons.size(); i++)
        {
            double out_area = 0.0; // area of facets point out
            double in_area = 0.0;
            
            for(uint32_t j=0; j<polyhedrons[i].size(); j++)
            {
                int orient = 0;
                uint32_t f = polyhedrons[i][j];
                uint32_t t = facets_centroids_mapping[f];
                if(UNDEFINED_VALUE == t)
                {
                    continue;
                }
                
                uint32_t t0 = input->m_constraints[3*t+0];
                uint32_t t1 = input->m_constraints[3*t+1];
                uint32_t t2 = input->m_constraints[3*t+2];
                for(uint32_t k=0; k<polyhedrons[i].size(); k++)
                {
                    if(k == j)
                    {
                        continue;
                    }
                    uint32_t nf = polyhedrons[i][k];
                    uint32_t f0 = facets[nf][0];
                    uint32_t f1 = facets[nf][1];
                    uint32_t f2 = facets[nf][2];
                    if(0 != (orient=orient3d(t0,t1,t2,f0, input->m_vertices)))
                    {
                        break;
                    }
                    if(0 != (orient=orient3d(t0,t1,t2,f1, input->m_vertices)))
                    {
                        break;
                    }
                    if(0 != (orient=orient3d(t0,t1,t2,f2, input->m_vertices)))
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
            gc.setDataCost((GCoptimization::SiteID)i, 1, 0.1*in_area);
        }
        
        // neighbor cost
        vector<bool> vertices_on_constrains = vector<bool>(input->m_vertices_count);
        for(uint32_t i=0; i<facets.size(); i++)
        {
            if(UNDEFINED_VALUE == facets_centroids_mapping[i])
            {
                continue;
            }
            for(uint32_t v : facets[i])
            {
                vertices_on_constrains[v] = true;
            }
        }
        for(uint32_t i=0; i<facets.size(); i++)
        {
            double w = 0.1;
            for(uint32_t v : facets[i])
            {
                if(!vertices_on_constrains[v])
                {
                    w = 1.0;
                    break;
                }
            }
            
            uint32_t n0,n1;
            polyhedralization.get_polyhedron_facet_neighbors(i, n0, n1);
            if(UNDEFINED_VALUE == n1)
            {
                n1 = polyhedrons.size();
            }
            gc.setNeighbors((GCoptimization::SiteID)n0, (GCoptimization::SiteID)n1, w*facets_approximated_areas[i]);
        }
        
        gc.swap();
    }
    
    // produce output
    {
        output->m_polyhedrons_labels = new uint32_t[polyhedrons.size()];
        for(uint32_t i=0; i<polyhedrons.size(); i++)
        {
            output->m_polyhedrons_labels[i] = gc.whatLabel((GCoptimization::SiteID)i);
        }
        
        m_vector_i_0.clear();
        for(uint32_t i=0; i<facets_vertices_mapping.size(); i++)
        {
            for(uint32_t j=0; j<facets_vertices_mapping[i].size(); j++)
            {
                m_vector_i_0.push_back(facets_vertices_mapping[i][j].size());
                for(uint32_t k=0; k<facets_vertices_mapping[i][j].size(); k++)
                {
                    m_vector_i_0.push_back(facets_vertices_mapping[i][j][k]);
                }
            }
        }
        output->m_facets_vertices_mapping = vector_to_array(m_vector_i_0);
        output->m_facets_centroids_mapping = vector_to_array(facets_centroids_mapping);
    }
}


InteriorCharacterizationHandle::InteriorCharacterizationHandle()
{
    m_input = new InteriorCharacterizationInput();
    m_input->m_vertices_count = 0;
    m_input->m_polyhedrons_count = 0;
    m_input->m_facets_count = 0;
    m_input->m_constraints_count = 0;
    m_input->m_vertices = nullptr;
    m_input->m_polyhedrons = nullptr;
    m_input->m_facets = nullptr;
    m_input->m_facets_centroids = nullptr;
    m_input->m_constraints = nullptr;
    m_output = new InteriorCharacterizationOutput();
    m_output->m_polyhedrons_labels = nullptr;
    m_output->m_facets_vertices_mapping = nullptr;
    m_output->m_facets_centroids_mapping = nullptr;
    m_interiorCharacterization = new InteriorCharacterization();
}
void InteriorCharacterizationHandle::Dispose()
{
    delete_vertices(m_input->m_vertices, m_input->m_vertices_count);
    delete[] m_input->m_polyhedrons;
    delete[] m_input->m_facets;
    delete_vertices(m_input->m_facets_centroids, m_input->m_facets_count);
    delete[] m_input->m_constraints;
    delete m_input;
    delete[] m_output->m_polyhedrons_labels;
    delete[] m_output->m_facets_vertices_mapping;
    delete[] m_output->m_facets_centroids_mapping;
    delete m_output;
    delete m_interiorCharacterization;
}

void InteriorCharacterizationHandle::AddInput(uint32_t explicit_count, double* explicit_values, uint32_t implicit_count, uint32_t* implicit_values, uint32_t polyhedrons_count, uint32_t* polyhedrons, uint32_t facets_count, uint32_t* facets, uint32_t* facets_centroids, double* facets_centroids_weights, uint32_t constraints_count, uint32_t* constraints)
{
    create_vertices(explicit_count, explicit_values, implicit_count, implicit_values, m_input->m_vertices, m_input->m_vertices_count);

    vector<uint32_t> vec = flat_array_to_vector(polyhedrons, polyhedrons_count);
    m_input->m_polyhedrons = vector_to_array(vec);
    m_input->m_polyhedrons_count = polyhedrons_count;
    
    vec =  flat_array_to_vector(facets, facets_count);
    m_input->m_facets = vector_to_array(vec);
    m_input->m_facets_centroids = new genericPoint*[facets_count];
    for(uint32_t i=0; i<facets_count; i++)
    {
        implicitPoint3D_BPT* p = new implicitPoint3D_BPT(m_input->m_vertices[facets_centroids[3*i+0]]->toExplicit3D(),
                                                         m_input->m_vertices[facets_centroids[3*i+1]]->toExplicit3D(),
                                                         m_input->m_vertices[facets_centroids[3*i+2]]->toExplicit3D(),
                                                         facets_centroids_weights[2*i+0],
                                                         facets_centroids_weights[2*i+1]);
        m_input->m_facets_centroids[i] = p;
    }
    m_input->m_facets_count = facets_count;
    
    m_input->m_constraints_count = constraints_count;
    m_input->m_constraints = duplicate_array(constraints, 3*constraints_count);
}

void InteriorCharacterizationHandle::Calculate()
{
    m_interiorCharacterization->interior_characterization(m_input, m_output);
}

uint32_t* InteriorCharacterizationHandle::GetPolyhedronsLabels()
{
    return m_output->m_polyhedrons_labels;
}
uint32_t* InteriorCharacterizationHandle::GetFacetsVerticesMapping()
{
    return m_output->m_facets_vertices_mapping;
}
uint32_t* InteriorCharacterizationHandle::GetFacetsCentroidsMapping()
{
    return m_output->m_facets_centroids_mapping;
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

extern "C" LIBRARY_EXPORT void AddInteriorCharacterizationInput(void* handle, uint32_t explicit_count, double* explicit_values, uint32_t implicit_count, uint32_t* implicit_values, uint32_t polyhedrons_count, uint32_t* polyhedrons, uint32_t facets_count, uint32_t* facets, uint32_t* facets_centroids, double* facets_centroids_weights, uint32_t constraints_count, uint32_t* constraints)
{
    ((InteriorCharacterizationHandle*)handle)->AddInput(explicit_count, explicit_values, implicit_count, implicit_values, polyhedrons_count, polyhedrons, facets_count, facets, facets_centroids, facets_centroids_weights, constraints_count, constraints);
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
