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

    uint32_t* m_polyhedrons_facets; // # of vertices followed by indexes, vertices are ordered in cw or ccw
    uint32_t m_polyhedrons_facets_count;
    
    uint32_t* m_constraints; // Oriented such that the right hand curls around a triangle and the thumb points in the out direction.
    uint32_t m_constraints_count; // # of constraints, same as m_constraints.size()/3
    
    double* m_polyhedrons_winding_numbers; // If this is not nullptr, then skip winding numbers calculation.
//    uint32_t m_polyhedrons_winding_numbers_count; // same as m_polyhedrons_count
    
    double m_min_cut_neighbor_multiplier; // Neighbor (edge) cost in the dual graph of the polyhedraliztion will be multiplied by this.
};
class InteriorCharacterizationOutput
{
public:
    double* m_polyhedrons_winding_numbers; // nullptr if m_polyhedrons_winding_numbers is given in input
    
    uint32_t* m_polyhedrons_interior_labels; // 0 if the polyhedron is out, otherwise the polyhedron is in.
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
    
    void AddInteriorCharacterizationInput(uint32_t, double*, uint32_t, uint32_t*, uint32_t, uint32_t*, uint32_t, uint32_t*, uint32_t, uint32_t*, double*, double);
    
    void CalculateInteriorCharacterization();
    
    double* GetOutputPolyhedronsWindingNumbers();
    uint32_t* GetOutputPolyhedronsLabels();
};

extern "C" LIBRARY_EXPORT void* CreateInteriorCharacterizationHandle();
extern "C" LIBRARY_EXPORT void DisposeInteriorCharacterizationHandle(void* handle);

extern "C" LIBRARY_EXPORT void AddInteriorCharacterizationInput(void* handle, uint32_t explicit_count, double* explicit_values, uint32_t implicit_count, uint32_t* implicit_values, uint32_t polyhedrons_count, uint32_t* polyhedrons, uint32_t polyhedrons_facets_count, uint32_t* polyhedrons_facets, uint32_t constraints_count, uint32_t* constraints, double* polyhedrons_winding_numbers, double min_cut_neighbor_multiplier);

extern "C" LIBRARY_EXPORT void CalculateInteriorCharacterization(void* handle);

extern "C" LIBRARY_EXPORT double* GetOutputPolyhedronsWindingNumbers(void* handle);
extern "C" LIBRARY_EXPORT uint32_t* GetOutputPolyhedronsLabels(void* handle);

#endif
