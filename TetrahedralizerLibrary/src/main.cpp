#include "delaunay_tetrahedralization.hpp"
#include "binary_space_partition.hpp"
#include "convex_hull_partition.hpp"
#include "interior_characterization.hpp"

using namespace std;

int main(int argc, const char * argv[])
{
    if(argc < 2)
    {
        cerr << "Usage: Tetrahedralizer in_file\n";
        return 1;
    }
    ifstream in_file(argv[1]);
    if(!in_file.is_open())
    {
        std::cerr << "Fail to open " << argv[1] << '\n';
        return 1;
    }
    
    string file_type;
    in_file >> file_type;
    uint32_t not_used;
    
    vector<double> explicit_vertices;
    vector<uint32_t> constraints;
    {
        uint32_t vn, cn;
        in_file >> vn >> cn >> not_used;
        explicit_vertices.reserve(3*vn);
        constraints.reserve(3*cn);
        for(uint32_t i=0; i<vn; i++)
        {
            double x,y,z;
            in_file >> x >> y >> z;
            explicit_vertices.push_back(x);
            explicit_vertices.push_back(y);
            explicit_vertices.push_back(z);
        }
        
        for(uint32_t i=0; i<cn; i++)
        {
            uint32_t c0,c1,c2;
            in_file >> not_used;
            in_file >> c0 >> c1 >> c2;
            constraints.push_back(c0);
            constraints.push_back(c1);
            constraints.push_back(c2);
        }
    }
    in_file.close();
    
    void* DT = CreateDelaunayTetrahedralizationHandle();
    void* BSP = CreateBinarySpacePartitionHandle();
    void* IC0 = CreateInteriorCharacterizationHandle();
    void* CHP = CreateConvexHullPartitionHandle();
    void* IC1 = CreateInteriorCharacterizationHandle();
    
    vector<chrono::steady_clock::time_point> times;
    
    AddDelaunayTetrahedralizationInput(DT, explicit_vertices.size()/3, explicit_vertices.data(), 0, nullptr);
    times.push_back(chrono::steady_clock::now());
    CalculateDelaunayTetrahedralization(DT);
    times.push_back(chrono::steady_clock::now());
    vector<uint32_t> tetrahedrons(GetDelaunayTetrahedralizationTetrahedronsCount(DT));
    GetDelaunayTetrahedralizationTetrahedrons(DT, tetrahedrons.data());
    
    AddBinarySpacePartitionInput(BSP, explicit_vertices.size()/3, explicit_vertices.data(), tetrahedrons.size()/4, tetrahedrons.data(), constraints.size()/3, constraints.data());
    times.push_back(chrono::steady_clock::now());
    CalculateBinarySpacePartition(BSP);
    times.push_back(chrono::steady_clock::now());
    vector<uint32_t> BSP_vertices(GetBinarySpacePartitionInsertedVerticesCount(BSP));
    vector<uint32_t> BSP_polyhedrons(GetBinarySpacePartitionPolyhedronsCount(BSP));
    vector<FacetInteropData> BSP_facets(GetBinarySpacePartitionFacetsCount(BSP));
    vector<SegmentInteropData> BSP_segments(GetBinarySpacePartitionSegmentsCount(BSP));
    GetBinarySpacePartitionInsertedVertices(BSP, BSP_vertices.data());
    GetBinarySpacePartitionPolyhedrons(BSP, BSP_polyhedrons.data());
    GetBinarySpacePartitionFacets(BSP, BSP_facets.data());
    GetBinarySpacePartitionSegments(BSP, BSP_segments.data());
    vector<uint32_t> coplanar_groups(GetBinarySpacePartitionCoplanarTrianglesCount(BSP));
    GetBinarySpacePartitionCoplanarTriangles(BSP, coplanar_groups.data());
    
    AddInteriorCharacterizationInput(IC0, explicit_vertices.size()/3, explicit_vertices.data(), count_flat_vector_elements(BSP_vertices), BSP_vertices.data(),
                                     count_flat_vector_elements(BSP_polyhedrons), BSP_polyhedrons.data(), BSP_facets.size(), BSP_facets.data(), BSP_segments.size(), BSP_segments.data(),
                                     count_flat_vector_elements(coplanar_groups), coplanar_groups.data(), constraints.size()/3, constraints.data(), 0.1);
    times.push_back(chrono::steady_clock::now());
    CalculateInteriorCharacterization(IC0);
    times.push_back(chrono::steady_clock::now());
    vector<uint32_t> polyhedrons_labels(count_flat_vector_elements(BSP_polyhedrons));
    GetInteriorCharacterizationPolyhedronsLabels(IC0, polyhedrons_labels.data());
    
    vector<FacetInteropData> BSP_constraints_facets;
    {
        vector<vector<uint32_t>> BSP_nested_polyhedrons = flat_array_to_nested_vector(BSP_polyhedrons.data(), count_flat_vector_elements(BSP_polyhedrons));
        vector<uint32_t> BSP_facets_occurance(BSP_facets.size(), 0);
        
        for(uint32_t i=0; i<BSP_nested_polyhedrons.size(); i++)
        {
            if(1 != polyhedrons_labels[i])
            {
                continue;
            }
            for(uint32_t f : BSP_nested_polyhedrons[i])
            {
                BSP_facets_occurance[f]++;
            }
        }
        for(uint32_t i=0; i<BSP_facets.size(); i++)
        {
            if(1 != BSP_facets_occurance[i])
            {
                continue;
            }
            if(UNDEFINED_VALUE==BSP_facets[i].ip0||UNDEFINED_VALUE==BSP_facets[i].ip1)
            {
                continue;
            }
            BSP_constraints_facets.push_back(BSP_facets[i]);
        }
    }
    
    AddConvexHullPartitionInput(CHP, explicit_vertices.size()/3, explicit_vertices.data(),  count_flat_vector_elements(BSP_vertices), BSP_vertices.data(),
                                tetrahedrons.size()/4, tetrahedrons.data(), BSP_constraints_facets.size(), BSP_constraints_facets.data(), BSP_segments.size(), BSP_segments.data(),
                                count_flat_vector_elements(coplanar_groups), coplanar_groups.data());
    times.push_back(chrono::steady_clock::now());
    CalculateConvexHullPartition(CHP);
    times.push_back(chrono::steady_clock::now());
    vector<uint32_t> CHP_vertices(GetConvexHullPartitionInsertedVerticesCount(CHP));
    vector<uint32_t> CHP_polyhedrons(GetConvexHullPartitionPolyhedronsCount(CHP));
    vector<FacetInteropData> CHP_facets(GetConvexHullPartitionFacetsCount(CHP));
    vector<SegmentInteropData> CHP_segments(GetConvexHullPartitionSegmentsCount(CHP));
    GetConvexHullPartitionInsertedVertices(CHP, CHP_vertices.data());
    GetConvexHullPartitionPolyhedrons(CHP, CHP_polyhedrons.data());
    GetConvexHullPartitionFacets(CHP, CHP_facets.data());
    GetConvexHullPartitionSegments(CHP, CHP_segments.data());
    
    
    BSP_vertices.insert(BSP_vertices.end(), CHP_vertices.begin(),CHP_vertices.end());
    AddInteriorCharacterizationInput(IC1, explicit_vertices.size()/3, explicit_vertices.data(), count_flat_vector_elements(BSP_vertices), BSP_vertices.data(),
                                     count_flat_vector_elements(CHP_polyhedrons), CHP_polyhedrons.data(), CHP_facets.size(), CHP_facets.data(), CHP_segments.size(), CHP_segments.data(),
                                     count_flat_vector_elements(coplanar_groups), coplanar_groups.data(), constraints.size()/3, constraints.data(), 0.1);
    times.push_back(chrono::steady_clock::now());
    CalculateInteriorCharacterization(IC1);
    times.push_back(chrono::steady_clock::now());
    
    polyhedrons_labels = vector<uint32_t>(count_flat_vector_elements(CHP_polyhedrons));
    GetInteriorCharacterizationPolyhedronsLabels(IC1, polyhedrons_labels.data());
    
    DisposeDelaunayTetrahedralizationHandle(DT);
    DisposeBinarySpacePartitionHandle(BSP);
    DisposeInteriorCharacterizationHandle(IC0);
    DisposeConvexHullPartitionHandle(CHP);
    DisposeInteriorCharacterizationHandle(IC1);
    
    string out_file_name(argv[1]);
    out_file_name.replace(out_file_name.size()-3, 3, "txt");
    
    ofstream out_file(out_file_name);
    if(!out_file.is_open())
    {
        std::cerr << "Fail to create output file\n";
        return 1;
    }
    
    out_file << "Poly Count: " << count_flat_vector_elements(CHP_polyhedrons) << "\n";
    for (uint32_t i=1; i<times.size(); i+=2)
    {
        out_file << chrono::duration_cast<std::chrono::milliseconds>(times[i] - times[i-1]).count() << "\n";
    }
    
//    out_file << explicit_vertices.size() << "\n";
//    for(uint32_t i=0; i<explicit_vertices.size(); i++)
//    {
//        out_file << explicit_vertices[i] << " ";
//    }
//    out_file << "\n";
//    
//    out_file << BSP_vertices.size() << "\n";
//    for(uint32_t i=0; i<BSP_vertices.size(); i++)
//    {
//        out_file << BSP_vertices[i] << " ";
//    }
//    
//    vector<vector<uint32_t>> CHP_nested_polyhedrons = flat_array_to_nested_vector(CHP_polyhedrons.data(), count_flat_vector_elements(CHP_polyhedrons));
//    vector<uint32_t> out_polyhedrons;
//    for(uint32_t i=0; i<polyhedrons_labels.size(); i++)
//    {
//
//    }
    
    out_file.close();
    return 0;
}
