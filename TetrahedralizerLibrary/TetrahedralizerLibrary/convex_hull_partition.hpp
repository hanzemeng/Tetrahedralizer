#ifndef convex_hull_partition_hpp
#define convex_hull_partition_hpp

#include "common_header.h"
#include "common_function.h"
#include "geometric_object/tetrahedralization.hpp"
#include "geometric_object/segment.h"
#include "geometric_object/facet.h"
#include "triangle_tetrahedron_intersection.hpp"
#include "facet_order.h"

class ConvexHullPartitionHandle
{
public:
    void Dispose();
    void AddInput(uint32_t explicit_count, double* explicit_values, uint32_t implicit_count, uint32_t* implicit_values, uint32_t convex_hull_facets_count, FacetInteropData* convex_hull_facets, uint32_t constraints_facets_count, FacetInteropData* constraints_facets, uint32_t segments_count, SegmentInteropData* segments);
    
    void Calculate();

    uint32_t GetInsertedVerticesCount();
    uint32_t* GetInsertedVertices();

    uint32_t GetPolyhedronsCount();
    uint32_t* GetPolyhedrons();
    uint32_t GetFacetsCount();
    void GetFacets(FacetInteropData* outArray);
    uint32_t GetSegmentsCount();
    void GetSegments(SegmentInteropData* outArray);
    
    
private:
    class PolyhedronConstraint
    {
        public:
        uint32_t c; // constraint
        uint32_t top; // top constraint node
        uint32_t bot; // bottom constraint node
    };
    
    std::vector<std::shared_ptr<genericPoint>> m_vertices;
    std::vector<Segment> m_segments;
    std::vector<Facet> m_facets;
    std::vector<Facet> m_constraints_facets;
    std::vector<uint32_t> m_coplanar_constraints_planes;
    
    std::vector<std::vector<uint32_t>> m_polyhedrons;
    std::vector<PolyhedronConstraint> m_polyhedrons_slice_tree;
    
    std::vector<uint32_t> m_inserted_vertices;
    uint32_t m_inserted_vertices_count;
    std::vector<uint32_t> m_output_polyhedrons;
    uint32_t m_output_polyhedrons_count;
    COMMON_FIELDS

    void convex_hull_partition();
    uint32_t build_polyhedrons_slice_tree(std::vector<uint32_t>& facets_indexes, std::vector<std::shared_ptr<genericPoint>>& vertices, std::vector<Segment>& segments, std::vector<Facet>& facets,
                                          std::unordered_map<uint32_t, std::unordered_map<uint32_t, int>>& constraints_facets_cache,
                                          std::unordered_map<uint32_t, std::unordered_map<uint32_t, int>>& constraints_vertices_cache);
};

extern "C" LIBRARY_EXPORT void* CreateConvexHullPartitionHandle();
extern "C" LIBRARY_EXPORT void DisposeConvexHullPartitionHandle(void* handle);

extern "C" LIBRARY_EXPORT void AddConvexHullPartitionInput(void* handle, uint32_t explicit_count, double* explicit_values, uint32_t implicit_count, uint32_t* implicit_values, uint32_t convex_hull_facets_count, FacetInteropData* convex_hull_facets, uint32_t constraints_facets_count, FacetInteropData* constraints_facets, uint32_t segments_count, SegmentInteropData* segments);

extern "C" LIBRARY_EXPORT void CalculateConvexHullPartition(void* handle);

extern "C" LIBRARY_EXPORT uint32_t GetConvexHullPartitionInsertedVerticesCount(void* handle);
extern "C" LIBRARY_EXPORT uint32_t* GetConvexHullPartitionInsertedVertices(void* handle);
extern "C" LIBRARY_EXPORT uint32_t GetConvexHullPartitionPolyhedronsCount(void* handle);
extern "C" LIBRARY_EXPORT uint32_t* GetConvexHullPartitionPolyhedrons(void* handle);
extern "C" LIBRARY_EXPORT uint32_t GetConvexHullPartitionFacetsCount(void* handle);
extern "C" LIBRARY_EXPORT void GetConvexHullPartitionFacets(void* handle, FacetInteropData* outArray);
extern "C" LIBRARY_EXPORT uint32_t GetConvexHullPartitionSegmentsCount(void* handle);
extern "C" LIBRARY_EXPORT void GetConvexHullPartitionSegments(void* handle, SegmentInteropData* outArray);

#endif
