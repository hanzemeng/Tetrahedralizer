#ifndef polyhedralization_hpp
#define polyhedralization_hpp

#include "common_header.h"
#include "common_function.h"
#include "segment.h"
#include "facet.h"

class Polyhedralization
{
public:
    std::vector<std::shared_ptr<genericPoint>> m_vertices;
    std::vector<std::vector<uint32_t>> m_polyhedrons;
    std::vector<Facet> m_facets;
    std::vector<Segment> m_segments;

    std::vector<std::vector<uint32_t>> m_inserted_vertices;
    
    // 1 if above, -1 if below, 0 if split. new polyhedron appended to m_polyhedrons
    int slice_polyhedron_with_plane(uint32_t p, uint32_t c0, uint32_t c1, uint32_t c2);
    void calculate_facets_centroids();
};

#endif
