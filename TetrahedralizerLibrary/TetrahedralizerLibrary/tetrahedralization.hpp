#ifndef tetrahedralization_hpp
#define tetrahedralization_hpp

#include "common_header.h"
#include "common_function.h"

// a class for organizing a static tetrahedralization

class Tetrahedralization
{
public:
    void assign_tetrahedrons(uint32_t* tetrahedrons, uint32_t tetrahedrons_count);
    uint32_t get_tetrahedrons_count();
    uint32_t get_tetrahedron_vertex(uint32_t t, uint32_t i);
    uint32_t get_tetrahedron_neighbor(uint32_t t, uint32_t i);
    void get_tetrahedron_facet(uint32_t t, uint32_t i, uint32_t& f0,uint32_t& f1,uint32_t& f2);
    uint32_t get_vertex_incident_tetrahedron(uint32_t v);

private:
    std::vector<uint32_t> m_tetrahedrons;
    std::vector<uint32_t> m_neighbors; // neighbors order: 0,1,2  1,0,3  0,2,3, 2,1,3
    std::vector<uint32_t> m_vertices_incidents;
    COMMON_FIELDS
};

#endif
