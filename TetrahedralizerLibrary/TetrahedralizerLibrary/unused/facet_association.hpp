#ifndef facet_association_hpp
#define facet_association_hpp

#include "common_header.h"
#include "common_function.h"


class FacetAssociationInput
{
public:
    genericPoint** m_vertices;
    uint32_t m_vertices_count;
    
    uint32_t* m_facets; // # of vertices followed by indexes, vertices are ordered in cw or ccw
    uint32_t m_facets_count; // number of facets

    uint32_t* m_constraints; // triangles that each facet vertex will associate with, orientation does not matter.
    uint32_t m_constraints_count; // number of constraints, same as m_constraints.size()/3
};
class FacetAssociationOutput
{
public:
    uint32_t* m_facets_vertices_mapping;
    // for every vertex in every facet, record # of triangles followed by indexes of the triangles
    // note that vertices are duplicated across facets
    uint32_t* m_facets_centroids_mapping;
    // for every facet centriod, an incident triangle will be recorded, UNDEFINED_VALUE if no such triangle
};

class FacetAssociation
{
public:
    void facet_association(FacetAssociationInput* input, FacetAssociationOutput* output);
private:
    void facet_constraint_association(uint32_t facet, uint32_t constraint);
    
    genericPoint** m_vertices;
    vector<vector<uint32_t>> m_facets;
    vector<vector<uint32_t>> m_coplanar_facets;
    uint32_t* m_constraints;
    vector<vector<uint32_t>> m_coplanar_constraints;
    vector<vector<vector<uint32_t>>> m_vertices_mapping;
    vector<uint32_t> m_centroids_mapping;
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
    
    uint32_t* GetFacetsVerticesMapping();
    uint32_t* GetFacetsCentroidsMapping();
};

extern "C" LIBRARY_EXPORT void* CreateFacetAssociationHandle();
extern "C" LIBRARY_EXPORT void DisposeFacetAssociationHandle(void* handle);

extern "C" LIBRARY_EXPORT void AddFacetAssociationInput(void* handle, uint32_t explicit_count, double* explicit_values, uint32_t implicit_count, uint32_t* implicit_values, uint32_t facets_count, uint32_t* facets, uint32_t constraints_count, uint32_t* constraints);

extern "C" LIBRARY_EXPORT void CalculateFacetAssociation(void* handle);

extern "C" LIBRARY_EXPORT uint32_t* GetFacetAssociationFacetsVerticesMapping(void* handle);
extern "C" LIBRARY_EXPORT uint32_t* GetFacetAssociationFacetsCentroidsMapping(void* handle);


#endif
