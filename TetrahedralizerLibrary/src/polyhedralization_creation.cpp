#include "polyhedralization_creation.hpp"
using namespace std;

void PolyhedralizationCreationHandle::calculate(vector<shared_ptr<genericPoint>>& vertices, vector<uint32_t>& constraints)
{
    cout << "constraints count: " << constraints.size()/3 << "\n";
    DelaunayTetrahedralizationHandle DT;
    auto t0 = chrono::steady_clock::now();
    Tetrahedralization tetrahedralization = DT.calculate(vertices);
    vector<uint32_t> convex_hull = tetrahedralization.get_bounding_facets();
    auto t1 = chrono::steady_clock::now();
    cout << chrono::duration_cast<std::chrono::milliseconds>(t1-t0).count() << "\n";
    cout << "convex hull tirangles: " << convex_hull.size()/3 << "\n";
    
    ConvexHullPartitionHandle CHP;
    auto [polyhedralization, approximated_vertices, coplanar_tirangles] = CHP.calculate(vertices, convex_hull, constraints);
    cout << "polyhedrons count: " << polyhedralization.m_polyhedrons.size() << "\n";
    
    InteriorCharacterizationHandle IC;
    vector<uint32_t> polyhedrons_labels = IC.calculate(polyhedralization, constraints, approximated_vertices, coplanar_tirangles, 0.1);
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
    cout << "inside polyhedrons count: " << polyhedralization.m_polyhedrons.size() << "\n";
    
//    vector<uint8_t> polyhedralization_bytes = polyhedralization.to_bytes();
//    ofstream out_file("test.txt", std::ios::binary);
//    out_file.write((char*)polyhedralization_bytes.data(), polyhedralization_bytes.size());
//    out_file.close();
}
