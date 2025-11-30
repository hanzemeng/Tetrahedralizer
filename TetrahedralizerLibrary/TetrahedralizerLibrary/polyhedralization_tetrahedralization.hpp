#ifndef polyhedralization_tetrahedralization_hpp
#define polyhedralization_tetrahedralization_hpp

#include "common_header.h"
#include "common_function.h"

class PolyhedralizationTetrahedralizationHandle
{
public:
    PolyhedralizationTetrahedralizationHandle();
    void Dispose();
    
    void AddInput(uint32_t, double*, uint32_t, uint32_t*, uint32_t, uint32_t*, uint32_t, uint32_t*, uint32_t*, double*);
    void Calculate();
    
    uint32_t GetInsertedFacetsCentroidsCount();
    uint32_t* GetInsertedFacetsCentroids(); // new points are added to the centroids of the listed facets
    uint32_t GetInsertedPolyhedronsCentroidsCount();
    uint32_t* GetInsertedPolyhedronsCentroids(); // new points are added to the centroids of the listed polyhedrons, indexed after facets centroids
    uint32_t GetTetrahedronsCount();
    uint32_t* GetTetrahedrons(); // oriented such that the right hand curls around the first 3 points and the thumb points to the 4th point.
    
private:
    vector<genericPoint*> m_vertices;
    vector<vector<uint32_t>> m_polyhedrons;
    vector<vector<uint32_t>> m_facets;
    vector<genericPoint*> m_facets_centroids;
    
    vector<vector<uint32_t>> m_triangulated_facets;
    vector<vector<uint32_t>> m_triangulated_facets_counters;
    
    vector<uint32_t> m_inserted_facets_centroids;
    vector<uint32_t> m_inserted_polyhedrons_centroids;
    vector<uint32_t> m_tetrahedrons;
    COMMON_FIELDS
    
    void polyhedralization_tetrahedralization();
    uint32_t find_connect_vertex(uint32_t polyhedron);
    void find_connect_vertex_helper(uint32_t p,uint32_t t0,uint32_t t1,uint32_t t2);
    void add_tetrahedron(uint32_t t0,uint32_t t1,uint32_t t2,uint32_t t3);
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
