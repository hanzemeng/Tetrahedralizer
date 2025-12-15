#ifndef common_function_h
#define common_function_h

#include "common_header.h"

inline std::vector<uint32_t> flat_array_to_vector(uint32_t* data, uint32_t count)
{
    std::vector<uint32_t> res;
    uint32_t f=0;
    for(uint32_t i=0; i<count; i++)
    {
        res.push_back(data[f]);
        uint32_t n = data[f];
        for(uint32_t j=f+1; j<f+1+n; j++)
        {
            res.push_back(data[j]);
        }
        f += n+1;
    }
    return res;
}
inline std::vector<std::vector<uint32_t>> flat_array_to_nested_vector(uint32_t* data, uint32_t count)
{
    std::vector<std::vector<uint32_t>> res;
    uint32_t f=0;
    for(uint32_t i=0; i<count; i++)
    {
        res.push_back(std::vector<uint32_t>());
        uint32_t n = data[f];
        for(uint32_t j=f+1; j<f+1+n; j++)
        {
            res.back().push_back(data[j]);
        }
        f += n+1;
    }
    return res;
}

inline void sort_ints(uint32_t& i0, uint32_t& i1)
{
    if(i0 > i1)
    {
        std::swap(i0,i1);
    }
}
inline void sort_ints(uint32_t& i0, uint32_t& i1, uint32_t& i2)
{
    if(i0 > i1)
    {
        std::swap(i0,i1);
    }
    if(i1 > i2)
    {
        std::swap(i1,i2);
    }
    if(i0 > i1)
    {
        std::swap(i0,i1);
    }
}

inline void clear_queue(std::queue<uint32_t>& q)
{
    while(!q.empty())
    {
        q.pop();
    }
}
inline void clear_stack(std::stack<uint32_t>& s)
{
    while(!s.empty())
    {
        s.pop();
    }
}

inline int double_to_int(double d)
{
    if(d < 0.0)
    {
        return -1;
    }
    if(d == 0.0)
    {
        return 0;
    }
    return 1;
}

inline int orient3d(uint32_t p0,uint32_t p1,uint32_t p2,uint32_t p3, std::shared_ptr<genericPoint>* m_vertices)
{
    return double_to_int(genericPoint::orient3D(*m_vertices[p0],*m_vertices[p1],*m_vertices[p2],*m_vertices[p3]));
}
inline int orient3d(std::shared_ptr<genericPoint> p0,std::shared_ptr<genericPoint> p1,std::shared_ptr<genericPoint> p2,std::shared_ptr<genericPoint> p3)
{
    return double_to_int(genericPoint::orient3D(*p0,*p1,*p2,*p3));
}
inline int in_sphere(uint32_t p0,uint32_t p1,uint32_t p2,uint32_t p3,uint32_t p4, std::shared_ptr<genericPoint>* m_vertices)
{
    return double_to_int(genericPoint::inSphere(*m_vertices[p0],*m_vertices[p1],*m_vertices[p2],*m_vertices[p3],*m_vertices[p4]));
}
inline bool is_collinear(uint32_t p0, uint32_t p1, uint32_t p2, std::shared_ptr<genericPoint>* m_vertices)
{
    return !genericPoint::misaligned(*m_vertices[p0],*m_vertices[p1],*m_vertices[p2]);
}
inline bool vertex_in_inner_segment(uint32_t p0,uint32_t s0,uint32_t s1, std::shared_ptr<genericPoint>* m_vertices)
{
    return genericPoint::pointInInnerSegment(*m_vertices[p0],*m_vertices[s0],*m_vertices[s1]);
}
inline bool vertex_in_segment(uint32_t p0,uint32_t s0,uint32_t s1, std::shared_ptr<genericPoint>* m_vertices)
{
    return genericPoint::pointInSegment(*m_vertices[p0],*m_vertices[s0],*m_vertices[s1]);
}
inline bool vertex_in_inner_triangle(uint32_t p0,uint32_t t0,uint32_t t1,uint32_t t2, std::shared_ptr<genericPoint>* m_vertices)
{
    return genericPoint::pointInInnerTriangle(*m_vertices[p0],*m_vertices[t0],*m_vertices[t1],*m_vertices[t2]);
}
inline bool vertex_in_triangle(uint32_t p0,uint32_t t0,uint32_t t1,uint32_t t2, std::shared_ptr<genericPoint>* m_vertices)
{
    return genericPoint::pointInTriangle(*m_vertices[p0],*m_vertices[t0],*m_vertices[t1],*m_vertices[t2]);
}
inline bool inner_segment_cross_inner_triangle(uint32_t s0,uint32_t s1,uint32_t t0,uint32_t t1,uint32_t t2, std::shared_ptr<genericPoint>* m_vertices)
{
    return genericPoint::innerSegmentCrossesInnerTriangle(*m_vertices[s0],*m_vertices[s1],*m_vertices[t0],*m_vertices[t1],*m_vertices[t2]);
}
inline bool inner_segment_cross_triangle(uint32_t s0,uint32_t s1,uint32_t t0,uint32_t t1,uint32_t t2, std::shared_ptr<genericPoint>* m_vertices)
{
    return genericPoint::innerSegmentCrossesTriangle(*m_vertices[s0],*m_vertices[s1],*m_vertices[t0],*m_vertices[t1],*m_vertices[t2]);
}
inline bool segment_cross_triangle(uint32_t s0,uint32_t s1,uint32_t t0,uint32_t t1, uint32_t t2, std::shared_ptr<genericPoint>* m_vertices)
{
    if(inner_segment_cross_triangle(s0, s1, t0, t1, t2, m_vertices))
    {
        return true;
    }
 
    if(0 == orient3d(s0,t0,t1,t2, m_vertices) && 0 == orient3d(s1,t0,t1,t2, m_vertices))
    {
        return false;
    }
    return
    (0 == orient3d(s0, t0, t1, t2, m_vertices) && vertex_in_triangle(s0, t0, t1, t2, m_vertices)) ||
    (0 == orient3d(s1, t0, t1, t2, m_vertices) && vertex_in_triangle(s1, t0, t1, t2, m_vertices));
}
inline bool inner_segment_cross_inner_segment(uint32_t s0,uint32_t s1,uint32_t s2,uint32_t s3, std::shared_ptr<genericPoint>* m_vertices)
{
    return genericPoint::innerSegmentsCross(*m_vertices[s0],*m_vertices[s1],*m_vertices[s2],*m_vertices[s3]);
}
inline bool segment_cross_segment(uint32_t s0,uint32_t s1,uint32_t s2,uint32_t s3, std::shared_ptr<genericPoint>* m_vertices)
{
    return genericPoint::segmentsCross(*m_vertices[s0],*m_vertices[s1],*m_vertices[s2],*m_vertices[s3]);
}
inline int max_component_in_triangle_normal(uint32_t t0,uint32_t t1,uint32_t t2, std::shared_ptr<genericPoint>* m_vertices)
{
    return genericPoint::maxComponentInTriangleNormal(
        m_vertices[t0]->toExplicit3D().X(),m_vertices[t0]->toExplicit3D().Y(),m_vertices[t0]->toExplicit3D().Z(),
        m_vertices[t1]->toExplicit3D().X(),m_vertices[t1]->toExplicit3D().Y(),m_vertices[t1]->toExplicit3D().Z(),
        m_vertices[t2]->toExplicit3D().X(),m_vertices[t2]->toExplicit3D().Y(),m_vertices[t2]->toExplicit3D().Z());
}
inline int orient3d_ignore_axis(uint32_t p0,uint32_t p1,uint32_t p2,int axis, std::shared_ptr<genericPoint>* m_vertices)
{
    if(0 == axis)
    {
        return double_to_int(genericPoint::orient2Dyz(*m_vertices[p0],*m_vertices[p1],*m_vertices[p2]));
    }
    if(1 == axis)
    {
        return double_to_int(genericPoint::orient2Dzx(*m_vertices[p0],*m_vertices[p1],*m_vertices[p2]));
    }
    if(2 == axis)
    {
        return double_to_int(genericPoint::orient2Dxy(*m_vertices[p0],*m_vertices[p1],*m_vertices[p2]));
    }
    throw "wrong axis value";
}

inline double3 approximate_point(std::shared_ptr<genericPoint> m_vertices)
{
    double3 res;
    m_vertices->getApproxXYZCoordinates(res.x, res.y, res.z, true);
    return res;
}
inline double3 approximate_facet_centroid(const std::vector<uint32_t>& vertices, std::shared_ptr<genericPoint>* vertices_positions)
{
    double3 res;
    for(uint32_t v : vertices)
    {
        res += approximate_point(vertices_positions[v]);
    }
    return res / (double)vertices.size();
}

// return true if p is on or inside the triangle, false otherwise
// always return false triangle is degenerate
inline bool barycentric_weight(const double3& t0, const double3& t1, const double3& t2, const double3& p, double3& w)
{
    double3 v0 = t1 - t0;
    double3 v1 = t2 - t0;
    double3 v2 = p  - t0;

    double d00 = v0.dot(v0);
    double d01 = v0.dot(v1);
    double d11 = v1.dot(v1);
    double d20 = v2.dot(v0);
    double d21 = v2.dot(v1);

    double denom = d00 * d11 - d01 * d01;
    double eps = 1e-12 * (d00 + d11);
    if(std::fabs(denom) < eps)
    {
        w.x = 0.333;
        w.y = 0.333;
        w.z = 0.333;
        return false;
    }
    w.y = (d11 * d20 - d01 * d21) / denom;
    w.z = (d00 * d21 - d01 * d20) / denom;
    w.x = 1.0 - w.y - w.z;

    double margin = -1e-12 * sqrt(d00 + d11);
    return w.x>=margin && w.y>=margin && w.z>=margin;
}
inline bool barycentric_weight(uint32_t t0, uint32_t t1, uint32_t t2, const double3& p, std::shared_ptr<genericPoint>* vertices, double3& w)
{
    double3 p0 = approximate_point(vertices[t0]);
    double3 p1 = approximate_point(vertices[t1]);
    double3 p2 = approximate_point(vertices[t2]);
    return barycentric_weight(p0,p1,p2,p,w);
}

inline std::vector<std::shared_ptr<genericPoint>> create_vertices(uint32_t explicit_count, double* explicit_values, uint32_t implicit_count, uint32_t* implicit_values)
{
    std::vector<std::shared_ptr<genericPoint>> vertices;
    
    for(uint32_t i=0; i<explicit_count; i++)
    {
        vertices.push_back(std::make_shared<explicitPoint3D>(explicit_values[3*i+0],explicit_values[3*i+1],explicit_values[3*i+2]));
    }
    uint32_t j = 0;
    for(uint32_t i=0; i<implicit_count; i++)
    {
        std::shared_ptr<genericPoint> new_vertex;
        if(5 == implicit_values[j]) // line plane
        {
            new_vertex = std::make_shared<implicitPoint3D_LPI>(
                                                 vertices[implicit_values[j+1]]->toExplicit3D(),
                                                 vertices[implicit_values[j+2]]->toExplicit3D(),
                                                 vertices[implicit_values[j+3]]->toExplicit3D(),
                                                 vertices[implicit_values[j+4]]->toExplicit3D(),
                                                 vertices[implicit_values[j+5]]->toExplicit3D());
            j += 6;
        }
        else if(9 == implicit_values[j]) // three planes
        {
            new_vertex = std::make_shared<implicitPoint3D_TPI>(
                                                 vertices[implicit_values[j+1]]->toExplicit3D(),
                                                 vertices[implicit_values[j+2]]->toExplicit3D(),
                                                 vertices[implicit_values[j+3]]->toExplicit3D(),
                                                 vertices[implicit_values[j+4]]->toExplicit3D(),
                                                 vertices[implicit_values[j+5]]->toExplicit3D(),
                                                 vertices[implicit_values[j+6]]->toExplicit3D(),
                                                 vertices[implicit_values[j+7]]->toExplicit3D(),
                                                 vertices[implicit_values[j+8]]->toExplicit3D(),
                                                 vertices[implicit_values[j+9]]->toExplicit3D());
            j += 10;
        }
        else
        {
            throw "wrong input";
        }
        
        vertices.push_back(new_vertex);
    }
    
    return vertices;
}
inline std::vector<std::shared_ptr<genericPoint>> create_vertices(uint32_t vertices_count, uint32_t* facets_centroids, double* facets_centroids_weights, std::shared_ptr<genericPoint>* vertices)
{
    std::vector<std::shared_ptr<genericPoint>> res;
    for(uint32_t i=0; i<vertices_count; i++)
    {
        res.push_back(std::make_shared<implicitPoint3D_BPT>
                                                            (vertices[facets_centroids[3*i+0]]->toExplicit3D(),
                                                             vertices[facets_centroids[3*i+1]]->toExplicit3D(),
                                                             vertices[facets_centroids[3*i+2]]->toExplicit3D(),
                                                             facets_centroids_weights[2*i+0],
                                                             facets_centroids_weights[2*i+1]));
    }
    return res;
}

inline std::vector<uint32_t> create_constraints(uint32_t constraints_count, uint32_t* constraints, std::shared_ptr<genericPoint>* vertices, bool add_placeholder)
{
    std::vector<uint32_t> res;
    std::unordered_set<std::tuple<uint32_t,uint32_t,uint32_t>,trio_iii_hash> unique_constraints;
    for(uint32_t i=0; i<constraints_count; i++)
    {
        uint32_t c0 = constraints[3*i+0];
        uint32_t c1 = constraints[3*i+1];
        uint32_t c2 = constraints[3*i+2];
        uint32_t t0 = c0;
        uint32_t t1 = c1;
        uint32_t t2 = c2;
        
        sort_ints(t0, t1, t2);
        if(unique_constraints.end() != unique_constraints.find(std::make_tuple(t0,t1,t2)) || t0 == t1 || t0 == t2 || t1 == t2 || is_collinear(t0, t1, t2, vertices))
        {
            unique_constraints.insert(std::make_tuple(t0,t1,t2));
            if(add_placeholder)
            {
                res.push_back(UNDEFINED_VALUE);
                res.push_back(UNDEFINED_VALUE);
                res.push_back(UNDEFINED_VALUE);
            }
            continue;
        }
        unique_constraints.insert(std::make_tuple(t0,t1,t2));
        res.push_back(c0);
        res.push_back(c1);
        res.push_back(c2);
    }
    return res;
}

#endif
