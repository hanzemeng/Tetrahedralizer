#include "program_data.h"


const uint32_t UNDEFINED_VALUE = 0xffffffff;
const uint32_t INFINITE_VERTEX = 0xfffffffe; // only appear in +3 offset


PolyhedronEdge::PolyhedronEdge(){}
PolyhedronEdge::PolyhedronEdge(PolyhedronEdge* other)
{
    this->e0 = other->e0;
    this->e1 = other->e1;
    this->p0 = other->p0;
    this->p1 = other->p1;
    this->p2 = other->p2;
    this->p3 = other->p3;
    this->p4 = other->p4;
    this->p5 = other->p5;
    this->f = other->f;
}

PolyhedronFacet::PolyhedronFacet(){}
PolyhedronFacet::PolyhedronFacet(PolyhedronFacet* other)
{
    this->edges = other->edges;
    this->p0 = other->p0;
    this->p1 = other->p1;
    this->p2 = other->p2;
    this->ip0 = other->ip0;
    this->ip1 = other->ip1;
}

NineUInt32::NineUInt32(uint32_t i0,uint32_t i1,uint32_t i2,uint32_t i3,uint32_t i4) :
i0(i0), i1(i1), i2(i2), i3(i3), i4(i4), i5(UNDEFINED_VALUE), i6(UNDEFINED_VALUE), i7(UNDEFINED_VALUE), i8(UNDEFINED_VALUE){}
NineUInt32::NineUInt32(uint32_t i0,uint32_t i1,uint32_t i2,uint32_t i3,uint32_t i4,uint32_t i5,uint32_t i6,uint32_t i7,uint32_t i8) :
i0(i0), i1(i1), i2(i2), i3(i3), i4(i4), i5(i5), i6(i6), i7(i7), i8(i8){}

uint32_t m_vertices_count;
uint32_t m_constraints_count;
uint32_t m_virtual_constraints_count;
vector<genericPoint*> m_vertices;
vector<uint32_t> m_constraints;

vector<uint32_t> m_tetrahedrons;
queue<uint32_t> m_tetrahedrons_gaps;
// neighbors order: 0,1,2  1,0,3  0,2,3,  2,1,3
vector<uint32_t> m_neighbors;

vector<Polyhedron*> m_polyhedrons;
vector<PolyhedronFacet*> m_polyhedrons_facets;
vector<PolyhedronEdge*> m_polyhedrons_edges;

vector<uint32_t> m_vertices_incidents;
vector<uint32_t> m_tetrahedrons_polyhedrons_mapping;
map<uint32_t, vector<uint32_t>> m_polyhedrons_intersect_constraints;

vector<NineUInt32> m_new_vertices_mappings;

vector<double> m_vertices_approximate_positions;
vector<double> m_polyhedrons_facets_approximate_areas;
vector<double> m_polyhedrons_facets_approximate_centers;
vector<double> m_polyhedrons_winding_numbers;

vector<bool> m_graph_label;

vector<vector<uint32_t>> m_coplanar_constraints;
vector<uint32_t> m_polyhedrons_facets_associations;

vector<uint8_t> m_dll_out;

vector<uint32_t> m_vector_i_0;
vector<uint32_t> m_vector_i_1;
vector<uint32_t> m_vector_i_2;
vector<uint32_t> m_vector_i_3;

queue<uint32_t> m_queue_i_0;
queue<uint32_t> m_queue_i_1;

deque<uint32_t> m_deque_i_0;

unordered_set<uint32_t> m_u_set_i_0;
unordered_set<uint32_t> m_u_set_i_1;
unordered_set<uint32_t> m_u_set_i_2;
unordered_set<uint32_t> m_u_set_i_3;

unordered_map<uint32_t,int> m_u_map_i_i_0;
unordered_map<pair<uint32_t,uint32_t>,uint32_t,pair_hash> m_u_map_ii_i_0;
unordered_map<pair<uint32_t,uint32_t>,vector<uint32_t>,pair_hash> m_u_map_ii_vector_i_0;
unordered_map<tuple<uint32_t,uint32_t,uint32_t>,uint32_t,trio_hash> m_u_map_iii_i_0;
unordered_map<uint32_t,tuple<uint32_t,uint32_t,uint32_t>> m_u_map_i_iii_0;
