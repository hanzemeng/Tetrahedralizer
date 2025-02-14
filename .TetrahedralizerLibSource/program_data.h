#ifndef PROGRAM_DATA
#define PROGRAM_DATA

#include <utility>
#include <vector>
#include <queue>
#include <stack>
#include <deque>
#include <unordered_set>
#include <map>
#include <unordered_map>
#include <cassert>
#include <numbers>
#include "include/implicit_point.h"
#include "graph_cut/GCoptimization.h"

using namespace std;


extern const uint32_t UNDEFINED_VALUE;
extern const uint32_t INFINITE_VERTEX; // only appear in +3 offset


class PolyhedronEdge
{
    public:
    uint32_t e0, e1; // two endpoints
    uint32_t p0, p1, p2, p3, p4, p5; // p0, p1, p2 is a plane, p3, p4, p5 is a plane. The line is the intersection of the two plane. Only use p0, p1 if both are explicit points.
    uint32_t f; // a incident polyhedron facet

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
struct NineUInt32
{
    public:
    uint32_t i0,i1,i2,i3,i4,i5,i6,i7,i8;

    NineUInt32(uint32_t i0,uint32_t i1,uint32_t i2,uint32_t i3,uint32_t i4);
    NineUInt32(uint32_t i0,uint32_t i1,uint32_t i2,uint32_t i3,uint32_t i4,uint32_t i5,uint32_t i6,uint32_t i7,uint32_t i8);
};

struct pair_hash
{
public:
    template <typename T, typename U>
    std::size_t operator()(const std::pair<T, U> &x) const
    {
        return std::hash<T>()(x.first) ^ std::hash<U>()(x.second);
    }
};

struct trio_hash
{
public:
    template <typename T, typename U, typename V>
    std::size_t operator()(const std::tuple<T, U, V> &x) const
    {
        return std::hash<T>()(std::get<0>(x)) ^ std::hash<U>()(std::get<1>(x)) ^ std::hash<V>()(std::get<2>(x));
    }
};


extern uint32_t m_vertices_count;
extern uint32_t m_constraints_count;
extern uint32_t m_virtual_constraints_count;
extern vector<genericPoint*> m_vertices;
extern vector<uint32_t> m_constraints;

extern vector<uint32_t> m_tetrahedrons;
extern queue<uint32_t> m_tetrahedrons_gaps;
// neighbors order: 0,1,2  1,0,3  0,2,3,  2,1,3
extern vector<uint32_t> m_neighbors;

extern vector<Polyhedron*> m_polyhedrons;
extern vector<PolyhedronFacet*> m_polyhedrons_facets;
extern vector<PolyhedronEdge*> m_polyhedrons_edges;

extern vector<uint32_t> m_vertices_incidents;
extern vector<uint32_t> m_tetrahedrons_polyhedrons_mapping; // tetrahedron index is divided by 4
extern map<uint32_t, vector<uint32_t>> m_polyhedrons_intersect_constraints;

extern vector<NineUInt32> m_new_vertices_mappings;

extern vector<double> m_vertices_approximate_positions;
extern vector<double> m_polyhedrons_facets_approximate_areas;
extern vector<double> m_polyhedrons_facets_approximate_centers;
extern vector<double> m_polyhedrons_winding_numbers;

extern vector<bool> m_graph_label;

extern vector<vector<uint32_t>> m_coplanar_constraints; // coplanar as well as adjacent
extern vector<uint32_t> m_polyhedrons_facets_associations;

extern vector<uint8_t> m_dll_out;

extern vector<uint32_t> m_vector_i_0;
extern vector<uint32_t> m_vector_i_1;
extern vector<uint32_t> m_vector_i_2;
extern vector<uint32_t> m_vector_i_3;

extern queue<uint32_t> m_queue_i_0;
extern queue<uint32_t> m_queue_i_1;

extern deque<uint32_t> m_deque_i_0;

extern unordered_set<uint32_t> m_u_set_i_0;
extern unordered_set<uint32_t> m_u_set_i_1;
extern unordered_set<uint32_t> m_u_set_i_2;
extern unordered_set<uint32_t> m_u_set_i_3;

extern unordered_map<uint32_t,int> m_u_map_i_i_0;
extern unordered_map<pair<uint32_t,uint32_t>,uint32_t,pair_hash> m_u_map_ii_i_0;
extern unordered_map<pair<uint32_t,uint32_t>,vector<uint32_t>,pair_hash> m_u_map_ii_vector_i_0;
extern unordered_map<tuple<uint32_t,uint32_t,uint32_t>,uint32_t,trio_hash> m_u_map_iii_i_0;
extern unordered_map<uint32_t,tuple<uint32_t,uint32_t,uint32_t>> m_u_map_i_iii_0;

#endif
