#ifndef tetrahedralization_hpp
#define tetrahedralization_hpp

#include "common_header.h"
#include "common_function.h"

// a class for organizing a static tetrahedralization

class Tetrahedralization
{
public:
    Tetrahedralization();
    Tetrahedralization(uint32_t* tetrahedrons, uint32_t tetrahedrons_count); // 4*m_tetrahedrons_count == m_tetrahedrons.size()
    ~Tetrahedralization();

    void assign_tetrahedrons(uint32_t* tetrahedrons, uint32_t tetrahedrons_count);
    void get_tetrahedron_opposite_vertex(uint32_t t, uint32_t& p);
    void get_tetrahedron_neighbor(uint32_t t, uint32_t i, uint32_t& n);
    void get_tetrahedron_face(uint32_t t, uint32_t i, uint32_t& f0,uint32_t& f1,uint32_t& f2);
    void get_tetrahedron_face(uint32_t t, uint32_t& f0,uint32_t& f1,uint32_t& f2);
    void get_vertex_incident_tetrahedron(uint32_t v, uint32_t& t);
    uint32_t get_vertex_incident_tetrahedron(uint32_t v);
    bool tetrahedron_contains_vertex(uint32_t t, uint32_t v);

private:
    uint32_t* m_tetrahedrons;
    uint32_t m_tetrahedrons_count;
    uint32_t* m_neighbors; // neighbors order: 0,1,2  1,0,3  0,2,3, 2,1,3
    uint32_t* m_vertices_incidents;
    COMMON_FIELDS
};

#endif
