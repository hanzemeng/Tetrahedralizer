#ifndef INTERIOR_CHARACTERIZATION
#define INTERIOR_CHARACTERIZATION

#include "program_data.h"
#include "utility.h"

void interior_characterization();

void sort_polyhedron_facet(uint32_t facet);
void get_polyhedron_facet_vertices(uint32_t facet, deque<uint32_t>& res); // the facet must be sorted


#endif
