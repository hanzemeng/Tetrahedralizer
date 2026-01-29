#include "polyhedralization_creation.hpp"
using namespace std;

void PolyhedralizationCreationHandle::calculate(vector<shared_ptr<genericPoint>>& vertices, vector<uint32_t>& constraints)
{
    cout << "constraints count: " << constraints.size()/3 << "\n";
    DelaunayTetrahedralizationHandle DT;
    Tetrahedralization tetrahedralization = DT.calculate(vertices);
    vector<uint32_t> convex_hull = tetrahedralization.get_bounding_facets();
    
    ConvexHullPartitionHandle CHP;
    Polyhedralization polyhedralization = CHP.calculate(vertices, convex_hull, constraints);
    cout << "poly count: " << polyhedralization.m_polyhedrons.size() << "\n";
    
//    vector<uint8_t> polyhedralization_bytes = polyhedralization.to_bytes();
//    ofstream out_file("test.txt", std::ios::binary);
//    out_file.write((char*)polyhedralization_bytes.data(), polyhedralization_bytes.size());
//    out_file.close();
}
