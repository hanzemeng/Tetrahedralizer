#ifndef binary_space_partition_hpp
#define binary_space_partition_hpp

#include "common_header.h"
#include "common_function.h"
#include "tetrahedralization.hpp"
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
    uint32_t* GetFacets();
    uint32_t* GetFacetsCentroids();
    double* GetFacetsCentroidsWeights();
    
    
private:
    class PolyhedronEdge
    {
        public:
        uint32_t e0, e1; // two endpoints
        uint32_t p0, p1, p2, p3, p4, p5; // p0, p1, p2 is a plane, p3, p4, p5 is a plane. The line is the intersection of the two plane. Only use p0, p1 if both are explicit points.

        PolyhedronEdge();
        PolyhedronEdge(const PolyhedronEdge& other);
    };
    class PolyhedronFacet
    {
        public:
        std::vector<uint32_t> edges; // bounding edges, sorted clockwise or counter clockwise
        uint32_t p0,p1,p2; // three points that define the plane
        uint32_t ip0, ip1; // two incident polyhedrons

        PolyhedronFacet();
        PolyhedronFacet(const PolyhedronFacet& other);
    };
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
    std::vector<PolyhedronFacet> m_polyhedrons_facets;
    std::vector<PolyhedronEdge> m_polyhedrons_edges;
    
    std::vector<uint32_t> m_inserted_vertices;
    uint32_t m_inserted_vertices_count;
    std::vector<uint32_t> m_output_polyhedrons;
    uint32_t m_output_polyhedrons_count;
    std::vector<uint32_t> m_output_facets;
    std::vector<uint32_t> m_output_facets_centroids;
    std::vector<double> m_output_facets_centroids_weights;
    uint32_t m_output_facets_count;
    COMMON_FIELDS

    void binary_space_partition();
    uint32_t find_or_add_edge(uint32_t p0, uint32_t p1);
    uint32_t find_or_add_facet(uint32_t e0, uint32_t e1, uint32_t e2, uint32_t p0, uint32_t p1,  uint32_t p2);
    void add_virtual_constraint(uint32_t e0, uint32_t e1, uint32_t c); // e0 and e1 incident the constraint c
    uint32_t build_polyhedrons_slice_tree(std::vector<std::pair<uint32_t, std::vector<std::shared_ptr<genericPoint>>>>& slices);
    uint32_t add_LPI(uint32_t e0, uint32_t e1, uint32_t p0,uint32_t p1,uint32_t p2); // LPI: line plane intersection
    uint32_t add_TPI(uint32_t p0,uint32_t p1,uint32_t p2,uint32_t p3,uint32_t p4,uint32_t p5,uint32_t p6,uint32_t p7,uint32_t p8); // TPI: three planes intersection
    void sort_polyhedron_facet(uint32_t facet);
    void get_polyhedron_facet_vertices(uint32_t facet, std::deque<uint32_t>& res); // the facet must be sorted
    
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
extern "C" LIBRARY_EXPORT uint32_t* GetBinarySpacePartitionFacets(void* handle);
extern "C" LIBRARY_EXPORT uint32_t* GetBinarySpacePartitionFacetsCentroids(void* handle);
extern "C" LIBRARY_EXPORT double* GetBinarySpacePartitionFacetsCentroidsWeights(void* handle);

#endif
