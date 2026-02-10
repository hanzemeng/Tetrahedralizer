#ifndef bounding_volume_hierarchy_2d_hpp
#define bounding_volume_hierarchy_2d_hpp

#include "../common_header.h"
#include "segment.h"
#include "facet.h"

// Minimal 2D version for internal BVH logic
struct double2
{
    double2() : x(0.0), y(0.0) {}
    double2(double x, double y) : x(x), y(y) {}
    
    double x, y;
    double2 operator-(const double2& v) const { return {x - v.x, y - v.y}; }
    double dot(const double2& v) const { return x * v.x + y * v.y; }
    double2 perp() const { return {-y, x}; } // Perpendicular vector
};

struct AABB2D
{
    double2 min_p, max_p;
    
    AABB2D(){};
    AABB2D(double2 p)
    {
        min_p = p;
        max_p = p;
    }
    
    void merge(double2 p)
    {
        min_p.x = std::min(min_p.x, p.x);
        min_p.y = std::min(min_p.y, p.y);
        max_p.x = std::max(max_p.x, p.x);
        max_p.y = std::max(max_p.y, p.y);
    }
    void merge(AABB2D& other)
    {
        min_p.x = std::min(min_p.x, other.min_p.x);
        min_p.y = std::min(min_p.y, other.min_p.y);
        max_p.x = std::max(max_p.x, other.max_p.x);
        max_p.y = std::max(max_p.y, other.max_p.y);
    }
    
    void add_margin(double epsilon=1e-7)
    {
        min_p.x -= epsilon;
        min_p.y -= epsilon;
        max_p.x += epsilon;
        max_p.y += epsilon;
    }

    bool overlaps(AABB2D& other)
    {
        return (min_p.x <= other.max_p.x && max_p.x >= other.min_p.x) &&
               (min_p.y <= other.max_p.y && max_p.y >= other.min_p.y);
    }
};
//
class BoundingVolumeHierarchy2D
{
    struct Node
    {
        AABB2D bounds;
        uint32_t left;
        uint32_t right;
        std::vector<uint32_t> triangles; // indexes to actual triangles
        
        Node()
        {
            left = UNDEFINED_VALUE;
            right = UNDEFINED_VALUE;
        };
    };

    std::vector<Node> m_nodes;
    std::vector<double2> m_approximated_vertices;
    std::vector<AABB2D> m_triangles_bounds;
    const uint32_t LEAF_TRIANGLES_COUNT = 8;

    uint32_t build_recursive(std::vector<uint32_t>& triangles)
    {
        uint32_t node_idx = m_nodes.size();
        m_nodes.push_back(Node());
        
        AABB2D node_box = m_triangles_bounds[triangles[0]];
        for(uint32_t i=1; i<triangles.size(); i++)
        {
            uint32_t t = triangles[i];
            node_box.merge(m_triangles_bounds[t]);
        }
        m_nodes[node_idx].bounds = node_box;

        if(triangles.size() <= LEAF_TRIANGLES_COUNT)
        {
            m_nodes[node_idx].triangles = std::move(triangles);
            return node_idx;
        }

        double dx = node_box.max_p.x - node_box.min_p.x;
        double dy = node_box.max_p.y - node_box.min_p.y;
        auto mid = triangles.begin() + triangles.size()/2;
        if(dx > dy)
        {
            std::nth_element(triangles.begin(), mid, triangles.end(), [&](const uint32_t& a, const uint32_t& b) { return m_triangles_bounds[a].min_p.x < m_triangles_bounds[b].min_p.x; });
        }
        else
        {
            std::nth_element(triangles.begin(), mid, triangles.end(), [&](const uint32_t& a, const uint32_t& b) { return m_triangles_bounds[a].min_p.y < m_triangles_bounds[b].min_p.y; });
        }
        
        std::vector<uint32_t> other_triangles = std::vector<uint32_t>(mid, triangles.end());
        triangles.erase(mid, triangles.end());
        
        m_nodes[node_idx].left = build_recursive(triangles);
        m_nodes[node_idx].right = build_recursive(other_triangles);
        return node_idx;
    }

public:
    void build(std::vector<double3>& approximated_vertices, int ignore_axis, std::vector<uint32_t>& triangles)
    {
        m_approximated_vertices.reserve(approximated_vertices.size());
        for(uint32_t i=0; i<approximated_vertices.size(); i++)
        {
            double x, y;
            if(0 == ignore_axis)
            {
                x = approximated_vertices[i].y;
                y = approximated_vertices[i].z;
            }
            else if(1 == ignore_axis)
            {
                x = approximated_vertices[i].x;
                y = approximated_vertices[i].z;
            }
            else
            {
                x = approximated_vertices[i].x;
                y = approximated_vertices[i].y;
            }
            m_approximated_vertices[i] = double2(x,y);
        }
        
        std::vector<uint32_t> triangles_indexes;
        m_triangles_bounds.resize(triangles.size()/3);
        triangles_indexes.reserve(triangles.size()/3);
        
        for(uint32_t i=0; i<triangles.size()/3; i++)
        {
            uint32_t t0 = triangles[3*i+0];
            uint32_t t1 = triangles[3*i+1];
            uint32_t t2 = triangles[3*i+2];
            
            if(UNDEFINED_VALUE == t0)
            {
                continue;
            }
            
            triangles_indexes.push_back(i);
            m_triangles_bounds[i] = m_approximated_vertices[t0];
            m_triangles_bounds[i].merge(m_approximated_vertices[t1]);
            m_triangles_bounds[i].merge(m_approximated_vertices[t2]);
            m_triangles_bounds[i].add_margin();
        }
        
        build_recursive(triangles_indexes);
    }

    uint32_t get_intersection(std::vector<std::shared_ptr<genericPoint>>& vertices, int ignore_axis, std::vector<Segment>& segments, Facet& facet, std::vector<uint32_t>& triangles)
    {
        std::vector<uint32_t> facet_vertices = facet.get_vertices(segments);
        AABB2D facet_aabb = AABB2D(m_approximated_vertices[facet_vertices[0]]);
        for(uint32_t i=1; i<facet_vertices.size(); i++)
        {
            facet_aabb.merge(m_approximated_vertices[facet_vertices[i]]);
        }
        facet_aabb.add_margin();
        
        std::stack<uint32_t> search_stack;
        search_stack.push(0);
        while(!search_stack.empty())
        {
            uint32_t n = search_stack.top();
            search_stack.pop();
            
            if(!m_nodes[n].bounds.overlaps(facet_aabb))
            {
                continue;
            }
            
            if(UNDEFINED_VALUE == m_nodes[n].left)
            {
                for(uint32_t t : m_nodes[n].triangles)
                {
                    if(!m_triangles_bounds[t].overlaps(facet_aabb))
                    {
                        continue;
                    }
                    
                    uint32_t c0 = triangles[3*t+0];
                    uint32_t c1 = triangles[3*t+1];
                    uint32_t c2 = triangles[3*t+2];
                    std::shared_ptr<genericPoint> centroid = facet.get_implicit_centroid(vertices);
                    if(genericPoint::pointInTriangle(*centroid,*vertices[c0],*vertices[c1],*vertices[c2]))
                    {
                        return t;
                    }
                }
            }
            else
            {
                search_stack.push(m_nodes[n].left);
                search_stack.push(m_nodes[n].right);
            }
        }
        
        return UNDEFINED_VALUE;
    }
};

#endif
