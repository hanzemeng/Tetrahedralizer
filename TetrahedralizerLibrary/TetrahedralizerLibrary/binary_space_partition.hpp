#ifndef binary_space_partition_hpp
#define binary_space_partition_hpp

#include "common_header.h"
#include "common_function.h"
#include "tetrahedralization.hpp"
#include "triangle_tetrahedron_intersection.h"

class BinarySpacePartitionInput
{
public:
    genericPoint** m_vertices;
    uint32_t m_vertices_count;

    uint32_t* m_tetrahedrons; // Oriented such that the right hand curls around the first 3 points and the thumb points to the 4th point.
    uint32_t m_tetrahedrons_count; // number of tetrahedrons, same as m_tetrahedrons.size()/4

    uint32_t* m_constraints; // Orientation does not matter.
    uint32_t m_constraints_count; // number of constraints, same as m_constraints.size()/3
};
class BinarySpacePartitionOutput
{
public:
    uint32_t* m_inserted_vertices; // 5/9 followed by indices of explicit vertices
    uint32_t m_inserted_vertices_count;

    uint32_t* m_polyhedrons; // # of facets followed by indexes
    uint32_t m_polyhedrons_count;

    uint32_t* m_polyhedrons_facets; // # of vertices followed by indexes, vertices are ordered in cw or ccw
    uint32_t m_polyhedrons_facets_count;
};


class PolyhedronEdge
{
    public:
    uint32_t e0, e1; // two endpoints
    uint32_t p0, p1, p2, p3, p4, p5; // p0, p1, p2 is a plane, p3, p4, p5 is a plane. The line is the intersection of the two plane. Only use p0, p1 if both are explicit points.
//    uint32_t f; // a incident polyhedron facet

    PolyhedronEdge();
    PolyhedronEdge(PolyhedronEdge* other);
};
class PolyhedronFacet
{
    public:
    vector<uint32_t> edges; // bounding edges, sorted clockwise or counter clockwise
    uint32_t p0,p1,p2; // three points that defines the plane
    uint32_t ip0, ip1; // two incident polyhedrons

    PolyhedronFacet();
    PolyhedronFacet(PolyhedronFacet* other);
};
class Polyhedron
{
    public:
    vector<uint32_t> facets;
};

class BinarySpacePartition
{
public:
    void binary_space_partition(BinarySpacePartitionInput* m_input, BinarySpacePartitionOutput* m_output);

private:
    vector<genericPoint*> m_vertices;
    uint32_t* m_tetrahedrons;
    Tetrahedralization m_tetrahedralization;
    vector<uint32_t> m_constraints;

    vector<Polyhedron*> m_polyhedrons;
    vector<PolyhedronFacet*> m_polyhedrons_facets;
    vector<PolyhedronEdge*> m_polyhedrons_edges;

    vector<uint32_t> m_vertices_incidents;
    vector<uint32_t> m_tetrahedrons_polyhedrons_mapping; // tetrahedron index is divided by 4
    map<uint32_t, vector<uint32_t>> m_polyhedrons_intersect_constraints;

    uint32_t m_virtual_constraints_count;

    vector<uint32_t> m_new_vertices_mappings;
    COMMON_FIELDS

    uint32_t find_or_add_edge(uint32_t p0, uint32_t p1);
    uint32_t find_or_add_facet(uint32_t e0, uint32_t e1, uint32_t e2, uint32_t p0, uint32_t p1,  uint32_t p2);
    void add_virtual_constraint(uint32_t e0, uint32_t e1, uint32_t c); // e0 and e1 incident the constraint c
    void get_polyhedron_facet_vertices(uint32_t f, uint32_t& f0,uint32_t& f1,uint32_t& f2); // only for facet with 3 vertices
    uint32_t add_LPI(uint32_t e0, uint32_t e1, uint32_t p0,uint32_t p1,uint32_t p2); // LPI: line plane intersection
    uint32_t add_TPI(uint32_t p0,uint32_t p1,uint32_t p2,uint32_t p3,uint32_t p4,uint32_t p5,uint32_t p6,uint32_t p7,uint32_t p8); // TPI: three planes intersection
    void sort_polyhedron_facet(uint32_t facet);
    void get_polyhedron_facet_vertices(uint32_t facet, deque<uint32_t>& res); // the facet must be sorted
};


class BinarySpacePartitionHandle
{
public:
    BinarySpacePartitionInput* m_input;
    BinarySpacePartitionOutput* m_output;
    BinarySpacePartition* m_binarySpacePartition;

    BinarySpacePartitionHandle();
    void Dispose();

    void AddBinarySpacePartitionInput(uint32_t, double*, uint32_t, uint32_t*, uint32_t, uint32_t*);
    void CalculateBinarySpacePartition();

    uint32_t GetOutputInsertedVerticesCount();
    uint32_t* GetOutputInsertedVertices();

    uint32_t GetOutputPolyhedronsCount();
    uint32_t* GetOutputPolyhedrons();

    uint32_t GetOutputPolyhedronsFacetsCount();
    uint32_t* GetOutputPolyhedronsFacets();
};

extern "C" LIBRARY_EXPORT void* CreateBinarySpacePartitionHandle();
extern "C" LIBRARY_EXPORT void DisposeBinarySpacePartitionHandle(void* handle);

extern "C" LIBRARY_EXPORT void AddBinarySpacePartitionInput(void* handle, uint32_t explicit_count, double* explicit_values, uint32_t tetrahedron_count, uint32_t* tetrahedrons, uint32_t constraints_count, uint32_t* constraints);

extern "C" LIBRARY_EXPORT void CalculateBinarySpacePartition(void* handle);

extern "C" LIBRARY_EXPORT uint32_t GetOutputInsertedVerticesCount(void* handle);
extern "C" LIBRARY_EXPORT uint32_t* GetOutputInsertedVertices(void* handle);

extern "C" LIBRARY_EXPORT uint32_t GetOutputPolyhedronsCount(void* handle);
extern "C" LIBRARY_EXPORT uint32_t* GetOutputPolyhedrons(void* handle);

extern "C" LIBRARY_EXPORT uint32_t GetOutputPolyhedronsFacetsCount(void* handle);
extern "C" LIBRARY_EXPORT uint32_t* GetOutputPolyhedronsFacets(void* handle);

#endif
