#include "interior_characterization.hpp"
using namespace std;

std::vector<uint32_t> InteriorCharacterizationHandle::calculate(Polyhedralization& polyhedralization, std::vector<uint32_t>& constraints, std::vector<double3>& approximated_vertices)
{
    vector<chrono::steady_clock::time_point> times;
    times.push_back(chrono::steady_clock::now());

    vector<double> facets_areas;
    facets_areas.reserve(polyhedralization.m_facets.size());
    {
        double total_area = 0.0;
        for(uint32_t i=0; i<polyhedralization.m_facets.size(); i++)
        {
            vector<uint32_t> vs = polyhedralization.m_facets[i].get_sorted_vertices(polyhedralization.m_segments);
            double area = 0.0;
            double3 p0 = approximated_vertices[vs[0]];
            for(uint32_t j=1; j<vs.size()-1; j++)
            {
                double3 p1 = approximated_vertices[vs[j]];
                double3 p2 = approximated_vertices[vs[j+1]];
                area += (p1-p0).cross(p2-p0).length();
            }
            facets_areas.push_back(area);
            total_area += area;
        }
        for(uint32_t i=0; i<facets_areas.size(); i++)
        {
            facets_areas[i] /= total_area;
        }
    }
    
    vector<uint32_t> valid_constraints;
    valid_constraints.reserve(constraints.size());
    for(uint32_t i=0; i<constraints.size(); i++)
    {
        if(UNDEFINED_VALUE == constraints[i])
        {
            continue;
        }
        valid_constraints.push_back(constraints[i]);
    }
    WindingNumberApproximation WNA = WindingNumberApproximation(approximated_vertices, valid_constraints);
    
    vector<double3> polyhedrons_centroids;
    polyhedrons_centroids.reserve(polyhedralization.m_polyhedrons.size());
    vector<uint32_t> vertices_cache = vector<uint32_t>(polyhedralization.m_vertices.size(), UNDEFINED_VALUE);
    for(uint32_t i=0; i<polyhedralization.m_polyhedrons.size(); i++)
    {
        double3 centroid(0.0,0.0,0.0);
        uint32_t vertices_count = 0;
        for(uint32_t j=0; j<polyhedralization.m_polyhedrons[i].size(); j++)
        {
            uint32_t f = polyhedralization.m_polyhedrons[i][j];
            polyhedralization.m_facets[f].get_vertices(polyhedralization.m_segments, Facet::m_get_vertices_res);
            
            for(uint32_t v : Facet::m_get_vertices_res)
            {
                if(i != vertices_cache[v])
                {
                    vertices_cache[v] = i;
                    centroid += approximated_vertices[v];
                    vertices_count++;
                }
            }
        }
        centroid /= (double)vertices_count;
        polyhedrons_centroids.push_back(centroid);
    }
    
    vector<double> polyhedrons_winding_numbers = vector<double>(polyhedralization.m_polyhedrons.size());
    if(polyhedrons_winding_numbers.size()<1024)
    {
        for(uint32_t j=0; j<polyhedrons_winding_numbers.size(); j++)
        {
            polyhedrons_winding_numbers[j] = WNA.compute(polyhedrons_centroids[j], approximated_vertices, valid_constraints);
        }
    }
    else
    {
        uint32_t num_threads = std::thread::hardware_concurrency();
        if(num_threads == 0)
        {
            num_threads = 2;
        }
        std::vector<std::thread> threads;
        uint32_t chunk_size = polyhedrons_winding_numbers.size() / num_threads;

        for(uint32_t i=0; i<num_threads; i++)
        {
            uint32_t start = i*chunk_size;
            uint32_t end = (i == num_threads-1) ? polyhedrons_winding_numbers.size() : (i+1)*chunk_size;
            threads.emplace_back([start, end, &WNA, &approximated_vertices, &valid_constraints, &polyhedrons_centroids, &polyhedrons_winding_numbers]()
                                {
                                    for(uint32_t j=start; j<end; j++)
                                    {
                                        polyhedrons_winding_numbers[j] = WNA.compute(polyhedrons_centroids[j], approximated_vertices, valid_constraints);
                                    }
                                });
        }

        for(auto& t : threads)
        {
            t.join();
        }
    }
    
    times.push_back(chrono::steady_clock::now());
    
    GCoptimizationGeneralGraph gc((GCoptimization::SiteID)polyhedralization.m_polyhedrons.size(), 2);
    for(uint32_t i=0; i<polyhedralization.m_facets.size(); i++)
    {
        uint32_t p0 = polyhedralization.m_facets_incident_polyhedrons[2*i+0];
        uint32_t p1 = polyhedralization.m_facets_incident_polyhedrons[2*i+1];
        if(UNDEFINED_VALUE==p0 || UNDEFINED_VALUE==p1)
        {
            continue;
        }
        
        double w0 = polyhedrons_winding_numbers[p0];
        double w1 = polyhedrons_winding_numbers[p1];
        double cost = facets_areas[i] * exp(-1*(w0-w1)*(w0-w1));
        gc.setNeighbors((GCoptimization::SiteID)p0, (GCoptimization::SiteID)p1, cost);
    }
    for(uint32_t i=0; i<polyhedralization.m_polyhedrons.size(); i++)
    {
        gc.setDataCost((GCoptimization::SiteID)i, 0, max(polyhedrons_winding_numbers[i]-0.0, 0.0));
        gc.setDataCost((GCoptimization::SiteID)i, 1, max(1.0-polyhedrons_winding_numbers[i], 0.0));
    }
    gc.swap();
    
    vector<uint32_t> polyhedrons_labels = vector<uint32_t>(polyhedralization.m_polyhedrons.size());
    for(uint32_t i=0; i<polyhedralization.m_polyhedrons.size(); i++)
    {
        polyhedrons_labels[i] = gc.whatLabel((GCoptimization::SiteID)i);
    }
    times.push_back(chrono::steady_clock::now());
    
    for (uint32_t i=1; i<times.size(); i++)
    {
        cerr << chrono::duration_cast<std::chrono::milliseconds>(times[i] - times[i-1]).count() << "\n";
    }
    
    return polyhedrons_labels;
}
