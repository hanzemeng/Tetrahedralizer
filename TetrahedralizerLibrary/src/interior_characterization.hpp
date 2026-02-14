#ifndef interior_characterization_hpp
#define interior_characterization_hpp

#include "common_header.h"
#include "common_function.h"
#include "geometric_object/polyhedralization.hpp"
#include "winding_number_approximation.hpp"
#include "graph_cut/GCoptimization.h"


class InteriorCharacterizationHandle
{
public:
    std::vector<uint32_t> calculate(Polyhedralization& polyhedralization, std::vector<uint32_t>& constraints, std::vector<double3>& approximated_vertices);
};

//extern "C" LIBRARY_EXPORT void* CreateInteriorCharacterizationHandle();
//extern "C" LIBRARY_EXPORT void DisposeInteriorCharacterizationHandle(void* handle);
//
//extern "C" LIBRARY_EXPORT void AddInteriorCharacterizationInput(void* handle, uint32_t explicit_count, double* explicit_values, uint32_t implicit_count, uint32_t* implicit_values,
//                                                                uint32_t polyhedrons_count, uint32_t* polyhedrons, uint32_t facets_count, FacetInteropData* facets, uint32_t segments_count, SegmentInteropData* segments,
//                                                                uint32_t coplanar_triangles_count, uint32_t* coplanar_triangles, uint32_t constraints_count, uint32_t* constraints, double polyhedron_in_multiplier);
//
//extern "C" LIBRARY_EXPORT void CalculateInteriorCharacterization(void* handle);
//
//extern "C" LIBRARY_EXPORT void GetInteriorCharacterizationPolyhedronsLabels(void* handle, uint32_t* out);
//extern "C" LIBRARY_EXPORT void GetInteriorCharacterizationFacetsCentroidsMapping(void* handle, uint32_t* out);

#endif
