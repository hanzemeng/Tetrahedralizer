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

    uint32_t* m_facets; // # of vertices followed by indexes, vertices are ordered in cw or ccw
    genericPoint** m_facets_centroids;
    uint32_t m_facets_count;
};

class PolyhedralizationTetrahedralizationOutput
{
public:
    uint32_t* m_inserted_facets_centroids; // new points are added to the centroids of the listed facets
    uint32_t m_inserted_facets_centroids_count;
    uint32_t* m_inserted_polyhedrons_centroids; // new points are added to the centroids of the listed polyhedrons, indexed after facets centroids
    uint32_t m_inserted_polyhedrons_centroids_count;
    uint32_t* m_tetrahedrons; // oriented such that the right hand curls around the first 3 points and the thumb points to the 4th point.
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
    vector<vector<uint32_t>> m_triangulated_facets_counters;
    vector<uint32_t> m_inserted_facets_centroids;
    vector<uint32_t> m_inserted_polyhedrons_centroids;
    vector<uint32_t> m_tetrahedrons;
    COMMON_FIELDS
    
    uint32_t find_connect_vertex(uint32_t polyhedron);
    bool polyhedron_has_triangle(uint32_t p,uint32_t t0,uint32_t t1,uint32_t t2);
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
    

    void AddPolyhedralizationTetrahedralizationInput(uint32_t, double*, uint32_t, uint32_t*, uint32_t, uint32_t*, uint32_t, uint32_t*, uint32_t*, double*);
    void CalculatePolyhedralizationTetrahedralization();
    
    uint32_t GetInsertedFacetsCentroidsCount();
    uint32_t* GetInsertedFacetsCentroids();
    uint32_t GetInsertedPolyhedronsCentroidsCount();
    uint32_t* GetInsertedPolyhedronsCentroids();
    uint32_t GetTetrahedronsCount();
    uint32_t* GetTetrahedrons();
};

extern "C" LIBRARY_EXPORT void* CreatePolyhedralizationTetrahedralizationHandle();
extern "C" LIBRARY_EXPORT void DisposePolyhedralizationTetrahedralizationHandle(void* handle);

extern "C" LIBRARY_EXPORT void AddPolyhedralizationTetrahedralizationInput(void* handle, uint32_t explicit_count, double* explicit_values, uint32_t implicit_count, uint32_t* implicit_values, uint32_t polyhedrons_count, uint32_t* polyhedrons, uint32_t facets_count, uint32_t* facets, uint32_t* facets_centroids, double* facets_centroids_weights);

extern "C" LIBRARY_EXPORT void CalculatePolyhedralizationTetrahedralization(void* handle);

extern "C" LIBRARY_EXPORT uint32_t GetPolyhedralizationTetrahedralizationInsertedFacetsCentroidsCount(void* handle);
extern "C" LIBRARY_EXPORT uint32_t* GetPolyhedralizationTetrahedralizationInsertedFacetsCentroids(void* handle);
extern "C" LIBRARY_EXPORT uint32_t GetPolyhedralizationTetrahedralizationInsertedPolyhedronsCentroidsCount(void* handle);
extern "C" LIBRARY_EXPORT uint32_t* GetPolyhedralizationTetrahedralizationInsertedPolyhedronsCentroids(void* handle);
extern "C" LIBRARY_EXPORT uint32_t GetPolyhedralizationTetrahedralizationTetrahedronsCount(void* handle);
extern "C" LIBRARY_EXPORT uint32_t* GetPolyhedralizationTetrahedralizationTetrahedrons(void* handle);

#endif
