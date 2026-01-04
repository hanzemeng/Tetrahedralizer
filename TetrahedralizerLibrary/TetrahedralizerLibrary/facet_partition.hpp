#ifndef facet_partition_hpp
#define facet_partition_hpp

#include "common_header.h"
#include "common_function.h"
#include "geometric_object/polyhedralization.hpp"
#include "geometric_object/facet.h"
#include "geometric_object/segment.h"

class FacetPartitionHandle
{
public:
    void Dispose();
    void AddInput(uint32_t explicit_count, double* explicit_values, uint32_t implicit_count, uint32_t* implicit_values,
                  uint32_t polyhedrons_count, uint32_t* polyhedrons, uint32_t facets_count, FacetInteropData* facets, uint32_t* facets_centroids_mapping, uint32_t segments_count, SegmentInteropData* segments,
                  uint32_t coplanar_triangles_count, uint32_t* coplanar_triangles, uint32_t constraints_count, uint32_t* constraints);
    
    void Calculate();
    
    uint32_t GetInsertedVerticesCount();
    void GetInsertedVertices(uint32_t* out);
    uint32_t GetPolyhedronsCount();
    void GetPolyhedrons(uint32_t* out);
    uint32_t GetFacetsCount();
    void GetFacets(FacetInteropData* out);
    void GetFacetsCentroidsMapping(uint32_t* out);
    uint32_t GetSegmentsCount();
    void GetSegments(SegmentInteropData* out);

private:
    uint32_t m_explicit_count;
    std::vector<double3> m_approximated_vertices;
    Polyhedralization m_polyhedralization;
    std::unordered_map<std::tuple<uint32_t, uint32_t, uint32_t>, uint32_t, iii32_hash> m_triangles_coplanar_groups;
    std::vector<uint32_t> m_coplanar_triangles;
    std::vector<int> m_coplanar_groups_normals;
    std::vector<uint32_t> m_constraints;
    std::vector<uint32_t> m_facets_centroids_mapping;
};

extern "C" LIBRARY_EXPORT void* CreateFacetPartitionHandle();
extern "C" LIBRARY_EXPORT void DisposeFacetPartitionHandle(void* handle);

extern "C" LIBRARY_EXPORT void AddFacetPartitionInput(void* handle, uint32_t explicit_count, double* explicit_values, uint32_t implicit_count, uint32_t* implicit_values,
                                                      uint32_t polyhedrons_count, uint32_t* polyhedrons, uint32_t facets_count, FacetInteropData* facets, uint32_t* facets_centroids_mapping, uint32_t segments_count, SegmentInteropData* segments,
                                                      uint32_t coplanar_triangles_count, uint32_t* coplanar_triangles, uint32_t constraints_count, uint32_t* constraints);
extern "C" LIBRARY_EXPORT void CalculateFacetPartition(void* handle);

extern "C" LIBRARY_EXPORT uint32_t GetFacetPartitionInsertedVerticesCount(void* handle);
extern "C" LIBRARY_EXPORT void GetFacetPartitionInsertedVertices(void* handle, uint32_t* out);
extern "C" LIBRARY_EXPORT uint32_t GetFacetPartitionPolyhedronsCount(void* handle);
extern "C" LIBRARY_EXPORT void GetFacetPartitionPolyhedrons(void* handle, uint32_t* out);
extern "C" LIBRARY_EXPORT uint32_t GetFacetPartitionFacetsCount(void* handle);
extern "C" LIBRARY_EXPORT void GetFacetPartitionFacets(void* handle, FacetInteropData* out);
extern "C" LIBRARY_EXPORT void GetFacetPartitionFacetsCentrodisMapping(void* handle, uint32_t* out);
extern "C" LIBRARY_EXPORT uint32_t GetFacetPartitionSegmentsCount(void* handle);
extern "C" LIBRARY_EXPORT void GetFacetPartitionSegments(void* handle, SegmentInteropData* out);

#endif
