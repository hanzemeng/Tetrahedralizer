#ifndef delaunay_tetrahedralization_hpp
#define delaunay_tetrahedralization_hpp

#include "common_header.h"
#include "common_function.h"

class DelaunayTetrahedralizationInput
{
public:
    genericPoint** m_vertices;
    uint32_t m_vertices_count;
};
class DelaunayTetrahedralizationOutput
{
public:
    uint32_t* m_tetrahedrons; // Oriented such that the right hand curls around the first 3 points and the thumb points to the 4th point.
    uint32_t m_tetrahedrons_count; // number of tetrahedrons, same as m_tetrahedrons.size()/4
};
class DelaunayTetrahedralization
{
public:
    void delaunay_tetrahedralization(DelaunayTetrahedralizationInput* input, DelaunayTetrahedralizationOutput* output);

private:
    const uint32_t INFINITE_VERTEX = UNDEFINED_VALUE-1; // only appear in +3 offset

    uint32_t m_vertices_count;

    genericPoint** m_vertices;
    vector<uint32_t> m_tetrahedrons;
    queue<uint32_t> m_tetrahedrons_gaps;
    vector<uint32_t> m_neighbors; // neighbors order: 0,1,2  1,0,3  0,2,3,  2,1,3
    COMMON_FIELDS
    
//    #include "common_function.h"
    void get_tetrahedron_opposite_vertex(uint32_t t, uint32_t& p);
    void get_tetrahedron_neighbor(uint32_t t, uint32_t i, uint32_t& n);
    void get_tetrahedron_face(uint32_t t, uint32_t i, uint32_t& f0,uint32_t& f1,uint32_t& f2);
    void get_tetrahedron_face(uint32_t t, uint32_t& f0,uint32_t& f1,uint32_t& f2);
    int symbolic_perturbation(uint32_t p0,uint32_t p1,uint32_t p2,uint32_t p3,uint32_t p4);
    int in_sphere(uint32_t t,uint32_t p);
    uint32_t add_tetrahedron(uint32_t p0,uint32_t p1,uint32_t p2,uint32_t p3);
    void remove_tetrahedron(uint32_t t);
    void tetrahedralize_hole_helper(uint32_t p0, uint32_t p1, uint32_t t);
};


class DelaunayTetrahedralizationHandle
{
public:
    DelaunayTetrahedralizationInput* m_input;
    DelaunayTetrahedralizationOutput* m_output;
    DelaunayTetrahedralization* m_delaunayTetrahedralization;
    
    DelaunayTetrahedralizationHandle();
    void Dispose();
    
    // number of vertices, then x,y,z of every vertex
    // number of vertices, then 5/9 followed by indices of explicit vertices
    void AddInputVertices(uint32_t, double*, uint32_t, uint32_t*);
    void CalculateDelaunayTetrahedralization();
    
    uint32_t GetOutputTetrahedronsCount();
    uint32_t* GetOutputTetrahedrons();
};


extern "C" LIBRARY_EXPORT void* CreateDelaunayTetrahedralizationHandle();
extern "C" LIBRARY_EXPORT void DisposeDelaunayTetrahedralizationHandle(void* handle);

extern "C" LIBRARY_EXPORT void AddDelaunayTetrahedralizationVertices(void* handle, uint32_t explicit_count, double* explicit_values, uint32_t implicit_count, uint32_t* implicit_values);

extern "C" LIBRARY_EXPORT void CalculateDelaunayTetrahedralization(void* handle);

extern "C" LIBRARY_EXPORT uint32_t GetOutputTetrahedronsCount(void* handle);
extern "C" LIBRARY_EXPORT uint32_t* GetOutputTetrahedrons(void* handle);

#endif
