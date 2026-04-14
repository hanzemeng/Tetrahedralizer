#include "polyhedralization_creation.hpp"

using namespace std;

pair<vector<double>, vector<uint32_t>> read_input(istream& in_file)
{
    string file_type;
    in_file >> file_type;
    uint32_t not_used;
    vector<double> input_vertices;
    vector<uint32_t> input_constraints;
    {
        using point = tuple<double, double, double>;
        uint32_t vn, cn;
        in_file >> vn >> cn >> not_used;
        map<point, uint32_t> unique_vertices;
        vector<uint32_t> vertices_mapping(vn);
        input_vertices.reserve(3*vn);
        input_constraints.reserve(3*cn);
        
        for(uint32_t i=0; i<vn; i++)
        {
            point p;
            in_file >> get<0>(p) >> get<1>(p) >> get<2>(p);
            auto it = unique_vertices.find(p);
            if(it==unique_vertices.end())
            {
                vertices_mapping[i] = unique_vertices.size();
                unique_vertices[p] = unique_vertices.size();
            }
            else
            {
                vertices_mapping[i] = it->second;
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
            if(point_count != 3)
            {
                throw "Input constraint not a triangle";
            }
            in_file >> c0 >> c1 >> c2;
            input_constraints.push_back(vertices_mapping[c0]);
            input_constraints.push_back(vertices_mapping[c1]);
            input_constraints.push_back(vertices_mapping[c2]);
        }
    }
    
    return make_pair(input_vertices, input_constraints);
}

pair<vector<double>, vector<uint32_t>> read_input_binary(istream& in_file)
{
    uint8_t header[3];
    if(!in_file.read((char*)header, 3))
    {
        throw "Failed to read header";
    }

    uint32_t header_info[3];
    if(!in_file.read((char*)(header_info), 12))
    {
        throw "Failed to read mesh dimensions";
    }
    
    uint32_t vn = header_info[0];
    uint32_t cn = header_info[1];
    // header_info[2] is not_used

    using point = tuple<double, double, double>;
    map<point, uint32_t> unique_vertices;
    vector<uint32_t> vertices_mapping(vn);
    vector<double> input_vertices;
    vector<uint32_t> input_constraints;
    input_vertices.reserve(3 * vn);
    input_constraints.reserve(3 * cn);

    for(uint32_t i=0; i<vn; i++)
    {
        double coords[3];
        in_file.read((char*)(coords), 24);
        point p{coords[0], coords[1], coords[2]};

        auto [it, inserted] = unique_vertices.try_emplace(p, unique_vertices.size());
        vertices_mapping[i] = it->second;
        if (inserted)
        {
            input_vertices.push_back(coords[0]);
            input_vertices.push_back(coords[1]);
            input_vertices.push_back(coords[2]);
        }
    }

    for(uint32_t i=0; i<cn; i++)
    {
        uint32_t face_data[4]; // [point_count, c0, c1, c2]
        in_file.read((char*)(face_data), 16);

        if(face_data[0] != 3)
        {
            throw "Input constraint not a triangle";
        }

        input_constraints.push_back(vertices_mapping[face_data[1]]);
        input_constraints.push_back(vertices_mapping[face_data[2]]);
        input_constraints.push_back(vertices_mapping[face_data[3]]);
    }

    return make_pair(input_vertices, input_constraints);
}

int main(int argc, const char * argv[])
{
    if(argc >= 2)
    {
        ifstream in_file(argv[1]);
        auto input = read_input(in_file);
        in_file.close();
        
        auto [input_vertices, input_constraints] = input;
        PolyhedralizationCreationHandle PC;
        vector<shared_ptr<genericPoint>> vertices = create_vertices(input_vertices.size()/3, input_vertices.data(), 0, nullptr);
        vector<uint32_t> constraints = create_constraints(input_constraints.size()/3, input_constraints.data(), vertices.data(), true);
        Polyhedralization polyhedralization = PC.calculate(vertices, constraints);

        
        polyhedralization.calculate_facets_incident_polyhedrons();
        uint32_t facets_count = 0;
        for(uint32_t i=0; i<polyhedralization.m_facets.size(); i++)
        {
            uint32_t keep = 0;
            if(UNDEFINED_VALUE == polyhedralization.m_facets_incident_polyhedrons[2*i+0])
            {
                keep++;
            }
            if(UNDEFINED_VALUE == polyhedralization.m_facets_incident_polyhedrons[2*i+1])
            {
                keep++;
            }
            if(1 != keep)
            {
                continue;
            }
            facets_count++;
        }
        
        ofstream out_surface_file(filesystem::path(argv[1]).filename().replace_extension(".off").string());
        out_surface_file << "OFF\n";
        out_surface_file << polyhedralization.m_vertices.size() << " " << facets_count << " 0 \n";
        out_surface_file << std::setprecision(std::numeric_limits<double>::max_digits10);
        for(uint32_t i=0; i<polyhedralization.m_vertices.size(); i++)
        {
            double3 vertex = approximate_vertex(polyhedralization.m_vertices[i]);
            out_surface_file << vertex.x << " " << vertex.y << " "<< vertex.z << "\n";
        }
        for(uint32_t i=0; i<polyhedralization.m_facets.size(); i++)
        {
            uint32_t keep = 0;
            if(UNDEFINED_VALUE == polyhedralization.m_facets_incident_polyhedrons[2*i+0])
            {
                keep++;
            }
            if(UNDEFINED_VALUE == polyhedralization.m_facets_incident_polyhedrons[2*i+1])
            {
                keep++;
            }
            if(1 != keep)
            {
                continue;
            }
            vector<uint32_t> vs = polyhedralization.m_facets[i].get_sorted_vertices(polyhedralization.m_segments);
            out_surface_file << vs.size();
            for(uint32_t v : vs)
            {
                out_surface_file << " " << v;
            }
            out_surface_file << "\n";
        }
        out_surface_file.close();
        
//        vector<uint8_t> polyhedralization_bytes = polyhedralization.to_bytes();
//
//        ofstream out_file(filesystem::path(argv[1]).filename().replace_extension(".bin").string(), std::ios::binary);
//        out_file.write((char*)polyhedralization_bytes.data(), polyhedralization_bytes.size());
//        out_file.close();
    }
    else
    {
        auto input = read_input_binary(std::cin);
        auto [input_vertices, input_constraints] = input;
        std::cerr << "Successfully read " << input_vertices.size()/3 << " vertices.\n";
        PolyhedralizationCreationHandle PC;
        std::vector<std::shared_ptr<genericPoint>> vertices = create_vertices(input_vertices.size()/3, input_vertices.data(), 0, nullptr);
        std::vector<uint32_t> constraints = create_constraints(input_constraints.size()/3, input_constraints.data(), vertices.data(), true);
        
        Polyhedralization polyhedralization = PC.calculate(vertices, constraints);
        std::vector<uint8_t> polyhedralization_bytes = polyhedralization.to_bytes();
        
        std::cout.write(
            reinterpret_cast<const char*>(polyhedralization_bytes.data()),
            polyhedralization_bytes.size()
        );
        std::cout.flush();
    }

    return 0;
}
