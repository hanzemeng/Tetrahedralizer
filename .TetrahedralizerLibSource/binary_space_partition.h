#ifndef BINARY_SPACE_PARTITION
#define BINARY_SPACE_PARTITION // I do not know why this phase has binary in its name.

#include "program_data.h"
#include "utility.h"

void binary_space_partition();

uint32_t find_or_add_edge(uint32_t p0, uint32_t p1);
uint32_t find_or_add_facet(uint32_t e0, uint32_t e1, uint32_t e2, uint32_t p0, uint32_t p1,  uint32_t p2);

void add_virtual_constraint(uint32_t e0, uint32_t e1, uint32_t c); // e0 and e1 incident the constraint c

void get_polyhedron_facet_vertices(uint32_t f, uint32_t& f0,uint32_t& f1,uint32_t& f2); // only for facet with 3 vertices

uint32_t tt_intersection(uint32_t t0, uint32_t t1, uint32_t t2, uint32_t t3, uint32_t c0, uint32_t c1, uint32_t c2);

uint32_t add_LPI(uint32_t e0, uint32_t e1, uint32_t p0,uint32_t p1,uint32_t p2); // LPI: line plane intersection
uint32_t add_TPI(uint32_t p0,uint32_t p1,uint32_t p2,uint32_t p3,uint32_t p4,uint32_t p5,uint32_t p6,uint32_t p7,uint32_t p8); // TPI: three plane intersection
#endif
