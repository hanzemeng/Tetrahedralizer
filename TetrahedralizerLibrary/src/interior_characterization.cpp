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
    
    vector<vector<uint32_t>> coplanar_constraints = vector<vector<uint32_t>>(coplanar_triangles.size());
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
        coplanar_constraints[cg].push_back(i);
    }
    vector<vector<uint32_t>> coplanar_facets = vector<vector<uint32_t>>(coplanar_triangles.size());
    for(uint32_t i=0; i<polyhedralization.m_facets.size(); i++)
    {
        uint32_t c0 = polyhedralization.m_facets[i].p0;
        uint32_t c1 = polyhedralization.m_facets[i].p1;
        uint32_t c2 = polyhedralization.m_facets[i].p2;
        if(UNDEFINED_VALUE == c0)
        {
            continue;
        }
        uint32_t cg = search_int(c0,c1,c2,triangles_coplanar_groups);
        coplanar_facets[cg].push_back(i);
    }
    
    times.push_back(chrono::steady_clock::now());
    
    uint32_t bvh_threshold=2048;
    vector<uint32_t> facets_centroids_mapping = vector<uint32_t>(polyhedralization.m_facets.size(), UNDEFINED_VALUE);
    for(uint32_t cg=0; cg<coplanar_triangles.size(); cg++)
    {
        if(coplanar_facets[cg].size()>1 && coplanar_facets[cg].size()*coplanar_constraints[cg].size()>bvh_threshold)
        {
            int ignore_axis = max_component_in_triangle_normal(coplanar_triangles[cg][0], coplanar_triangles[cg][1], coplanar_triangles[cg][2], polyhedralization.m_vertices.data());
            vector<uint32_t> constraints_triangles;
            constraints_triangles.reserve(3*coplanar_constraints[cg].size());
            for(uint32_t i=0; i<coplanar_constraints[cg].size(); i++)
            {
                uint32_t c = coplanar_constraints[cg][i];
                constraints_triangles.push_back(constraints[3*c+0]);
                constraints_triangles.push_back(constraints[3*c+1]);
                constraints_triangles.push_back(constraints[3*c+2]);
            }
            BoundingVolumeHierarchy2D bvh;
            bvh.build(approximated_vertices, ignore_axis, constraints_triangles);
            for(uint32_t f : coplanar_facets[cg])
            {
                uint32_t i = bvh.get_intersection(polyhedralization.m_vertices, ignore_axis, polyhedralization.m_segments, polyhedralization.m_facets[f], constraints_triangles);
                if(UNDEFINED_VALUE==i)
                {
                    continue;
                }
                facets_centroids_mapping[f] = coplanar_constraints[cg][i];
            }
        }
        else
        {
            for(uint32_t f : coplanar_facets[cg])
            {
                shared_ptr<genericPoint> centroid = polyhedralization.m_facets[f].get_implicit_centroid(polyhedralization.m_vertices);
                for(uint32_t c : coplanar_constraints[cg])
                {
                    uint32_t c0 = constraints[3*c+0];
                    uint32_t c1 = constraints[3*c+1];
                    uint32_t c2 = constraints[3*c+2];
                    if(genericPoint::pointInTriangle(*centroid,*polyhedralization.m_vertices[c0],*polyhedralization.m_vertices[c1],*polyhedralization.m_vertices[c2]))
                    {
                        facets_centroids_mapping[f] = c;
                        break;
                    }
                }
            }
        }
    }
    times.push_back(chrono::steady_clock::now());

    GCoptimizationGeneralGraph gc((GCoptimization::SiteID)polyhedralization.m_polyhedrons.size()+1, 2);
    {
        vector<vector<uint32_t>> neighbor_graph = vector<vector<uint32_t>>(polyhedralization.m_polyhedrons.size()+1);
        vector<double> polyhedrons_to_ghost_weight = vector<double>(polyhedralization.m_polyhedrons.size(), -1.0); // total area of facets connected to the ghost polyhedron, negative means not connectd
        vector<double> polyhedrons_in_costs = vector<double>(polyhedralization.m_polyhedrons.size(), 0.0);
        vector<double> polyhedrons_out_costs = vector<double>(polyhedralization.m_polyhedrons.size(), 0.0);
        for(uint32_t i=0; i<polyhedralization.m_facets.size(); i++)
        {
            uint32_t n0 = polyhedralization.m_facets[i].ip0;
            uint32_t n1 = polyhedralization.m_facets[i].ip1;
            if(n0 == UNDEFINED_VALUE)
            {
                swap(n0,n1);
            }
            
            if(UNDEFINED_VALUE == facets_centroids_mapping[i])
            {
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
                }
                else
                {
                    neighbor_graph[n0].push_back(n1);
                    neighbor_graph[n1].push_back(n0);
                    gc.setNeighbors((GCoptimization::SiteID)n0, (GCoptimization::SiteID)n1, facets_approximated_areas[i]);
                }
            }
            else
            {
                uint32_t c0 = constraints[3*facets_centroids_mapping[i]+0];
                uint32_t c1 = constraints[3*facets_centroids_mapping[i]+1];
                uint32_t c2 = constraints[3*facets_centroids_mapping[i]+2];
                uint32_t cg = search_int(c0, c1, c2, triangles_coplanar_groups);

                for(uint32_t f : polyhedralization.m_polyhedrons[n0])
                {
                    if(i==f)
                    {
                        continue;
                    }
                    uint32_t ncg = search_int(polyhedralization.m_facets[f].p0,polyhedralization.m_facets[f].p1,polyhedralization.m_facets[f].p2,triangles_coplanar_groups);
                    if(cg == ncg)
                    {
                        continue;
                    }
                    
                    int orient = 0;
                    vector<uint32_t> vs = polyhedralization.m_facets[f].get_vertices(polyhedralization.m_segments);
                    for(uint32_t v : vs)
                    {
                        if(0 != (orient=orient3d(c0,c1,c2,v, polyhedralization.m_vertices.data())))
                        {
                            break;
                        }
                    }
                    if(orient > 0)
                    {
                        polyhedrons_in_costs[n0] += facets_approximated_areas[f];
                        if(UNDEFINED_VALUE != n1)
                        {
                            polyhedrons_out_costs[n1] += facets_approximated_areas[f];
                        }
                    }
                    else if(orient < 0)
                    {
                        if(UNDEFINED_VALUE != n1)
                        {
                            polyhedrons_in_costs[n1] += facets_approximated_areas[f];
                        }
                        polyhedrons_out_costs[n0] += facets_approximated_areas[f];
                    }
                }
            }
        }
        for(uint32_t i=0; i<polyhedralization.m_polyhedrons.size(); i++)
        {
            if(polyhedrons_to_ghost_weight[i] > 0.0)
            {
                gc.setNeighbors((GCoptimization::SiteID)i, (GCoptimization::SiteID)polyhedralization.m_polyhedrons.size(), polyhedrons_to_ghost_weight[i]);
            }
        }
        
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
        
        gc.setDataCost((GCoptimization::SiteID)polyhedralization.m_polyhedrons.size(), 1, 1.0); // ghost polyhedron
        for(uint32_t i=0; i<polyhedralization.m_polyhedrons.size(); i++)
        {
            if(reachable_from_ghost[i])
            {
                gc.setDataCost((GCoptimization::SiteID)i, 0, polyhedrons_out_costs[i]);
                gc.setDataCost((GCoptimization::SiteID)i, 1, polyhedron_in_multiplier * polyhedrons_in_costs[i]);
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
