#include "polyhedralization_creation.hpp"
using namespace std;

Polyhedralization PolyhedralizationCreationHandle::calculate(vector<shared_ptr<genericPoint>>& vertices, vector<uint32_t>& constraints)
{
    cerr << "constraints count: " << constraints.size()/3 << "\n";
    auto t0 = chrono::steady_clock::now();
    vector<uint32_t> convex_hull;
    {
        vector<double3> approximated_vertices;
        approximate_verteices(approximated_vertices, vertices);
        Quickhull QH;
        convex_hull = QH.calculate(vertices, approximated_vertices);
    }
    auto t1 = chrono::steady_clock::now();
    cerr << chrono::duration_cast<std::chrono::milliseconds>(t1-t0).count() << "\n";
    cerr << "convex hull triangles: " << convex_hull.size()/3 << "\n";
    
    tuple<Polyhedralization, std::vector<double3>, std::vector<std::vector<uint32_t>>> CHP_res;
    {
        ConvexHullPartitionHandle CHP;
        CHP_res = CHP.calculate(vertices, convex_hull, constraints);
    }
    auto& [polyhedralization, approximated_vertices, coplanar_tirangles] = CHP_res;
    vector<uint32_t>().swap(polyhedralization.m_slice_facets_cache);
    vector<std::tuple<uint32_t,uint32_t,uint32_t,uint32_t>>().swap(polyhedralization.m_slice_segments_cache);
    vector<std::pair<uint32_t,int>>().swap(polyhedralization.m_slice_vertices_cache);
    vector<std::vector<uint32_t>>().swap(polyhedralization.m_segments_incident_facets);
    
    cerr << "polyhedrons count: " << polyhedralization.m_polyhedrons.size() << "\n";
    polyhedralization.clear_auxiliary_data();
    
    if(polyhedralization.m_polyhedrons.size()>1)
    {
        vector<uint32_t> polyhedrons_labels;
        {
            InteriorCharacterizationHandle IC;
            polyhedrons_labels = IC.calculate(polyhedralization, constraints, approximated_vertices);
        }
        vector<vector<uint32_t>> new_polyhedrons;
        for(uint32_t i=0; i<polyhedrons_labels.size(); i++)
        {
            if(1 != polyhedrons_labels[i])
            {
                continue;
            }
            new_polyhedrons.push_back(std::move(polyhedralization.m_polyhedrons[i]));
        }
        polyhedralization.m_polyhedrons = new_polyhedrons;
    }

    cerr << "inside polyhedrons count: " << polyhedralization.m_polyhedrons.size() << "\n";
    return polyhedralization;
}
