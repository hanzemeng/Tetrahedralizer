#ifndef tetrahedralization_hpp
#define tetrahedralization_hpp

#include "../common_header.h"
#include "../common_function.h"

class Tetrahedralization
{
public:
    void assign_tetrahedrons(uint32_t* tetrahedrons, uint32_t tetrahedrons_count);
    
    uint32_t get_tetrahedrons_count();
    std::vector<uint32_t> get_all_facets();
    std::vector<uint32_t> get_bounding_facets();
    
    uint32_t get_tetrahedron_vertex(uint32_t t, uint32_t i);
    std::tuple<uint32_t,uint32_t,uint32_t,uint32_t> get_tetrahedron_vertices(uint32_t t);
    std::pair<uint32_t,uint32_t> get_tetrahedron_neighbor(uint32_t t, uint32_t i);
    std::tuple<uint32_t,uint32_t,uint32_t> get_tetrahedron_facet(uint32_t t, uint32_t i);
    
    uint32_t get_incident_tetrahedron(uint32_t v);
    std::vector<uint32_t> get_incident_tetrahedrons(uint32_t v);
    std::vector<uint32_t> get_incident_tetrahedrons(uint32_t v0, uint32_t v1);
    uint32_t get_incident_tetrahedron(uint32_t v0, uint32_t v1, uint32_t v2);
    
    bool tetrahedron_has_vertex(uint32_t t, uint32_t v);

private:
    std::vector<uint32_t> m_tetrahedrons;
    std::vector<uint32_t> m_neighbors; // neighbors order: 0,1,2  1,0,3  0,2,3, 2,1,3
    std::vector<uint32_t> m_vertices_incidents;
    std::queue<uint32_t> m_visit;
    std::vector<uint32_t> m_visited;
    uint32_t m_visited_index;
};

#endif
