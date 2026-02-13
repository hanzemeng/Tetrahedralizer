#include "delaunay_tetrahedralization.hpp"
#include "convex_hull_partition.hpp"
#include "interior_characterization.hpp"
#include "polyhedralization_creation.hpp"

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
    
    vector<double> input_vertices;
    vector<uint32_t> input_constraints;
    {
        using point = tuple<double, double, double>;
        map<point, uint32_t> unique_vertices;
        unordered_map<uint32_t, uint32_t> duplicate_indices; // duplicate -> original
        uint32_t index = 0; // tracks how many unique vertices we've encountered
        
        uint32_t vn, cn;
        in_file >> vn >> cn >> not_used;
        input_vertices.reserve(3*vn);
        input_constraints.reserve(3*cn);
        for(uint32_t i=0; i<vn; i++)
        {
            // If we have already seen this point, don't add to vertex vector and track relation
            point p;
            in_file >> get<0>(p) >> get<1>(p) >> get<2>(p);
            auto it = unique_vertices.find(p);
            if (it==unique_vertices.end()) {
                // New point
                unique_vertices[p] = index; 
                duplicate_indices[i] = index; // map original indices to themselves
                index++;
            } else {
                duplicate_indices[i] = it->second;
                continue;
            }
            input_vertices.push_back(get<0>(p));
            input_vertices.push_back(get<1>(p));
            input_vertices.push_back(get<2>(p));
        }
        
        for(uint32_t i=0; i<cn; i++)
        {
            uint32_t point_count,c0,c1,c2;
            in_file >> point_count;
            if (point_count != 3) {
                cerr << "Input constraint not a triangle\n";
                return 1;
            }
            in_file >> c0 >> c1 >> c2;
            input_constraints.push_back(duplicate_indices[c0]);
            input_constraints.push_back(duplicate_indices[c1]);
            input_constraints.push_back(duplicate_indices[c2]);
        }
    }
    in_file.close();
    

    PolyhedralizationCreationHandle PC;
    vector<shared_ptr<genericPoint>> vertices = create_vertices(input_vertices.size()/3, input_vertices.data(), 0, nullptr);
    vector<uint32_t> constraints = create_constraints(input_constraints.size()/3, input_constraints.data(), vertices.data(), true);
    PC.calculate(vertices, constraints);
    return 0;
}
