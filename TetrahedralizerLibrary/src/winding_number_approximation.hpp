#ifndef winding_number_approximation_hpp
#define winding_number_approximation_hpp

#include "common_header.h"

class WindingNumberApproximation
{
public:
    struct Node {
        double3 m_bb_min, m_bb_max;
        double3 m_centroid;
        double3 m_total_area_normal;
        double m_radius = 0.0;
        
        uint32_t m_left_child = UNDEFINED_VALUE;
        uint32_t m_right_child = UNDEFINED_VALUE;
        
        uint32_t m_triangle_start = UNDEFINED_VALUE;
        uint32_t m_triangle_count = 0;
        bool m_is_leaf = false;
    };

    // Constructor handles the pre-calculation of normals and tree building
    WindingNumberApproximation(const std::vector<double3>& vertices, const std::vector<uint32_t>& triangles);
    
    // Fast query using the dipole approximation
    double compute(const double3& p, const std::vector<double3>& vertices, const std::vector<uint32_t>& triangles, double theta = 2.0) const;

private:
    std::vector<Node> m_nodes;
    std::vector<uint32_t> m_sorted_indices;
    
    // Internal cache for per-triangle data to speed up tree traversal/building
    std::vector<double3> m_tri_centroids;
    std::vector<double3> m_tri_area_normals;

    uint32_t build_recursive(uint32_t start, uint32_t end, const std::vector<double3>& vertices, const std::vector<uint32_t>& triangles);
    double evaluate_recursive(uint32_t node_idx, const double3& p, const std::vector<double3>& vertices, const std::vector<uint32_t>& triangles, double theta) const;
    double exact_solid_angle(const double3& p, const double3& v0, const double3& v1, const double3& v2) const;
};

#endif
