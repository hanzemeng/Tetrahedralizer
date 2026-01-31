#include "polyhedralization_creation.hpp"
using namespace std;

void PolyhedralizationCreationHandle::calculate(vector<shared_ptr<genericPoint>>& vertices, vector<uint32_t>& constraints)
{
    cout << "constraints count: " << constraints.size()/3 << "\n";
    DelaunayTetrahedralizationHandle DT;
    Tetrahedralization tetrahedralization = DT.calculate(vertices);
    vector<uint32_t> convex_hull = tetrahedralization.get_bounding_facets();
    
    ConvexHullPartitionHandle CHP;
    auto [polyhedralization, approximated_vertices, coplanar_tirangles] = CHP.calculate(vertices, convex_hull, constraints);
    cout << "poly count: " << polyhedralization.m_polyhedrons.size() << "\n";
    
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
    
    vector<uint8_t> polyhedralization_bytes = polyhedralization.to_bytes();
    ofstream out_file("test.txt", std::ios::binary);
    out_file.write((char*)polyhedralization_bytes.data(), polyhedralization_bytes.size());
    out_file.close();
}
