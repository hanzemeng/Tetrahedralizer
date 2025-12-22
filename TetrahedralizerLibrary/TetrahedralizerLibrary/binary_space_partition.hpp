#ifndef binary_space_partition_hpp
#define binary_space_partition_hpp

#include "common_header.h"
#include "common_function.h"
#include "geometric_object/tetrahedralization.hpp"
#include "geometric_object/polyhedralization.hpp"
#include "geometric_object/segment.h"
#include "geometric_object/facet.h"
#include "triangle_tetrahedron_intersection.hpp"
#include "facet_order.h"

class BinarySpacePartitionHandle
{
public:
    void Dispose();
    void AddInput(uint32_t, double*, uint32_t, uint32_t*, uint32_t, uint32_t*, bool);
    
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
    Tetrahedralization m_tetrahedralization;
    std::vector<uint32_t> m_constraints;
    
    std::vector<PolyhedronConstraint> m_polyhedrons_slice_tree;
    Polyhedralization m_polyhedralization;
    std::vector<uint32_t> m_temp_output;
    
    void binary_space_partition();
};

extern "C" LIBRARY_EXPORT void* CreateBinarySpacePartitionHandle();
extern "C" LIBRARY_EXPORT void DisposeBinarySpacePartitionHandle(void* handle);

extern "C" LIBRARY_EXPORT void AddBinarySpacePartitionInput(void* handle, uint32_t explicit_count, double* explicit_values, uint32_t tetrahedron_count, uint32_t* tetrahedrons, uint32_t constraints_count, uint32_t* constraints, bool);

extern "C" LIBRARY_EXPORT void CalculateBinarySpacePartition(void* handle);

extern "C" LIBRARY_EXPORT uint32_t GetBinarySpacePartitionInsertedVerticesCount(void* handle);
extern "C" LIBRARY_EXPORT uint32_t* GetBinarySpacePartitionInsertedVertices(void* handle);
extern "C" LIBRARY_EXPORT uint32_t GetBinarySpacePartitionPolyhedronsCount(void* handle);
extern "C" LIBRARY_EXPORT uint32_t* GetBinarySpacePartitionPolyhedrons(void* handle);
extern "C" LIBRARY_EXPORT uint32_t GetBinarySpacePartitionFacetsCount(void* handle);
extern "C" LIBRARY_EXPORT void GetBinarySpacePartitionFacets(void* handle, FacetInteropData* outArray);
extern "C" LIBRARY_EXPORT uint32_t GetBinarySpacePartitionSegmentsCount(void* handle);
extern "C" LIBRARY_EXPORT void GetBinarySpacePartitionSegments(void* handle, SegmentInteropData* outArray);

#endif
