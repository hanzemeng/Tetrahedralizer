#include "winding_number_approximation.hpp"

WindingNumberApproximation::WindingNumberApproximation(const std::vector<double3>& vertices, const std::vector<uint32_t>& triangles) {
    uint32_t tri_count = (uint32_t)(triangles.size() / 3);
    if (tri_count == 0) return;

    m_tri_centroids.resize(tri_count);
    m_tri_area_normals.resize(tri_count);
    m_sorted_indices.resize(tri_count);

    // Step 1: Pre-calculate triangle properties
    for (uint32_t i = 0; i < tri_count; ++i) {
        m_sorted_indices[i] = i;
        const double3& v0 = vertices[triangles[i * 3 + 0]];
        const double3& v1 = vertices[triangles[i * 3 + 1]];
        const double3& v2 = vertices[triangles[i * 3 + 2]];

        m_tri_centroids[i] = (v0 + v1 + v2) / 3.0;
        // Area-weighted normal: 0.5 * (v1-v0) x (v2-v0)
        m_tri_area_normals[i] = 0.5 * (v1 - v0).cross(v2 - v0);
    }

    m_nodes.reserve(tri_count * 2);
    build_recursive(0, tri_count, vertices, triangles);
}

uint32_t WindingNumberApproximation::build_recursive(uint32_t start, uint32_t end, const std::vector<double3>& vertices, const std::vector<uint32_t>& triangles) {
    uint32_t node_idx = (uint32_t)m_nodes.size();
    m_nodes.emplace_back();
    
    uint32_t count = end - start;
    const double d_max = std::numeric_limits<double>::max();
    const double d_min = -std::numeric_limits<double>::max();
    double3 bb_min(d_max, d_max, d_max);
    double3 bb_max(d_min, d_min, d_min);
    double3 sum_pos(0,0,0);
    double3 sum_normal(0,0,0);

    for (uint32_t i = start; i < end; ++i) {
        uint32_t tri_idx = m_sorted_indices[i];
        sum_normal += m_tri_area_normals[tri_idx];
        sum_pos += m_tri_centroids[tri_idx];

        for (uint32_t j = 0; j < 3; ++j) {
            const double3& v = vertices[triangles[tri_idx * 3 + j]];
            bb_min = bb_min.min(v);
            bb_max = bb_max.max(v);
        }
    }

    m_nodes[node_idx].m_bb_min = bb_min;
    m_nodes[node_idx].m_bb_max = bb_max;
    m_nodes[node_idx].m_total_area_normal = sum_normal;
    m_nodes[node_idx].m_centroid = sum_pos / (double)count;
    m_nodes[node_idx].m_radius = (bb_max - m_nodes[node_idx].m_centroid).length();

    if (count <= 4) {
        m_nodes[node_idx].m_is_leaf = true;
        m_nodes[node_idx].m_triangle_start = start;
        m_nodes[node_idx].m_triangle_count = count;
        return node_idx;
    }

    double3 size = bb_max - bb_min;
    uint32_t axis = (size.x > size.y && size.x > size.z) ? 0 : (size.y > size.z ? 1 : 2);

    uint32_t mid = start + count / 2;
    std::nth_element(m_sorted_indices.begin() + start, m_sorted_indices.begin() + mid, m_sorted_indices.begin() + end,
        [&](uint32_t a, uint32_t b) {
            return m_tri_centroids[a][axis] < m_tri_centroids[b][axis];
        });

    uint32_t left = build_recursive(start, mid, vertices, triangles);
    uint32_t right = build_recursive(mid, end, vertices, triangles);
    
    m_nodes[node_idx].m_left_child = left;
    m_nodes[node_idx].m_right_child = right;

    return node_idx;
}

double WindingNumberApproximation::evaluate_recursive(uint32_t node_idx, const double3& p, const std::vector<double3>& vertices, const std::vector<uint32_t>& triangles, double theta) const {
    const Node& node = m_nodes[node_idx];
    double3 r_vec = node.m_centroid - p;
    double dist_sq = r_vec.length_squared();
    double dist = std::sqrt(dist_sq);

    // Multipole Acceptance Criterion
    if (!node.m_is_leaf && dist > 1e-9 && (node.m_radius / dist) < (1.0 / theta)) {
        return r_vec.dot(node.m_total_area_normal) / (dist_sq * dist);
    }

    if (node.m_is_leaf) {
        double sum = 0;
        for (uint32_t i = 0; i < node.m_triangle_count; ++i) {
            uint32_t tri_idx = m_sorted_indices[node.m_triangle_start + i];
            sum += exact_solid_angle(p,
                vertices[triangles[tri_idx * 3 + 0]],
                vertices[triangles[tri_idx * 3 + 1]],
                vertices[triangles[tri_idx * 3 + 2]]);
        }
        return sum;
    }

    double res = 0;
    if (node.m_left_child != UNDEFINED_VALUE) res += evaluate_recursive(node.m_left_child, p, vertices, triangles, theta);
    if (node.m_right_child != UNDEFINED_VALUE) res += evaluate_recursive(node.m_right_child, p, vertices, triangles, theta);
    return res;
}

double WindingNumberApproximation::compute(const double3& p, const std::vector<double3>& vertices, const std::vector<uint32_t>& triangles, double theta) const {
    if (m_nodes.empty()) return 0.0;
    return evaluate_recursive(0, p, vertices, triangles, theta) / (4.0 * M_PI);
}

double WindingNumberApproximation::exact_solid_angle(const double3& p, const double3& v0, const double3& v1, const double3& v2) const {
    double3 a = v0 - p, b = v1 - p, c = v2 - p;
    double al = a.length(), bl = b.length(), cl = c.length();
    double det = a.dot(b.cross(c));
    double denom = al*bl*cl + a.dot(b)*cl + a.dot(c)*bl + b.dot(c)*al;
    return 2.0 * std::atan2(det, denom);
}
