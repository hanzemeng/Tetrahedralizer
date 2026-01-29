#ifndef delaunay_tetrahedralization_hpp
#define delaunay_tetrahedralization_hpp

#include "common_header.h"
#include "common_function.h"
#include "geometric_object/tetrahedralization.hpp"

class DelaunayTetrahedralizationHandle
{
public:
    Tetrahedralization calculate(const std::vector<std::shared_ptr<genericPoint>>& vertices);
    
private:
    uint32_t INFINITE_VERTEX = UNDEFINED_VALUE - 1;
    std::vector<std::shared_ptr<genericPoint>> m_vertices;
    std::vector<uint32_t> m_tetrahedrons;
    std::queue<uint32_t> m_tetrahedrons_gaps;
    std::vector<uint32_t> m_neighbors; // neighbors order: 0,1,2  1,0,3  0,2,3,  2,1,3
    
    void delaunay_tetrahedralization();
    uint32_t get_tetrahedron_neighbor(uint32_t t, uint32_t i);
    std::tuple<uint32_t,uint32_t,uint32_t> get_tetrahedron_facet(uint32_t t, uint32_t i);
    int symbolic_perturbation(uint32_t p0,uint32_t p1,uint32_t p2,uint32_t p3,uint32_t p4);
    int in_sphere(uint32_t t,uint32_t p);
    uint32_t add_tetrahedron(uint32_t p0,uint32_t p1,uint32_t p2,uint32_t p3);
    void remove_tetrahedron(uint32_t t);
};


//extern "C" LIBRARY_EXPORT void* CreateDelaunayTetrahedralizationHandle();
//extern "C" LIBRARY_EXPORT void DisposeDelaunayTetrahedralizationHandle(void* handle);
//
//extern "C" LIBRARY_EXPORT void AddDelaunayTetrahedralizationInput(void* handle, uint32_t explicit_count, double* explicit_values, uint32_t implicit_count, uint32_t* implicit_values);
//
//extern "C" LIBRARY_EXPORT void CalculateDelaunayTetrahedralization(void* handle);
//
//extern "C" LIBRARY_EXPORT uint32_t GetDelaunayTetrahedralizationTetrahedronsCount(void* handle);
//extern "C" LIBRARY_EXPORT void GetDelaunayTetrahedralizationTetrahedrons(void* handle, uint32_t* out);

#endif
