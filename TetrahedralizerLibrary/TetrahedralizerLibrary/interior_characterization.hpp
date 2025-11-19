#ifndef interior_characterization_hpp
#define interior_characterization_hpp

#include "common_header.h"
#include "common_function.h"
#include "generic_point_approximation.hpp"
#include "polyhedralization.hpp"

class InteriorCharacterizationInput
{
public:
    genericPoint** m_vertices;
    uint32_t m_vertices_count;

    uint32_t* m_polyhedrons; // # of facets followed by indexes
    uint32_t m_polyhedrons_count;

    uint32_t* m_facets; // # of vertices followed by indexes, vertices are ordered in cw or ccw, first 3 vertices of are not collinear
    genericPoint** m_facets_centroids;
    uint32_t m_facets_count;
    
    uint32_t* m_constraints; // Oriented such that the right hand curls around a triangle and the thumb points in the out direction.
    uint32_t m_constraints_count; // # of constraints, same as m_constraints.size()/3
    
    double m_polyhedron_in_multiplier; // multiply this to the cost labeling a polyhedron to be in
};
class InteriorCharacterizationOutput
{
public:
    uint32_t* m_polyhedrons_labels;
    // 1 if the polyhedron is in, otherwise the polyhedron is out
    uint32_t* m_facets_vertices_mapping;
    // for every vertex in every facet, record # of triangles followed by indexes of the triangles
    // note that vertices are duplicated across facets
    uint32_t* m_facets_centroids_mapping;
    // for every facet centriod, an incident triangle will be recorded, UNDEFINED_VALUE if no such triangle
};


class InteriorCharacterization
{
public:
    void interior_characterization(InteriorCharacterizationInput* input, InteriorCharacterizationOutput* output);
private:
    COMMON_FIELDS
};

class InteriorCharacterizationHandle
{
public:
    InteriorCharacterizationInput* m_input;
    InteriorCharacterizationOutput* m_output;
    InteriorCharacterization* m_interiorCharacterization;
    
    InteriorCharacterizationHandle();
    void Dispose();
    
    void AddInput(uint32_t explicit_count, double* explicit_values, uint32_t implicit_count, uint32_t* implicit_values, uint32_t polyhedrons_count, uint32_t* polyhedrons, uint32_t facets_count, uint32_t* facets, uint32_t* facets_centroids, double* facets_centroids_weights, uint32_t constraints_count, uint32_t* constraints, double polyhedron_in_multiplier);
    
    void Calculate();
    
    uint32_t* GetPolyhedronsLabels();
    uint32_t* GetFacetsVerticesMapping();
    uint32_t* GetFacetsCentroidsMapping();
};

extern "C" LIBRARY_EXPORT void* CreateInteriorCharacterizationHandle();
extern "C" LIBRARY_EXPORT void DisposeInteriorCharacterizationHandle(void* handle);

extern "C" LIBRARY_EXPORT void AddInteriorCharacterizationInput(void* handle, uint32_t explicit_count, double* explicit_values, uint32_t implicit_count, uint32_t* implicit_values, uint32_t polyhedrons_count, uint32_t* polyhedrons, uint32_t facets_count, uint32_t* facets, uint32_t* facets_centroids, double* facets_centroids_weights, uint32_t constraints_count, uint32_t* constraints, double polyhedron_in_multiplier);

extern "C" LIBRARY_EXPORT void CalculateInteriorCharacterization(void* handle);

extern "C" LIBRARY_EXPORT uint32_t* GetInteriorCharacterizationPolyhedronsLabels(void* handle);
extern "C" LIBRARY_EXPORT uint32_t* GetInteriorCharacterizationFacetsVerticesMapping(void* handle);
extern "C" LIBRARY_EXPORT uint32_t* GetInteriorCharacterizationFacetsCentroidsMapping(void* handle);

#endif
