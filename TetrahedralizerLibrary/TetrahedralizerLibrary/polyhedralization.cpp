#include "polyhedralization.hpp"


Polyhedralization::Polyhedralization()
{
}
Polyhedralization::Polyhedralization(uint32_t polyhedrons_count, uint32_t* polyhedrons, uint32_t facets_count, uint32_t* facets)
{
    assign_polyhedrons(polyhedrons_count, polyhedrons, facets_count, facets);
}


void Polyhedralization::assign_polyhedrons(uint32_t polyhedrons_count, uint32_t* polyhedrons, uint32_t facets_count, uint32_t* facets)
{
    m_neighbors = vector<uint32_t>(2*facets_count, UNDEFINED_VALUE);

    vector<vector<uint32_t>> nestedPolyhedrons = flat_array_to_nested_vector(polyhedrons, polyhedrons_count);
    
    for(uint32_t i=0; i<nestedPolyhedrons.size(); i++)
    {
        for(uint32_t f : nestedPolyhedrons[i])
        {
            if(UNDEFINED_VALUE == m_neighbors[2*f+0])
            {
                m_neighbors[2*f+0] = i;
            }
            else if(UNDEFINED_VALUE == m_neighbors[2*f+1])
            {
                m_neighbors[2*f+1] = i;
            }
            else
            {
                throw "wtf";
            }
        }
    }
}

void Polyhedralization::get_polyhedron_facet_neighbors(uint32_t f, uint32_t& n0, uint32_t& n1)
{
    n0 = m_neighbors[2*f+0];
    n1 = m_neighbors[2*f+1];
    if(UNDEFINED_VALUE == n0)
    {
        swap(n0, n1);
    }
}
