#ifndef polyhedralization_hpp
#define polyhedralization_hpp

#include "common_header.h"
#include "common_function.h"

// a class for organizing a static polyhedralization

class Polyhedralization
{
public:
    Polyhedralization();
    Polyhedralization(uint32_t polyhedrons_count, uint32_t* polyhedrons, uint32_t facets_count, uint32_t* facets);

    void assign_polyhedrons(uint32_t polyhedrons_count, uint32_t* polyhedrons, uint32_t facets_count, uint32_t* facets);
    void get_polyhedron_facet_neighbors(uint32_t f, uint32_t& n0, uint32_t& n1); // f is facet, n0 and n1 are neightbors

private:
    vector<uint32_t> m_neighbors;
};

#endif
