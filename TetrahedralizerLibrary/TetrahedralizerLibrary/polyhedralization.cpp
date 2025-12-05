#include "polyhedralization.hpp"
using namespace std;

void Polyhedralization::assign_polyhedrons(const vector<vector<uint32_t>>& polyhedrons)
{
    uint32_t facets_count = polyhedrons[0][0];
    for(uint32_t i=0; i<polyhedrons.size(); i++)
    {
        for(uint32_t j=0; j<polyhedrons[i].size(); j++)
        {
            facets_count = max(facets_count, polyhedrons[i][j]);
        }
    }
    m_neighbors = vector<uint32_t>(2*(facets_count+1), UNDEFINED_VALUE);
    
    for(uint32_t i=0; i<polyhedrons.size(); i++)
    {
        for(uint32_t f : polyhedrons[i])
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
