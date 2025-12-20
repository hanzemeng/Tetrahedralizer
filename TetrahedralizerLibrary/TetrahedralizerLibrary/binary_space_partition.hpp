#ifndef binary_space_partition_hpp
#define binary_space_partition_hpp

#include "common_header.h"
#include "common_function.h"
#include "geometric_object/tetrahedralization.hpp"
#include "geometric_object/segment.h"
#include "geometric_object/facet.h"
#include "triangle_tetrahedron_intersection.hpp"

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
    bool m_aggressively_add_virtual_constraints;
    
    std::vector<PolyhedronConstraint> m_polyhedrons_slice_tree;
    std::vector<std::vector<uint32_t>> m_polyhedrons;
    std::vector<Facet> m_facets;
    std::vector<Segment> m_segments;
    
    std::vector<uint32_t> m_inserted_vertices;
    uint32_t m_inserted_vertices_count;
    std::vector<uint32_t> m_output_polyhedrons;
    uint32_t m_output_polyhedrons_count;
    COMMON_FIELDS
    
    void binary_space_partition();
    uint32_t find_or_add_edge(uint32_t p0, uint32_t p1);
    uint32_t find_or_add_facet(uint32_t e0, uint32_t e1, uint32_t e2, uint32_t p0, uint32_t p1,  uint32_t p2);
    void add_virtual_constraint(uint32_t e0, uint32_t e1, uint32_t c); // e0 and e1 incident the constraint c
    uint32_t build_polyhedrons_slice_tree(std::vector<std::tuple<uint32_t,std::vector<Segment>, std::vector<std::shared_ptr<genericPoint>>>>& slices);
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
