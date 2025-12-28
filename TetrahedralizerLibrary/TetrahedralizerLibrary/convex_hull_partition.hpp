#ifndef convex_hull_partition_hpp
#define convex_hull_partition_hpp

#include "common_header.h"
#include "common_function.h"
#include "geometric_object/polyhedralization.hpp"
#include "geometric_object/tetrahedralization.hpp"
#include "geometric_object/segment.h"
#include "geometric_object/facet.h"
#include "triangle_tetrahedron_intersection.h"
#include "facet_order.h"

class ConvexHullPartitionHandle
{
public:
    void Dispose();
    void AddInput(uint32_t explicit_count, double* explicit_values, uint32_t implicit_count, uint32_t* implicit_values,
                  uint32_t tetrahedrons_count, uint32_t* tetrahedrons, uint32_t constraints_facets_count, FacetInteropData* constraints_facets,
                  uint32_t segments_count, SegmentInteropData* segments, uint32_t coplanar_triangles_count, uint32_t* coplanar_triangles);
    
    void Calculate();

    uint32_t GetInsertedVerticesCount();
    void GetInsertedVertices(uint32_t* out);
    uint32_t GetPolyhedronsCount();
    void GetPolyhedrons(uint32_t* out);
    uint32_t GetFacetsCount();
    void GetFacets(FacetInteropData* outArray);
    uint32_t GetSegmentsCount();
    void GetSegments(SegmentInteropData* outArray);
    
    
private:
    std::vector<Segment> m_constraints_segments;
    std::vector<Facet> m_constraints_facets;
    Polyhedralization m_polyhedralization;
    
    std::unordered_map<std::tuple<uint32_t, uint32_t, uint32_t>, uint32_t, trio_iii_hash> m_triangles_coplanar_groups;
    
    void convex_hull_partition();
};

extern "C" LIBRARY_EXPORT void* CreateConvexHullPartitionHandle();
extern "C" LIBRARY_EXPORT void DisposeConvexHullPartitionHandle(void* handle);

extern "C" LIBRARY_EXPORT void AddConvexHullPartitionInput(void* handle, uint32_t explicit_count, double* explicit_values, uint32_t implicit_count, uint32_t* implicit_values,
                                                           uint32_t tetrahedrons_count, uint32_t* tetrahedrons, uint32_t constraints_facets_count, FacetInteropData* constraints_facets,
                                                           uint32_t segments_count, SegmentInteropData* segments, uint32_t coplanar_triangles_count, uint32_t* coplanar_triangles);

extern "C" LIBRARY_EXPORT void CalculateConvexHullPartition(void* handle);

extern "C" LIBRARY_EXPORT uint32_t GetConvexHullPartitionInsertedVerticesCount(void* handle);
extern "C" LIBRARY_EXPORT void GetConvexHullPartitionInsertedVertices(void* handle, uint32_t* out);
extern "C" LIBRARY_EXPORT uint32_t GetConvexHullPartitionPolyhedronsCount(void* handle);
extern "C" LIBRARY_EXPORT void GetConvexHullPartitionPolyhedrons(void* handle, uint32_t* out);
extern "C" LIBRARY_EXPORT uint32_t GetConvexHullPartitionFacetsCount(void* handle);
extern "C" LIBRARY_EXPORT void GetConvexHullPartitionFacets(void* handle, FacetInteropData* outArray);
extern "C" LIBRARY_EXPORT uint32_t GetConvexHullPartitionSegmentsCount(void* handle);
extern "C" LIBRARY_EXPORT void GetConvexHullPartitionSegments(void* handle, SegmentInteropData* outArray);

#endif
