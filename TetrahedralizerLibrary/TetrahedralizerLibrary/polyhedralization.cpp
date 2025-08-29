#include "polyhedralization.hpp"


Polyhedralization::Polyhedralization()
{
    m_neighbors = nullptr;
}
Polyhedralization::Polyhedralization(uint32_t* polyhedrons, uint32_t polyhedrons_count, uint32_t* polyhedrons_facets, uint32_t polyhedrons_facets_count)
{
    m_neighbors = nullptr;
    assign_polyhedrons(polyhedrons, polyhedrons_count, polyhedrons_facets, polyhedrons_facets_count);
}
Polyhedralization::~Polyhedralization()
{
    delete[] m_neighbors;
}


void Polyhedralization::assign_polyhedrons(uint32_t* polyhedrons, uint32_t polyhedrons_count, uint32_t* polyhedrons_facets, uint32_t polyhedrons_facets_count)
{
    delete[] m_neighbors;
    m_neighbors = new uint32_t[2*polyhedrons_facets_count];
    for(uint32_t i=0; i<2*polyhedrons_facets_count; i++)
    {
        m_neighbors[i] = UNDEFINED_VALUE;
    }

    // calculate neighbors

    m_vector_i_0.resize(polyhedrons_facets_count);
    m_vector_i_1.resize(polyhedrons_facets_count);
    for(uint32_t i=0; i<polyhedrons_facets_count; i++)
    {
        m_vector_i_0[i] = UNDEFINED_VALUE;
        m_vector_i_1[i] = UNDEFINED_VALUE;
    }

    uint32_t f = 0;
    for(uint32_t i=0; i<polyhedrons_count; i++)
    {
        uint32_t n = polyhedrons[f];
        for(uint32_t j=f+1; j<f+1+n; j++)
        {
            if(UNDEFINED_VALUE == m_vector_i_0[polyhedrons[j]])
            {
                m_neighbors[2*polyhedrons[j]+0] = i;
                m_vector_i_0[polyhedrons[j]] = i;
                m_vector_i_1[polyhedrons[j]] = 2*polyhedrons[j]+1;
            }
            else
            {
                m_neighbors[m_vector_i_1[polyhedrons[j]]] = i;
                uint32_t o = m_vector_i_0[polyhedrons[j]];
                m_neighbors[2*polyhedrons[j]+0] = i;
                m_neighbors[2*polyhedrons[j]+1] = o;
                m_vector_i_0[polyhedrons[j]] = UNDEFINED_VALUE;
            }
        }
        f += n+1;
    }
}

void Polyhedralization::get_polyhedron_facet_neighbors(uint32_t f, uint32_t& n0, uint32_t& n1)
{
    n0 = m_neighbors[2*f+0];
    n1 = m_neighbors[2*f+1];
}
