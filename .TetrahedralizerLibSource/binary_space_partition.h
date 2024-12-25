#ifndef BINARY_SPACE_PARTITION
#define BINARY_SPACE_PARTITION // I do not know why this phase has binary in its name.

#include "program_data.h"
#include "utility.h"

void binary_space_partition();

uint32_t find_or_add_edge(uint32_t p0, uint32_t p1);
uint32_t find_or_add_facet(uint32_t e0, uint32_t e1, uint32_t e2, uint32_t p0, uint32_t p1,  uint32_t p2);

void add_virtual_constraint(uint32_t e0, uint32_t e1, uint32_t c); // e0 and e1 incident the constraint c

bool has_vertex(uint32_t t, uint32_t v);
bool edge_intersects_triangle(uint32_t e0,uint32_t e1,uint32_t t0,uint32_t t1,uint32_t t2, uint32_t& o0);

void add_tetrahedron_incident(uint32_t p0, unordered_set<uint32_t>& visited, queue<uint32_t>& to_be_visited, unordered_set<uint32_t>& res_set, queue<uint32_t>& res_queue);
void add_tetrahedron_incident(uint32_t p0, uint32_t p1, unordered_set<uint32_t>& visited, queue<uint32_t>& to_be_visited, unordered_set<uint32_t>& res_set, queue<uint32_t>& res_queue);

void get_polyhedron_facet_vertices(uint32_t f, uint32_t& f0,uint32_t& f1,uint32_t& f2); // only for facet with 3 vertices

void improper_intersection_helper_0(uint32_t& t0,uint32_t& t1,uint32_t& t2,uint32_t& t3,int& o0,int& o1,int& o2,int& o3, uint32_t& zero_count);

uint32_t add_LPI(uint32_t e0, uint32_t e1, uint32_t p0,uint32_t p1,uint32_t p2); // LPI: line plane intersection
uint32_t add_TPI(uint32_t p0,uint32_t p1,uint32_t p2,uint32_t p3,uint32_t p4,uint32_t p5,uint32_t p6,uint32_t p7,uint32_t p8); // TPI: three plane intersection
#endif
