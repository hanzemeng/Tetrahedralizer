#ifndef interior_characterization_hpp
#define interior_characterization_hpp

#include "common_header.h"
#include "common_function.h"
#include "geometric_object/facet.h"
#include "geometric_object/segment.h"
#include "graph_cut/GCoptimization.h"

class InteriorCharacterizationHandle
{
public:
    void Dispose();
    void AddInput(uint32_t explicit_count, double* explicit_values, uint32_t implicit_count, uint32_t* implicit_values,
                  uint32_t polyhedrons_count, uint32_t* polyhedrons, uint32_t facets_count, FacetInteropData* facets, uint32_t segments_count, SegmentInteropData* segments,
                  uint32_t coplanar_triangles_count, uint32_t* coplanar_triangles, uint32_t constraints_count, uint32_t* constraints, double polyhedron_in_multiplier);
    
    void Calculate();
    
    void GetPolyhedronsLabels(uint32_t* out); // 1 if the polyhedron is in, otherwise the polyhedron is out
    void GetFacetsCentroidsMapping(uint32_t* out);

private:
    std::vector<std::shared_ptr<genericPoint>> m_vertices;
    std::vector<std::vector<uint32_t>> m_polyhedrons;
    std::vector<Facet> m_facets;
    std::vector<Segment> m_segments;
    std::unordered_map<std::tuple<uint32_t, uint32_t, uint32_t>, uint32_t, iii32_hash> m_triangles_coplanar_groups;
    std::vector<int> m_coplanar_groups_normals;
    std::vector<uint32_t> m_constraints;
    double m_polyhedron_in_multiplier; // multiply this to the cost labeling a polyhedron to be in
    
    std::vector<uint32_t> m_polyhedrons_labels;
    std::vector<uint32_t> m_facets_centroids_mapping;
    
    void interior_characterization();
};

extern "C" LIBRARY_EXPORT void* CreateInteriorCharacterizationHandle();
extern "C" LIBRARY_EXPORT void DisposeInteriorCharacterizationHandle(void* handle);

extern "C" LIBRARY_EXPORT void AddInteriorCharacterizationInput(void* handle, uint32_t explicit_count, double* explicit_values, uint32_t implicit_count, uint32_t* implicit_values,
                                                                uint32_t polyhedrons_count, uint32_t* polyhedrons, uint32_t facets_count, FacetInteropData* facets, uint32_t segments_count, SegmentInteropData* segments,
                                                                uint32_t coplanar_triangles_count, uint32_t* coplanar_triangles, uint32_t constraints_count, uint32_t* constraints, double polyhedron_in_multiplier);

extern "C" LIBRARY_EXPORT void CalculateInteriorCharacterization(void* handle);

extern "C" LIBRARY_EXPORT void GetInteriorCharacterizationPolyhedronsLabels(void* handle, uint32_t* out);
extern "C" LIBRARY_EXPORT void GetInteriorCharacterizationFacetsCentroidsMapping(void* handle, uint32_t* out);

#endif
