#ifndef polyhedralization_tetrahedralization_hpp
#define polyhedralization_tetrahedralization_hpp

#include "common_header.h"
#include "common_function.h"

class PolyhedralizationTetrahedralizationInput
{
public:
    genericPoint** m_vertices;
    uint32_t m_vertices_count;
    
    uint32_t* m_polyhedrons; // # of facets followed by indexes
    uint32_t m_polyhedrons_count;

    uint32_t* m_polyhedrons_facets; // # of vertices followed by indexes, vertices are ordered in cw or ccw, assume no collinear points
    uint32_t m_polyhedrons_facets_count;
};

class PolyhedralizationTetrahedralizationOutput
{
public:
    double* m_inserted_vertices;
    uint32_t m_inserted_vertices_count;
    uint32_t* m_tetrahedrons; // Oriented such that the right hand curls around the first 3 points and the thumb points to the 4th point.
    uint32_t m_tetrahedrons_count; // number of tetrahedrons, same as m_tetrahedrons.size()/4
};


class PolyhedralizationTetrahedralization
{
public:
    void polyhedralization_tetrahedralization(PolyhedralizationTetrahedralizationInput* input, PolyhedralizationTetrahedralizationOutput* output);

private:
    vector<genericPoint*> m_vertices;
    vector<vector<uint32_t>> m_polyhedrons;
    vector<vector<uint32_t>> m_facets;
    vector<vector<uint32_t>> m_triangulated_facets;
    vector<vector<uint32_t>> m_facets_counters;
    vector<uint32_t> m_result_tetrahedrons;
    COMMON_FIELDS
    
    uint32_t find_connect_vertex(uint32_t polyhedron);
    void connect_polyhedron_with_vertex(uint32_t polyhedron, uint32_t vertex);
    void connect_polyhedron_with_centroid(uint32_t polyhedron);
    bool polyhedron_has_triangle(uint32_t p,uint32_t t0,uint32_t t1,uint32_t t2);
    void add_tetrahedron(unordered_set<uint32_t>& u_set);
    void add_tetrahedron(uint32_t t0,uint32_t t1,uint32_t t2,uint32_t t3);
};

class PolyhedralizationTetrahedralizationHandle
{
public:
    PolyhedralizationTetrahedralizationInput* m_input;
    PolyhedralizationTetrahedralizationOutput* m_output;
    PolyhedralizationTetrahedralization* m_polyhedralizationTetrahedralization;
    
    PolyhedralizationTetrahedralizationHandle();
    void Dispose();
    

    void AddPolyhedralizationTetrahedralizationInput(uint32_t, double*, uint32_t, uint32_t*, uint32_t, uint32_t*, uint32_t, uint32_t*);
    void CalculatePolyhedralizationTetrahedralization();
    
    uint32_t GetOutputInsertedVerticesCount();
    double* GetOutputInsertedVertices();
    uint32_t GetOutputTetrahedronsCount();
    uint32_t* GetOutputTetrahedrons();
};

extern "C" LIBRARY_EXPORT void* CreatePolyhedralizationTetrahedralizationHandle();
extern "C" LIBRARY_EXPORT void DisposePolyhedralizationTetrahedralizationHandle(void* handle);

extern "C" LIBRARY_EXPORT void AddPolyhedralizationTetrahedralizationInput(void* handle, uint32_t explicit_count, double* explicit_values, uint32_t implicit_count, uint32_t* implicit_values, uint32_t polyhedrons_count, uint32_t* polyhedrons, uint32_t polyhedrons_facets_count, uint32_t* polyhedrons_facets);

extern "C" LIBRARY_EXPORT void CalculatePolyhedralizationTetrahedralization(void* handle);

extern "C" LIBRARY_EXPORT uint32_t GetPolyhedralizationTetrahedralizationInsertedVerticesCount(void* handle);
extern "C" LIBRARY_EXPORT double* GetPolyhedralizationTetrahedralizationInsertedVertices(void* handle);
extern "C" LIBRARY_EXPORT uint32_t GetPolyhedralizationTetrahedralizationTetrahedronsCount(void* handle);
extern "C" LIBRARY_EXPORT uint32_t* GetPolyhedralizationTetrahedralizationTetrahedrons(void* handle);

#endif
