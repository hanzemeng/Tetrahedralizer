#ifndef common_function_h
#define common_function_h

#include "common_header.h"

inline void create_vertices(uint32_t explicit_count, double* explicit_values, uint32_t implicit_count, uint32_t* implicit_values, genericPoint**& vertices, u_int32_t& vertices_count)
{
    vertices = new genericPoint*[explicit_count+implicit_count];
    vertices_count = explicit_count+implicit_count;
    
    for(uint32_t i=0; i<explicit_count; i++)
    {
        vertices[i] = new explicitPoint3D(explicit_values[3*i+0],explicit_values[3*i+1],explicit_values[3*i+2]);
    }
    
    uint32_t j = 0;
    for(uint32_t i=0; i<implicit_count; i++)
    {
        genericPoint* new_vertex;
        if(5 == implicit_values[j]) // line plane
        {
            new_vertex = new implicitPoint3D_LPI(
                                                 vertices[implicit_values[j+1]]->toExplicit3D(),
                                                 vertices[implicit_values[j+2]]->toExplicit3D(),
                                                 vertices[implicit_values[j+3]]->toExplicit3D(),
                                                 vertices[implicit_values[j+4]]->toExplicit3D(),
                                                 vertices[implicit_values[j+5]]->toExplicit3D());
            j += 6;
        }
        else if(9 == implicit_values[j]) // three planes
        {
            new_vertex = new implicitPoint3D_TPI(
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
        
        vertices[explicit_count+i] = new_vertex;
    }
}

inline void delete_vertex(genericPoint* vertex)
{
    if(vertex->isLPI())
    {
        delete (implicitPoint3D_LPI*)vertex;
    }
    else if(vertex->isTPI())
    {
        delete (implicitPoint3D_TPI*)vertex;
    }
    else if(vertex->isExplicit3D())
    {
        delete (explicitPoint3D*)vertex;
    }
    else
    {
        throw "unknown type";
    }
}
inline void delete_vertices(genericPoint** vertices, uint32_t vertices_count)
{
    for(uint32_t i=0; i<vertices_count; i++)
    {
        delete_vertex(vertices[i]);
    }
    delete[] vertices;
}

inline vector<uint32_t> read_flat_vector(uint32_t count, uint32_t* data)
{
    vector<uint32_t> res;
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

inline void vector_to_array(vector<uint32_t>& vec, uint32_t*& arr)
{
    arr = new uint32_t[vec.size()];
    for(uint32_t i=0; i<vec.size(); i++)
    {
        arr[i] = vec[i];
    }
}

inline void sort_ints(uint32_t& i0, uint32_t& i1)
{
    if(i0 > i1)
    {
        swap(i0,i1);
    }
}
inline void sort_ints(uint32_t& i0, uint32_t& i1, uint32_t& i2)
{
    if(i0 > i1 && i0 > i2)
    {
        swap(i0,i2);
    }
    else if(i1 > i0 && i1 > i2)
    {
        swap(i1,i2);
    }
    if(i0>i1)
    {
        swap(i0,i1);
    }
}

inline void clear_queue(queue<uint32_t>& q)
{
    while(!q.empty())
    {
        q.pop();
    }
}
inline void clear_stack(stack<uint32_t>& s)
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

inline int orient3d(uint32_t p0,uint32_t p1,uint32_t p2,uint32_t p3, genericPoint** m_vertices)
{
    return double_to_int(genericPoint::orient3D(*m_vertices[p0],*m_vertices[p1],*m_vertices[p2],*m_vertices[p3]));
}
inline int in_sphere(uint32_t p0,uint32_t p1,uint32_t p2,uint32_t p3,uint32_t p4, genericPoint** m_vertices)
{
    return double_to_int(genericPoint::inSphere(*m_vertices[p0],*m_vertices[p1],*m_vertices[p2],*m_vertices[p3],*m_vertices[p4]));
}
inline bool is_collinear(uint32_t p0, uint32_t p1, uint32_t p2, genericPoint** m_vertices)
{
    return !genericPoint::misaligned(*m_vertices[p0],*m_vertices[p1],*m_vertices[p2]);
}
inline bool vertex_in_inner_segment(uint32_t p0,uint32_t s0,uint32_t s1, genericPoint** m_vertices)
{
    return genericPoint::pointInInnerSegment(*m_vertices[p0],*m_vertices[s0],*m_vertices[s1]);
}
inline bool vertex_in_segment(uint32_t p0,uint32_t s0,uint32_t s1, genericPoint** m_vertices)
{
    return genericPoint::pointInSegment(*m_vertices[p0],*m_vertices[s0],*m_vertices[s1]);
}
inline bool vertex_in_inner_triangle(uint32_t p0,uint32_t t0,uint32_t t1,uint32_t t2, genericPoint** m_vertices)
{
    return genericPoint::pointInInnerTriangle(*m_vertices[p0],*m_vertices[t0],*m_vertices[t1],*m_vertices[t2]);
}
inline bool vertex_in_triangle(uint32_t p0,uint32_t t0,uint32_t t1,uint32_t t2, genericPoint** m_vertices)
{
    return genericPoint::pointInTriangle(*m_vertices[p0],*m_vertices[t0],*m_vertices[t1],*m_vertices[t2]);
}
inline bool inner_segment_cross_inner_triangle(uint32_t s0,uint32_t s1,uint32_t t0,uint32_t t1,uint32_t t2, genericPoint** m_vertices)
{
    return genericPoint::innerSegmentCrossesInnerTriangle(*m_vertices[s0],*m_vertices[s1],*m_vertices[t0],*m_vertices[t1],*m_vertices[t2]);
}
inline bool inner_segment_cross_triangle(uint32_t s0,uint32_t s1,uint32_t t0,uint32_t t1,uint32_t t2, genericPoint** m_vertices)
{
    return genericPoint::innerSegmentCrossesTriangle(*m_vertices[s0],*m_vertices[s1],*m_vertices[t0],*m_vertices[t1],*m_vertices[t2]);
}
inline bool inner_segment_cross_inner_segment(uint32_t s0,uint32_t s1,uint32_t s2,uint32_t s3, genericPoint** m_vertices)
{
    return genericPoint::innerSegmentsCross(*m_vertices[s0],*m_vertices[s1],*m_vertices[s2],*m_vertices[s3]);
}
inline bool segment_cross_triangle(uint32_t s0,uint32_t s1,uint32_t t0,uint32_t t1, uint32_t t2, genericPoint** m_vertices)
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
inline int max_component_in_triangle_normal(uint32_t t0,uint32_t t1,uint32_t t2, genericPoint** m_vertices)
{
    return genericPoint::maxComponentInTriangleNormal(
        m_vertices[t0]->toExplicit3D().X(),m_vertices[t0]->toExplicit3D().Y(),m_vertices[t0]->toExplicit3D().Z(),
        m_vertices[t1]->toExplicit3D().X(),m_vertices[t1]->toExplicit3D().Y(),m_vertices[t1]->toExplicit3D().Z(),
        m_vertices[t2]->toExplicit3D().X(),m_vertices[t2]->toExplicit3D().Y(),m_vertices[t2]->toExplicit3D().Z());
}
inline int orient3d_ignore_axis(uint32_t p0,uint32_t p1,uint32_t p2,int axis, genericPoint** m_vertices)
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
#endif
