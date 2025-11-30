#ifndef delaunay_tetrahedralization_hpp
#define delaunay_tetrahedralization_hpp

#include "common_header.h"
#include "common_function.h"

class DelaunayTetrahedralizationHandle
{
public:
    DelaunayTetrahedralizationHandle();
    void Dispose();

    void AddInput(uint32_t, double*, uint32_t, uint32_t*);
    void Calculate();
    
    uint32_t GetOutputTetrahedronsCount();
    uint32_t* GetOutputTetrahedrons();
    
    
private:
    uint32_t INFINITE_VERTEX = UNDEFINED_VALUE - 1;
    vector<genericPoint*> m_vertices;
    vector<uint32_t> m_tetrahedrons;
    queue<uint32_t> m_tetrahedrons_gaps;
    vector<uint32_t> m_neighbors; // neighbors order: 0,2,1  1,0,3  0,2,3,  2,1,3
    COMMON_FIELDS
    
    void delaunay_tetrahedralization();
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


extern "C" LIBRARY_EXPORT void* CreateDelaunayTetrahedralizationHandle();
extern "C" LIBRARY_EXPORT void DisposeDelaunayTetrahedralizationHandle(void* handle);

extern "C" LIBRARY_EXPORT void AddDelaunayTetrahedralizationInput(void* handle, uint32_t explicit_count, double* explicit_values, uint32_t implicit_count, uint32_t* implicit_values);

extern "C" LIBRARY_EXPORT void CalculateDelaunayTetrahedralization(void* handle);

extern "C" LIBRARY_EXPORT uint32_t GetOutputTetrahedronsCount(void* handle);
extern "C" LIBRARY_EXPORT uint32_t* GetOutputTetrahedrons(void* handle);

#endif
