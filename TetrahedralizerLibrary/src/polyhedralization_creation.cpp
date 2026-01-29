#include "polyhedralization_creation.hpp"
using namespace std;

void PolyhedralizationCreationHandle::calculate(vector<shared_ptr<genericPoint>>& vertices, vector<uint32_t>& constraints)
{
    DelaunayTetrahedralizationHandle DT;
    Tetrahedralization tetrahedralization = DT.calculate(vertices);
    vector<uint32_t> convex_hull = tetrahedralization.get_bounding_facets();
    
    ConvexHullPartitionHandle CHP;
    Polyhedralization polyhedralization = CHP.calculate(vertices, convex_hull, constraints);
    cout << polyhedralization.m_polyhedrons.size() << "\n";
}
