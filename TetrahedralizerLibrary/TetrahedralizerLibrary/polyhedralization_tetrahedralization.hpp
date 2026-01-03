#ifndef polyhedralization_tetrahedralization_hpp
#define polyhedralization_tetrahedralization_hpp

#include "common_header.h"
#include "common_function.h"
#include "geometric_object/facet.h"
#include "geometric_object/segment.h"

class PolyhedralizationTetrahedralizationHandle
{
public:
    void Dispose();
    void AddInput(uint32_t explicit_count, double* explicit_values, uint32_t implicit_count, uint32_t* implicit_values,
                  uint32_t polyhedrons_count, uint32_t* polyhedrons, uint32_t facets_count, FacetInteropData* facets, uint32_t segments_count, SegmentInteropData* segments);
    
    void Calculate();
    
    uint32_t GetInsertedFacetsCentroidsCount();
    void GetInsertedFacetsCentroids(uint32_t* out); // new points are added to the centroids of the listed facets
    uint32_t GetInsertedPolyhedronsCentroidsCount();
    void GetInsertedPolyhedronsCentroids(uint32_t* out); // new points are added to the centroids of the listed polyhedrons, indexed after facets centroids
    uint32_t GetTetrahedronsCount();
    void GetTetrahedrons(uint32_t* out); // oriented such that the right hand curls around the first 3 points and the thumb points to the 4th point.
    
private:
    std::vector<std::shared_ptr<genericPoint>> m_vertices;
    std::vector<double3> m_approximated_vertices;
    std::vector<std::shared_ptr<genericPoint>> m_facets_centroids;
    std::vector<std::vector<uint32_t>> m_polyhedrons;
    std::vector<Facet> m_facets;
    std::vector<Segment> m_segments;
    
    std::vector<std::vector<uint32_t>> m_triangulated_facets;
    std::vector<std::vector<uint32_t>> m_triangulated_facets_counters;
    
    std::vector<uint32_t> m_inserted_facets_centroids;
    std::vector<uint32_t> m_inserted_polyhedrons_centroids;
    std::vector<uint32_t> m_tetrahedrons;
    
    void polyhedralization_tetrahedralization();
    uint32_t find_connect_vertex(uint32_t polyhedron);
    void find_connect_vertex_helper(uint32_t p,uint32_t t0,uint32_t t1,uint32_t t2);
    void add_tetrahedron(uint32_t t0,uint32_t t1,uint32_t t2,uint32_t t3);
};

extern "C" LIBRARY_EXPORT void* CreatePolyhedralizationTetrahedralizationHandle();
extern "C" LIBRARY_EXPORT void DisposePolyhedralizationTetrahedralizationHandle(void* handle);

extern "C" LIBRARY_EXPORT void AddPolyhedralizationTetrahedralizationInput(void* handle, uint32_t explicit_count, double* explicit_values, uint32_t implicit_count, uint32_t* implicit_values,
                                                                           uint32_t polyhedrons_count, uint32_t* polyhedrons, uint32_t facets_count, FacetInteropData* facets, uint32_t segments_count, SegmentInteropData* segments);

extern "C" LIBRARY_EXPORT void CalculatePolyhedralizationTetrahedralization(void* handle);

extern "C" LIBRARY_EXPORT uint32_t GetPolyhedralizationTetrahedralizationInsertedFacetsCentroidsCount(void* handle);
extern "C" LIBRARY_EXPORT void GetPolyhedralizationTetrahedralizationInsertedFacetsCentroids(void* handle, uint32_t* out);
extern "C" LIBRARY_EXPORT uint32_t GetPolyhedralizationTetrahedralizationInsertedPolyhedronsCentroidsCount(void* handle);
extern "C" LIBRARY_EXPORT void GetPolyhedralizationTetrahedralizationInsertedPolyhedronsCentroids(void* handle, uint32_t* out);
extern "C" LIBRARY_EXPORT uint32_t GetPolyhedralizationTetrahedralizationTetrahedronsCount(void* handle);
extern "C" LIBRARY_EXPORT void GetPolyhedralizationTetrahedralizationTetrahedrons(void* handle, uint32_t* out);

#endif
