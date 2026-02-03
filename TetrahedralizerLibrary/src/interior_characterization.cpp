#include "interior_characterization.hpp"
using namespace std;

std::vector<uint32_t> InteriorCharacterizationHandle::calculate(Polyhedralization& polyhedralization, std::vector<uint32_t>& constraints, std::vector<double3>& approximated_vertices, std::vector<std::vector<uint32_t>>& coplanar_triangles, double polyhedron_in_multiplier)
{
    vector<chrono::steady_clock::time_point> times;
    times.push_back(chrono::steady_clock::now());
    
    unordered_map<tuple<uint32_t,uint32_t,uint32_t>, uint32_t,iii32_hash> triangles_coplanar_groups;
    for(uint32_t i=0; i<coplanar_triangles.size(); i++)
    {
        for(uint32_t j=0; j<coplanar_triangles[i].size()/3; j++)
        {
            uint32_t c0 = coplanar_triangles[i][3*j+0];
            uint32_t c1 = coplanar_triangles[i][3*j+1];
            uint32_t c2 = coplanar_triangles[i][3*j+2];
            assign_int(c0, c1, c2, i, triangles_coplanar_groups);
        }
    }
    
    vector<double> facets_approximated_areas;
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
            facets_approximated_areas.push_back(area);
            total_area += area;
        }
        for(uint32_t i=0; i<facets_approximated_areas.size(); i++)
        {
            facets_approximated_areas[i] /= total_area;
        }
    }
    
    vector<vector<uint32_t>> constraints_coplanar_groups = vector<vector<uint32_t>>(coplanar_triangles.size());
    for(uint32_t i=0; i<constraints.size()/3; i++)
    {
        uint32_t c0 = constraints[3*i+0];
        uint32_t c1 = constraints[3*i+1];
        uint32_t c2 = constraints[3*i+2];
        if(UNDEFINED_VALUE == c0)
        {
            continue;
        }
        uint32_t cg = search_int(c0,c1,c2,triangles_coplanar_groups);
        constraints_coplanar_groups[cg].push_back(i);
    }
    
    times.push_back(chrono::steady_clock::now());
    
    vector<uint32_t> facets_centroids_mapping = vector<uint32_t>(polyhedralization.m_facets.size(), UNDEFINED_VALUE);
    for(uint32_t i=0; i<polyhedralization.m_facets.size(); i++)
    {
        uint32_t cg = search_int(polyhedralization.m_facets[i].p0, polyhedralization.m_facets[i].p1, polyhedralization.m_facets[i].p2,triangles_coplanar_groups);

        shared_ptr<genericPoint> centroid = polyhedralization.m_facets[i].get_implicit_centroid(polyhedralization.m_vertices);
//        if(constraints_coplanar_groups[cg].size() > 1)
//        {
//            cout << constraints_coplanar_groups[cg].size() << "\n";
//        }
        
        for(uint32_t j=0; j<constraints_coplanar_groups[cg].size(); j++)
        {
            uint32_t c = constraints_coplanar_groups[cg][j];
            uint32_t c0 = constraints[3*c+0];
            uint32_t c1 = constraints[3*c+1];
            uint32_t c2 = constraints[3*c+2];
            if(genericPoint::pointInTriangle(*centroid,*polyhedralization.m_vertices[c0],*polyhedralization.m_vertices[c1],*polyhedralization.m_vertices[c2]))
            {
                facets_centroids_mapping[i] = c;
                break;
            }
        }
    }
    times.push_back(chrono::steady_clock::now());

    GCoptimizationGeneralGraph gc((GCoptimization::SiteID)polyhedralization.m_polyhedrons.size()+1, 2);
    {
        // neighbor cost and neighbor graph
        vector<vector<uint32_t>> neighbor_graph = vector<vector<uint32_t>>(polyhedralization.m_polyhedrons.size()+1);
        vector<double> polyhedrons_to_ghost_weight = vector<double>(polyhedralization.m_polyhedrons.size(), -1.0); // total area of facets connected to the ghost polyhedron, negative means not connectd
        for(uint32_t i=0; i<polyhedralization.m_facets.size(); i++)
        {
            if(UNDEFINED_VALUE != facets_centroids_mapping[i])
            {
                continue;
            }

            uint32_t n0 = polyhedralization.m_facets[i].ip0;
            uint32_t n1 = polyhedralization.m_facets[i].ip1;
            if(n0 == UNDEFINED_VALUE)
            {
                swap(n0,n1);
            }

            if(UNDEFINED_VALUE == n1)
            {
                neighbor_graph[n0].push_back(polyhedralization.m_polyhedrons.size());
                neighbor_graph[polyhedralization.m_polyhedrons.size()].push_back(n0);
                
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
        for(uint32_t i=0; i<polyhedralization.m_polyhedrons.size(); i++)
        {
            if(polyhedrons_to_ghost_weight[i] > 0.0)
            {
                gc.setNeighbors((GCoptimization::SiteID)i, (GCoptimization::SiteID)polyhedralization.m_polyhedrons.size(), polyhedrons_to_ghost_weight[i]);
            }
        }
        
        // check polygons that can be reached from ghost
        vector<bool> reachable_from_ghost = vector<bool>(polyhedralization.m_polyhedrons.size()+1,false);
        {
            queue<uint32_t> visit_polyhedrons;
            visit_polyhedrons.push(polyhedralization.m_polyhedrons.size());
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
        times.push_back(chrono::steady_clock::now());
        
        // polyhedron cost
        vector<vector<uint32_t>> facets_vertices = vector<vector<uint32_t>>(polyhedralization.m_facets.size());
        for(uint32_t i=0; i<polyhedralization.m_facets.size(); i++)
        {
            facets_vertices[i] = polyhedralization.m_facets[i].get_vertices(polyhedralization.m_segments);
        }
        gc.setDataCost((GCoptimization::SiteID)polyhedralization.m_polyhedrons.size(), 1, 1.0); // ghost polyhedron
        for(uint32_t i=0; i<polyhedralization.m_polyhedrons.size(); i++)
        {
            double out_area = 0.0; // area of facets point out
            double in_area = 0.0;
            
            for(uint32_t j=0; j<polyhedralization.m_polyhedrons[i].size(); j++)
            {
                uint32_t f = polyhedralization.m_polyhedrons[i][j];
                uint32_t c = facets_centroids_mapping[f];
                if(UNDEFINED_VALUE == c)
                {
                    continue;
                }
                uint32_t t0 = constraints[3*c+0];
                uint32_t t1 = constraints[3*c+1];
                uint32_t t2 = constraints[3*c+2];
                
                int orient = 0;
                for(uint32_t k=0; k<polyhedralization.m_polyhedrons[i].size(); k++)
                {
                    if(k == j)
                    {
                        continue;
                    }
                    uint32_t nf = polyhedralization.m_polyhedrons[i][k];
                    for(uint32_t v : facets_vertices[nf])
                    {
                        if(0 != (orient=orient3d(t0,t1,t2,v, polyhedralization.m_vertices.data())))
                        {
                            goto HAS_ORIENT;
                        }
                    }
                }
                HAS_ORIENT:
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
                gc.setDataCost((GCoptimization::SiteID)i, 1, polyhedron_in_multiplier * in_area);
            }
            else
            {
                gc.setDataCost((GCoptimization::SiteID)i, 0, 1.0); // the polyhedron has to be in if it can't be reached from the ghost
                gc.setDataCost((GCoptimization::SiteID)i, 1, 0.0);
            }
        }
        times.push_back(chrono::steady_clock::now());
        gc.swap();
        times.push_back(chrono::steady_clock::now());
    }
    
    vector<uint32_t> polyhedrons_labels = vector<uint32_t>(polyhedralization.m_polyhedrons.size());
    for(uint32_t i=0; i<polyhedralization.m_polyhedrons.size(); i++)
    {
        polyhedrons_labels[i] = gc.whatLabel((GCoptimization::SiteID)i);
    }
    
    for (uint32_t i=1; i<times.size(); i++)
    {
        cout << chrono::duration_cast<std::chrono::milliseconds>(times[i] - times[i-1]).count() << "\n";
    }
    
    return polyhedrons_labels;
}
