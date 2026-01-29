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
    // convex_hull are triangles that may be coplanar
    Polyhedralization calculate(std::vector<std::shared_ptr<genericPoint>>& vertices, std::vector<uint32_t>& convex_hull, std::vector<uint32_t>& constraints);
};

//extern "C" LIBRARY_EXPORT void* CreateConvexHullPartitionHandle();
//extern "C" LIBRARY_EXPORT void DisposeConvexHullPartitionHandle(void* handle);
//
//extern "C" LIBRARY_EXPORT void AddConvexHullPartitionInput(void* handle, uint32_t explicit_count, double* explicit_values, uint32_t implicit_count, uint32_t* implicit_values,
//                                                           uint32_t tetrahedrons_count, uint32_t* tetrahedrons, uint32_t constraints_facets_count, FacetInteropData* constraints_facets,
//                                                           uint32_t segments_count, SegmentInteropData* segments, uint32_t coplanar_triangles_count, uint32_t* coplanar_triangles);
//
//extern "C" LIBRARY_EXPORT void CalculateConvexHullPartition(void* handle);
//
//extern "C" LIBRARY_EXPORT uint32_t GetConvexHullPartitionInsertedVerticesCount(void* handle);
//extern "C" LIBRARY_EXPORT void GetConvexHullPartitionInsertedVertices(void* handle, uint32_t* out);
//extern "C" LIBRARY_EXPORT uint32_t GetConvexHullPartitionPolyhedronsCount(void* handle);
//extern "C" LIBRARY_EXPORT void GetConvexHullPartitionPolyhedrons(void* handle, uint32_t* out);
//extern "C" LIBRARY_EXPORT uint32_t GetConvexHullPartitionFacetsCount(void* handle);
//extern "C" LIBRARY_EXPORT void GetConvexHullPartitionFacets(void* handle, FacetInteropData* outArray);
//extern "C" LIBRARY_EXPORT uint32_t GetConvexHullPartitionSegmentsCount(void* handle);
//extern "C" LIBRARY_EXPORT void GetConvexHullPartitionSegments(void* handle, SegmentInteropData* outArray);

#endif
