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

    uint32_t* m_polyhedrons_facets; // # of vertices followed by indexes, vertices are ordered in cw or ccw
    uint32_t m_polyhedrons_facets_count;
};

class PolyhedralizationTetrahedralizationOutput
{
public:
    uint32_t* m_tetrahedrons; // Oriented such that the right hand curls around the first 3 points and the thumb points to the 4th point.
    uint32_t m_tetrahedrons_count; // number of tetrahedrons, same as m_tetrahedrons.size()/4
};


class PolyhedralizationTetrahedralization
{
public:
    void polyhedralization_tetrahedralization(PolyhedralizationTetrahedralizationInput* input, PolyhedralizationTetrahedralizationOutput* output);

private:
    COMMON_FIELDS
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
    
    uint32_t GetOutputTetrahedronsCount();
    uint32_t* GetOutputTetrahedrons();
};

extern "C" LIBRARY_EXPORT void* CreatePolyhedralizationTetrahedralizationHandle();
extern "C" LIBRARY_EXPORT void DisposePolyhedralizationTetrahedralizationHandle(void* handle);

extern "C" LIBRARY_EXPORT void AddPolyhedralizationTetrahedralizationInput(void* handle, uint32_t explicit_count, double* explicit_values, uint32_t implicit_count, uint32_t* implicit_values, uint32_t polyhedrons_count, uint32_t* polyhedrons, uint32_t polyhedrons_facets_count, uint32_t* polyhedrons_facets);

extern "C" LIBRARY_EXPORT void CalculatePolyhedralizationTetrahedralization(void* handle);

extern "C" LIBRARY_EXPORT uint32_t GetPolyhedralizationTetrahedralizationTetrahedronsCount(void* handle);
extern "C" LIBRARY_EXPORT uint32_t* GetPolyhedralizationTetrahedralizationTetrahedrons(void* handle);

#endif
