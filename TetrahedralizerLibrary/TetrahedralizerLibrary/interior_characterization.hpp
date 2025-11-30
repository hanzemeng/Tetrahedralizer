#ifndef interior_characterization_hpp
#define interior_characterization_hpp

#include "common_header.h"
#include "common_function.h"
#include "polyhedralization.hpp"

class InteriorCharacterizationHandle
{
public:
    InteriorCharacterizationHandle();
    void Dispose();
    
    void AddInput(uint32_t explicit_count, double* explicit_values, uint32_t implicit_count, uint32_t* implicit_values, uint32_t polyhedrons_count, uint32_t* polyhedrons, uint32_t facets_count, uint32_t* facets, uint32_t* facets_centroids, double* facets_centroids_weights, uint32_t constraints_count, uint32_t* constraints, double polyhedron_in_multiplier);
    
    void Calculate();
    
    uint32_t* GetPolyhedronsLabels(); // 1 if the polyhedron is in, otherwise the polyhedron is out
    uint32_t* GetFacetsVerticesMapping(); // for every vertex in every facet, record # of triangles followed by indexes of the triangles, note that vertices are duplicated across facets
    uint32_t* GetFacetsCentroidsMapping(); // for every facet centriod, an incident triangle will be recorded, UNDEFINED_VALUE if no such triangle
    
private:
    vector<genericPoint*> m_vertices;
    vector<vector<uint32_t>> m_polyhedrons;
    vector<vector<uint32_t>> m_facets;
    vector<genericPoint*> m_facets_centroids;
    vector<uint32_t> m_constraints;
    double m_polyhedron_in_multiplier; // multiply this to the cost labeling a polyhedron to be in
    
    vector<uint32_t> m_polyhedrons_labels;
    vector<uint32_t> m_facets_vertices_mapping;
    vector<uint32_t> m_facets_centroids_mapping;
    COMMON_FIELDS
    
    void interior_characterization();
};

extern "C" LIBRARY_EXPORT void* CreateInteriorCharacterizationHandle();
extern "C" LIBRARY_EXPORT void DisposeInteriorCharacterizationHandle(void* handle);

extern "C" LIBRARY_EXPORT void AddInteriorCharacterizationInput(void* handle, uint32_t explicit_count, double* explicit_values, uint32_t implicit_count, uint32_t* implicit_values, uint32_t polyhedrons_count, uint32_t* polyhedrons, uint32_t facets_count, uint32_t* facets, uint32_t* facets_centroids, double* facets_centroids_weights, uint32_t constraints_count, uint32_t* constraints, double polyhedron_in_multiplier);

extern "C" LIBRARY_EXPORT void CalculateInteriorCharacterization(void* handle);

extern "C" LIBRARY_EXPORT uint32_t* GetInteriorCharacterizationPolyhedronsLabels(void* handle);
extern "C" LIBRARY_EXPORT uint32_t* GetInteriorCharacterizationFacetsVerticesMapping(void* handle);
extern "C" LIBRARY_EXPORT uint32_t* GetInteriorCharacterizationFacetsCentroidsMapping(void* handle);

#endif
