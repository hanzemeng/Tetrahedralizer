#ifndef DELAUNAY_TETRAHEDRALIZATION
#define DELAUNAY_TETRAHEDRALIZATION

#include "program_data.h"
#include "utility.h"

void delaunay_tetrahedralization();
void delaunay_tetrahedralization(const unordered_set<uint32_t>& input_vertices);


int in_sphere(uint32_t t,uint32_t p);
int symbolic_perturbation(uint32_t p0,uint32_t p1,uint32_t p2,uint32_t p3,uint32_t p4);

uint32_t add_tetrahedron(uint32_t p0,uint32_t p1,uint32_t p2,uint32_t p3);
void remove_tetrahedron(uint32_t p0);

void tetrahedralize_hole_helper(uint32_t p0, uint32_t p1, uint32_t t);

#endif
