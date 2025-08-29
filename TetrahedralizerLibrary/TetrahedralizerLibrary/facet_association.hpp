#ifndef facet_association_hpp
#define facet_association_hpp

#include "common_header.h"
#include "common_function.h"
#include "tetrahedralization.hpp"


class FacetAssociationInput
{
public:
    genericPoint** m_vertices;
    uint32_t m_vertices_count;
    
    uint32_t* m_tetrahedrons; // Oriented such that the right hand curls around the first 3 points and the thumb points to the 4th point.
    uint32_t m_tetrahedrons_count; // number of tetrahedrons, same as m_tetrahedrons.size()/4

    uint32_t* m_constraints; // Orientation does not matter.
    uint32_t m_constraints_count; // number of constraints, same as m_constraints.size()/3
};
class FacetAssociationOutput
{
public:
    uint32_t* m_tetrahedrons_facets_mapping;
    // for every tetrahedron (t), for every facet (f) of t, for every vertex (v) of f:
    // v is contained in the m_tetrahedrons_facets_mapping[v]th constraint. v is UNDEFINED_VALUE if it is not contained in any constraint.
    // facets are checked in: 0,1,2  1,0,3  0,2,3, 2,1,3
};

class FacetAssociation
{
public:
    void facet_association(FacetAssociationInput* input, FacetAssociationOutput* output);
private:
    COMMON_FIELDS
};


class FacetAssociationHandle
{
public:
    FacetAssociationInput* m_input;
    FacetAssociationOutput* m_output;
    FacetAssociation* m_facetAssociation;
    
    FacetAssociationHandle();
    void Dispose();
    

    void AddFacetAssociationInput(uint32_t, double*, uint32_t, uint32_t*, uint32_t, uint32_t*, uint32_t, uint32_t*);
    void CalculateFacetAssociation();
    
    uint32_t* GetOutputFacetAssociation();
};

extern "C" LIBRARY_EXPORT void* CreateFacetAssociationHandle();
extern "C" LIBRARY_EXPORT void DisposeFacetAssociationHandle(void* handle);

extern "C" LIBRARY_EXPORT void AddFacetAssociationInput(void* handle, uint32_t explicit_count, double* explicit_values, uint32_t implicit_count, uint32_t* implicit_values, uint32_t tetrahedron_count, uint32_t* tetrahedrons, uint32_t constraints_count, uint32_t* constraints);

extern "C" LIBRARY_EXPORT void CalculateFacetAssociation(void* handle);

extern "C" LIBRARY_EXPORT uint32_t* GetOutputFacetAssociation(void* handle);


#endif
