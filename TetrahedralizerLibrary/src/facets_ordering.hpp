#ifndef constraints_ordering_hpp
#define constraints_ordering_hpp

#include "common_header.h"
#include "common_function.h"
#include "geometric_object/segment.h"
#include "geometric_object/facet.h"

class FacetsOrderingHandle
{
public:
    FacetsOrderingHandle(uint32_t candidate_facets_count,uint32_t examine_facets_count,uint32_t change_scheme_threshold, uint32_t random_seed);
    
    std::vector<uint32_t> order_facets(std::vector<std::shared_ptr<genericPoint>>& vertices, std::vector<double3>& approximated_vertices, std::vector<Segment>& segments, std::vector<Facet>& facets);
    void order_facets(std::vector<uint32_t>& all_facets_indexes, std::vector<std::shared_ptr<genericPoint>>& vertices, std::vector<double3>& approximated_vertices, std::vector<Segment>& segments, std::vector<Facet>& facets,
                 std::vector<uint32_t>& order_tree,
                 std::vector<std::pair<double3,double>>& facets_spheres, std::unordered_map<uint32_t, std::pair<double3,double>>& planes_equations);
    
private:
    void shuffle_indexes(std::vector<uint32_t>& indexes, uint32_t k);
    int check_plane_facet_intersection(uint32_t c, uint32_t nc,std::vector<std::shared_ptr<genericPoint>>& vertices, std::vector<Segment>& segments,std::vector<Facet>& facets, std::vector<std::pair<double3,double>>& facets_spheres, std::unordered_map<uint32_t, std::pair<double3,double>>& planes_equations);
    
    std::mt19937 m_gen;
    std::uniform_int_distribution<uint32_t> m_dist;
    
    uint32_t m_candidate_facets_count;
    uint32_t m_examine_facets_count;
    uint32_t m_change_scheme_threshold; // If facets count is above, choose a bisecting plane. Otherwise, choose a least intersecting plane.
    
//    std::unordered_map<uint32_t, std::unordered_map<uint32_t, int>> m_facets_cache;
//    std::unordered_map<uint32_t, std::unordered_map<uint32_t, int>> m_vertices_cache;
};

#endif 
