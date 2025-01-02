#include <fstream>
#include <iostream>
#include "include/implicit_point.h"
#include "program_data.h"
#include "delaunay_tetrahedralization.h"
#include "binary_space_partition.h"
#include "interior_characterization.h"
#include "facet_association.h"
using namespace std;



int main(int argc, char *argv[])
{
    streambuf *cinbuf = std::cin.rdbuf();
    streambuf *coutbuf = std::cout.rdbuf();
    
    ifstream fileIn(argv[1]);
    ofstream fileOut(argv[2]);
    cin.rdbuf(fileIn.rdbuf());
    cout.rdbuf(fileOut.rdbuf());
    
    initFPU();
    cin >> m_vertices_count >> m_constraints_count;
    m_vertices.clear();
    for(uint32_t i=0; i<m_vertices_count; i++)
    {
        double t0,t1,t2;
        cin >> t0 >> t1 >> t2;
        m_vertices.push_back(new explicitPoint3D(t0,t1,t2));
    }
    m_constraints.clear();
    uint32_t removed_constraints_count = 0;
    for(uint32_t i=0; i<m_constraints_count; i++)
    {
        uint32_t t0,t1,t2;
        cin >> t0 >> t1 >> t2;
        if(t0 == t1 || t0 == t2 || t1 == t2 || is_collinear(t0, t1, t2))
        {
            removed_constraints_count++;
            continue;
        }
        m_constraints.push_back(t0);
        m_constraints.push_back(t1);
        m_constraints.push_back(t2);
    }
    m_constraints_count -= removed_constraints_count;

        
    delaunay_tetrahedralization();
    binary_space_partition();
    interior_characterization();
    
    cout << m_new_vertices_mappings.size() << endl;
    for(uint32_t i=0; i<m_new_vertices_mappings.size(); i++)
    {
//        if(UNDEFINED_VALUE == m_new_vertices_mappings[i].i5)
//        {
//            cout << 5 << " " << m_new_vertices_mappings[i].i0 << " " << m_new_vertices_mappings[i].i1 << " " << m_new_vertices_mappings[i].i2 << " " << m_new_vertices_mappings[i].i3 << " " << m_new_vertices_mappings[i].i4 << endl;
//        }
//        else
//        {
//            cout << 9 << " " << m_new_vertices_mappings[i].i0 << " " << m_new_vertices_mappings[i].i1 << " " << m_new_vertices_mappings[i].i2 << " " << m_new_vertices_mappings[i].i3 << " " << m_new_vertices_mappings[i].i4 << " " << m_new_vertices_mappings[i].i5 << " " << m_new_vertices_mappings[i].i6 << " " << m_new_vertices_mappings[i].i7 << " " << m_new_vertices_mappings[i].i8 << endl;;
//        }
        
        cout << i+m_vertices_count << " ";
        double x,y,z;
        m_vertices[i+m_vertices_count]->getApproxXYZCoordinates(x, y, z,true);
        cout << x << " " << y << " " << z << endl;
    }
    
    uint32_t out_polyhedron_count = 0;
    for(uint32_t i=0; i<m_polyhedrons.size(); i++)
    {
        bool out_polyhedron = m_graph_label[i];
        if(out_polyhedron)
        {
            out_polyhedron_count++;
        }
    }
    
    cout << out_polyhedron_count << endl;
    for(uint32_t i=0; i<m_polyhedrons.size(); i++)
    {
        bool out_polyhedron = m_graph_label[i];
        if(!out_polyhedron)
        {
            continue;
        }
        
        if(m_polyhedrons[i]->facets.size() < 4)
        {
            throw "something is wrong";
        }
        
        cout << m_polyhedrons[i]->facets.size() << endl;
        for(uint32_t facet : m_polyhedrons[i]->facets)
        {
            get_polyhedron_facet_vertices(facet, m_deque_i_0);
            cout << m_deque_i_0.size() << " ";
            for(uint32_t e : m_deque_i_0)
            {
                cout << e << " ";
            }
            cout << endl;
        }
//        m_u_set_i_0.clear();
//        for(uint32_t facet : m_polyhedrons[i]->facets)
//        {
//            get_polyhedron_facet_vertices(facet, m_deque_i_0);
//            for(uint32_t v : m_deque_i_0)
//            {
//                m_u_set_i_0.insert(v);
//            }
//        }
//        delaunay_tetrahedralization(m_u_set_i_0);
//        for(uint32_t i=0; i<m_tetrahedrons.size(); i+=4)
//        {
//            if(UNDEFINED_VALUE == m_tetrahedrons[i] || INFINITE_VERTEX == m_tetrahedrons[i+3])
//            {
//                continue;
//            }
//            m_vector_i_1.push_back(m_tetrahedrons[i+0]);
//            m_vector_i_1.push_back(m_tetrahedrons[i+1]);
//            m_vector_i_1.push_back(m_tetrahedrons[i+2]);
//            m_vector_i_1.push_back(m_tetrahedrons[i+3]);
//        }
    }
//    cout << m_vector_i_1.size() / 4 << endl;
//    for(uint32_t i=0; i<m_vector_i_1.size(); i+=4)
//    {
//        cout << m_vector_i_1[i+0] << " " << m_vector_i_1[i+1] << " " << m_vector_i_1[i+2] << " " << m_vector_i_1[i+3] << endl;
//    }

    for(uint32_t i=0; i<m_vertices.size(); i++)
    {
        delete m_vertices[i];
    }
    for(uint32_t i=0; i<m_polyhedrons.size(); i++)
    {
        delete m_polyhedrons[i];
    }
    for(uint32_t i=0; i<m_polyhedrons_facets.size(); i++)
    {
        delete m_polyhedrons_facets[i];
    }
    for(uint32_t i=0; i<m_polyhedrons_edges.size(); i++)
    {
        delete m_polyhedrons_edges[i];
    }
    
    cin.rdbuf(cinbuf); // need to restore original cin
    cout.rdbuf(coutbuf); // need to restore original cout
    
    return 0;
}


void read_double(uint8_t*& src, double& v)
{
    memcpy(&v, src, sizeof(double));
    src += sizeof(double);
}
void read_uint32(uint8_t*& src, uint32_t& v)
{
    memcpy(&v, src, sizeof(uint32_t));
    src += sizeof(uint32_t);
}

void write_double(uint8_t* des, const double& v)
{
    memcpy(des, &v, sizeof(double));
    for(uint32_t i=0; i<sizeof(double); i++)
    {
        m_dll_out.push_back(des[i]);
    }
}

void write_uint32(uint8_t* des, const uint32_t& v)
{
    memcpy(des, &v, sizeof(uint32_t));
    for(uint32_t i=0; i<sizeof(uint32_t); i++)
    {
        m_dll_out.push_back(des[i]);
    }
}


/*
input:
    <vertices#> <constraint#>
    <vertices0's x> <vertices0's y> <vertices0's z>
    ...
    <constraint0's 0th vertex> <constraint0's 1st vertex> <constraint0's 2nd vertex>
    ...
output:
    <inserted vertices#>
        <explicit vertex 0> ... <explicit vertex 8>
        ...
    <tetrahedron#>
        <tetrahedron's 0th vertex>... <tetrahedron0's 3nd vertex>
        ...
*/
extern "C" __declspec(dllexport) uint8_t* tetrahedralize(uint8_t* input)
{
//    ofstream MyFile("/Users/hanzemeng/Desktop/out.txt");
    initFPU();
    read_uint32(input, m_vertices_count);
    read_uint32(input, m_constraints_count);
//    MyFile << m_vertices_count << " " << m_constraints_count << endl;
    
    m_vertices.clear();
    for(uint32_t i=0; i<m_vertices_count; i++)
    {
        double t0,t1,t2;
        read_double(input, t0);
        read_double(input, t1);
        read_double(input, t2);
//        MyFile << t0 << " " << t1 << " " << t2 << endl;
        m_vertices.push_back(new explicitPoint3D(t0,t1,t2));
    }
    m_constraints.clear();
    uint32_t removed_constraints_count = 0;
    for(uint32_t i=0; i<m_constraints_count; i++)
    {
        uint32_t t0,t1,t2;
        read_uint32(input, t0);
        read_uint32(input, t1);
        read_uint32(input, t2);
//        MyFile << t0 << " " << t1 << " " << t2 << endl;
        if(t0 == t1 || t0 == t2 || t1 == t2 || is_collinear(t0, t1, t2))
        {
            removed_constraints_count++;
            continue;
        }
        m_constraints.push_back(t0);
        m_constraints.push_back(t1);
        m_constraints.push_back(t2);
    }
    m_constraints_count -= removed_constraints_count;

    delaunay_tetrahedralization();
    binary_space_partition();
    interior_characterization();
    
    uint8_t buffer[8];
    m_dll_out.clear();
    
    write_uint32(buffer, m_new_vertices_mappings.size());
//    MyFile <<  m_new_vertices_mappings.size() << endl;
    for(uint32_t i=0; i<m_new_vertices_mappings.size(); i++)
    {
        write_uint32(buffer, m_new_vertices_mappings[i].i0);
        write_uint32(buffer, m_new_vertices_mappings[i].i1);
        write_uint32(buffer, m_new_vertices_mappings[i].i2);
        write_uint32(buffer, m_new_vertices_mappings[i].i3);
        write_uint32(buffer, m_new_vertices_mappings[i].i4);
        write_uint32(buffer, m_new_vertices_mappings[i].i5);
        write_uint32(buffer, m_new_vertices_mappings[i].i6);
        write_uint32(buffer, m_new_vertices_mappings[i].i7);
        write_uint32(buffer, m_new_vertices_mappings[i].i8);
        
        
//        double x,y,z;
//        m_vertices[i+m_vertices_count]->getApproxXYZCoordinates(x, y, z,true);
//        write_double(buffer, x);
//        write_double(buffer, y);
//        write_double(buffer, z);
        
//        MyFile << x << " " << y << " " << z << endl;
    }
    
    m_vector_i_1.clear();
    for(uint32_t i=0; i<m_polyhedrons.size(); i++)
    {
        bool out_polyhedron = m_graph_label[i];
        if(!out_polyhedron)
        {
            continue;
        }
        
        if(m_polyhedrons[i]->facets.size() < 4)
        {
            throw "something is wrong";
        }
        
        m_u_set_i_0.clear();
        for(uint32_t facet : m_polyhedrons[i]->facets)
        {
            get_polyhedron_facet_vertices(facet, m_deque_i_0);
            for(uint32_t v : m_deque_i_0)
            {
                m_u_set_i_0.insert(v);
            }
        }
        delaunay_tetrahedralization(m_u_set_i_0);
        for(uint32_t i=0; i<m_tetrahedrons.size(); i+=4)
        {
            if(UNDEFINED_VALUE == m_tetrahedrons[i] || INFINITE_VERTEX == m_tetrahedrons[i+3])
            {
                continue;
            }
            m_vector_i_1.push_back(m_tetrahedrons[i+0]);
            m_vector_i_1.push_back(m_tetrahedrons[i+1]);
            m_vector_i_1.push_back(m_tetrahedrons[i+2]);
            m_vector_i_1.push_back(m_tetrahedrons[i+3]);
        }
    }
    write_uint32(buffer,  m_vector_i_1.size()/4);
    for(uint32_t i=0; i<m_vector_i_1.size(); i++)
    {
        write_uint32(buffer, m_vector_i_1[i]);
    }
    

    for(uint32_t i=0; i<m_vertices.size(); i++)
    {
        delete m_vertices[i];
    }
    for(uint32_t i=0; i<m_polyhedrons.size(); i++)
    {
        delete m_polyhedrons[i];
    }
    for(uint32_t i=0; i<m_polyhedrons_facets.size(); i++)
    {
        delete m_polyhedrons_facets[i];
    }
    for(uint32_t i=0; i<m_polyhedrons_edges.size(); i++)
    {
        delete m_polyhedrons_edges[i];
    }

//    MyFile.close();
    return m_dll_out.data();
}

extern "C" __declspec(dllexport) void load_tetrahedralized_mesh(uint8_t* input)
{
    uint32_t ori_vertices_count, new_vertices_count, tetrahedrons_count;
    read_uint32(input, ori_vertices_count);
    read_uint32(input, new_vertices_count);
    read_uint32(input, tetrahedrons_count);
    read_uint32(input, m_constraints_count);
    m_vertices_count = ori_vertices_count + new_vertices_count;
    
    m_vertices.clear();
    for(uint32_t i=0; i<ori_vertices_count; i++)
    {
        double x,y,z;
        read_double(input, x);
        read_double(input, y);
        read_double(input, z);
        m_vertices.push_back(new explicitPoint3D(x,y,z));
    }
    uint32_t ps[9];
    for(uint32_t i=0; i<new_vertices_count; i++)
    {
        for(uint32_t j=0; j<9; j++)
        {
            read_uint32(input, ps[j]);
        }
        if(UNDEFINED_VALUE == ps[5])
        {
            m_vertices.push_back(new implicitPoint3D_LPI(m_vertices[ps[0]]->toExplicit3D(),m_vertices[ps[1]]->toExplicit3D(),m_vertices[ps[2]]->toExplicit3D(),m_vertices[ps[3]]->toExplicit3D(),m_vertices[ps[4]]->toExplicit3D()));
        }
        else
        {
            m_vertices.push_back(new implicitPoint3D_TPI(m_vertices[ps[0]]->toExplicit3D(),m_vertices[ps[1]]->toExplicit3D(),m_vertices[ps[2]]->toExplicit3D(),m_vertices[ps[3]]->toExplicit3D(),m_vertices[ps[4]]->toExplicit3D(),m_vertices[ps[5]]->toExplicit3D(),m_vertices[ps[6]]->toExplicit3D(),m_vertices[ps[7]]->toExplicit3D(),m_vertices[ps[8]]->toExplicit3D()));
        }
    }
    
    m_tetrahedrons.clear();
    for(uint32_t i=0; i<4*tetrahedrons_count; i++)
    {
        uint32_t temp;
        read_uint32(input, temp);
        m_tetrahedrons.push_back(temp);
    }
    m_constraints.clear();
    for(uint32_t i=0; i<3*m_constraints_count; i++)
    {
        uint32_t temp;
        read_uint32(input, temp);
        m_constraints.push_back(temp);
    }
}

extern "C" __declspec(dllexport) void unload_tetrahedralized_mesh()
{
    for(uint32_t i=0; i<m_vertices.size(); i++)
    {
        delete m_vertices[i];
    }
}

extern "C" __declspec(dllexport) uint8_t* approximate_positions()
{
    uint8_t buffer[8];
    m_dll_out.clear();
    for(uint32_t i=0; i<m_vertices.size(); i++)
    {
        double x,y,z;
        m_vertices[i]->getApproxXYZCoordinates(x, y, z, true);
        write_double(buffer, x);
        write_double(buffer, y);
        write_double(buffer, z);
    }
    return m_dll_out.data();
}

extern "C" __declspec(dllexport) uint8_t* associate_facets()
{
    facet_association();
    
    uint8_t buffer[8];
    m_dll_out.clear();

    for(uint32_t i=0; i<m_polyhedrons_facets_associations.size(); i++)
    {
        write_uint32(buffer, m_polyhedrons_facets_associations[i]);
    }
    return m_dll_out.data();
}
