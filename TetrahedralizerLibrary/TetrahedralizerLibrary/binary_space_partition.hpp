#ifndef binary_space_partition_hpp
#define binary_space_partition_hpp

#include "common_header.h"
#include "common_function.h"
#include "geometric_object/tetrahedralization.hpp"
#include "geometric_object/polyhedralization.hpp"
#include "geometric_object/segment.h"
#include "geometric_object/facet.h"
#include "triangle_tetrahedron_intersection.h"
#include "facet_order.h"

class BinarySpacePartitionHandle
{
public:
    void Dispose();
    void AddInput(uint32_t, double*, uint32_t, uint32_t*, uint32_t, uint32_t*, bool);
    
    void Calculate();
    
    uint32_t GetInsertedVerticesCount();
    void GetInsertedVertices(uint32_t* out);
    uint32_t GetPolyhedronsCount();
    void GetPolyhedrons(uint32_t* out);
    uint32_t GetFacetsCount();
    void GetFacets(FacetInteropData* out);
    uint32_t GetSegmentsCount();
    void GetSegments(SegmentInteropData* out);
    uint32_t GetCoplanarTrianglesCount();
    void GetCoplanarTriangles(uint32_t* out);
    
    
private:
    std::vector<std::shared_ptr<genericPoint>> m_vertices;
    Tetrahedralization m_tetrahedralization;
    std::vector<uint32_t> m_constraints;
    
    std::vector<std::vector<uint32_t>> m_coplanar_triangles;
    Polyhedralization m_polyhedralization;

    void binary_space_partition();
};

extern "C" LIBRARY_EXPORT void* CreateBinarySpacePartitionHandle();
extern "C" LIBRARY_EXPORT void DisposeBinarySpacePartitionHandle(void* handle);

extern "C" LIBRARY_EXPORT void AddBinarySpacePartitionInput(void* handle, uint32_t explicit_count, double* explicit_values, uint32_t tetrahedron_count, uint32_t* tetrahedrons, uint32_t constraints_count, uint32_t* constraints, bool);

extern "C" LIBRARY_EXPORT void CalculateBinarySpacePartition(void* handle);

extern "C" LIBRARY_EXPORT uint32_t GetBinarySpacePartitionInsertedVerticesCount(void* handle);
extern "C" LIBRARY_EXPORT void GetBinarySpacePartitionInsertedVertices(void* handle, uint32_t* out);
extern "C" LIBRARY_EXPORT uint32_t GetBinarySpacePartitionPolyhedronsCount(void* handle);
extern "C" LIBRARY_EXPORT void GetBinarySpacePartitionPolyhedrons(void* handle, uint32_t* out);
extern "C" LIBRARY_EXPORT uint32_t GetBinarySpacePartitionFacetsCount(void* handle);
extern "C" LIBRARY_EXPORT void GetBinarySpacePartitionFacets(void* handle, FacetInteropData* out);
extern "C" LIBRARY_EXPORT uint32_t GetBinarySpacePartitionSegmentsCount(void* handle);
extern "C" LIBRARY_EXPORT void GetBinarySpacePartitionSegments(void* handle, SegmentInteropData* out);
extern "C" LIBRARY_EXPORT uint32_t GetBinarySpacePartitionCoplanarTrianglesCount(void* handle);
extern "C" LIBRARY_EXPORT void GetBinarySpacePartitionCoplanarTriangles(void* handle, uint32_t* out);

#endif
