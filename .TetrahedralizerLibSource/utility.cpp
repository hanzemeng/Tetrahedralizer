#include "utility.h"

int double_to_int(double d)
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
int orient3d(uint32_t p0,uint32_t p1,uint32_t p2,uint32_t p3)
{
    return double_to_int(genericPoint::orient3D(*m_vertices[p0],*m_vertices[p1],*m_vertices[p2],*m_vertices[p3]));
}
int in_sphere(uint32_t p0,uint32_t p1,uint32_t p2,uint32_t p3,uint32_t p4)
{
    return double_to_int(genericPoint::inSphere(*m_vertices[p0],*m_vertices[p1],*m_vertices[p2],*m_vertices[p3],*m_vertices[p4]));
}
bool is_collinear(uint32_t p0, uint32_t p1, uint32_t p2)
{
    return !genericPoint::misaligned(*m_vertices[p0],*m_vertices[p1],*m_vertices[p2]);
}
bool vertex_in_inner_segment(uint32_t p0,uint32_t s0,uint32_t s1)
{
    return genericPoint::pointInInnerSegment(*m_vertices[p0],*m_vertices[s0],*m_vertices[s1]);
}
bool vertex_in_segment(uint32_t p0,uint32_t s0,uint32_t s1)
{
    return genericPoint::pointInSegment(*m_vertices[p0],*m_vertices[s0],*m_vertices[s1]);
}
bool vertex_in_inner_triangle(uint32_t p0,uint32_t t0,uint32_t t1,uint32_t t2)
{
    return genericPoint::pointInInnerTriangle(*m_vertices[p0],*m_vertices[t0],*m_vertices[t1],*m_vertices[t2]);
}
bool vertex_in_triangle(uint32_t p0,uint32_t t0,uint32_t t1,uint32_t t2)
{
    return genericPoint::pointInTriangle(*m_vertices[p0],*m_vertices[t0],*m_vertices[t1],*m_vertices[t2]);
}
bool inner_segment_cross_inner_triangle(uint32_t s0,uint32_t s1,uint32_t t0,uint32_t t1,uint32_t t2)
{
    return genericPoint::innerSegmentCrossesInnerTriangle(*m_vertices[s0],*m_vertices[s1],*m_vertices[t0],*m_vertices[t1],*m_vertices[t2]);
}
bool inner_segment_cross_triangle(uint32_t s0,uint32_t s1,uint32_t t0,uint32_t t1,uint32_t t2)
{
    return genericPoint::innerSegmentCrossesTriangle(*m_vertices[s0],*m_vertices[s1],*m_vertices[t0],*m_vertices[t1],*m_vertices[t2]);
}
bool inner_segment_cross_inner_segment(uint32_t s0,uint32_t s1,uint32_t s2,uint32_t s3)
{
    return genericPoint::innerSegmentsCross(*m_vertices[s0],*m_vertices[s1],*m_vertices[s2],*m_vertices[s3]);
}
int max_component_in_triangle_normal(uint32_t t0,uint32_t t1,uint32_t t2)
{
    return genericPoint::maxComponentInTriangleNormal(
        m_vertices[t0]->toExplicit3D().X(),m_vertices[t0]->toExplicit3D().Y(),m_vertices[t0]->toExplicit3D().Z(),
        m_vertices[t1]->toExplicit3D().X(),m_vertices[t1]->toExplicit3D().Y(),m_vertices[t1]->toExplicit3D().Z(),
        m_vertices[t2]->toExplicit3D().X(),m_vertices[t2]->toExplicit3D().Y(),m_vertices[t2]->toExplicit3D().Z());
}
int orient3d_ignore_axis(uint32_t p0,uint32_t p1,uint32_t p2,int axis)
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
int orient3d_ignore_axis(const explicitPoint3D& pp0,uint32_t p1,uint32_t p2,int axis)
{
    if(0 == axis)
    {
        return double_to_int(genericPoint::orient2Dyz(pp0,*m_vertices[p1],*m_vertices[p2]));
    }
    if(1 == axis)
    {
        return double_to_int(genericPoint::orient2Dzx(pp0,*m_vertices[p1],*m_vertices[p2]));
    }
    if(2 == axis)
    {
        return double_to_int(genericPoint::orient2Dxy(pp0,*m_vertices[p1],*m_vertices[p2]));
    }
    throw "wrong axis value";
}

bool vertex_in_segment(uint32_t p0,uint32_t s0,uint32_t s1, int axis)
{
    return genericPoint::pointInSegment(*m_vertices[p0],*m_vertices[s0],*m_vertices[s1], axis);
}
bool vertex_in_triangle(uint32_t p0,uint32_t t0,uint32_t t1,uint32_t t2, int axis)
{
    return genericPoint::pointInTriangle(*m_vertices[p0],*m_vertices[t0],*m_vertices[t1],*m_vertices[t2], axis);
}


void sort_ints(uint32_t& i0, uint32_t& i1)
{
    if(i0 > i1)
    {
        swap(i0,i1);
    }
}
void sort_ints(uint32_t& i0, uint32_t& i1, uint32_t& i2)
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
void sort_ints(int& i0, int& i1)
{
    if(i0 > i1)
    {
        swap(i0,i1);
    }
}

void clear_queue(queue<uint32_t>& q)
{
    while(!q.empty())
    {
        q.pop();
    }
}
void clear_stack(stack<uint32_t>& s)
{
    while(!s.empty())
    {
        s.pop();
    }
}



void get_tetrahedron_face(uint32_t t, uint32_t i, uint32_t& f0,uint32_t& f1,uint32_t& f2)
{
    switch(i)
    {
        case 0:
            f0 = m_tetrahedrons[t+0];
            f1 = m_tetrahedrons[t+1];
            f2 = m_tetrahedrons[t+2];
            break;
        case 1:
            f0 = m_tetrahedrons[t+1];
            f1 = m_tetrahedrons[t+0];
            f2 = m_tetrahedrons[t+3];
            break;
        case 2:
            f0 = m_tetrahedrons[t+0];
            f1 = m_tetrahedrons[t+2];
            f2 = m_tetrahedrons[t+3];
            break;
        case 3:
            f0 = m_tetrahedrons[t+2];
            f1 = m_tetrahedrons[t+1];
            f2 = m_tetrahedrons[t+3];
            break;
        default:
            throw "wrong face index";
    }
}
void get_tetrahedron_face(uint32_t t, uint32_t& f0,uint32_t& f1,uint32_t& f2)
{
    get_tetrahedron_face(t&0xfffffffc,t&3,f0,f1,f2);
}
void get_tetrahedron_opposite_vertex(uint32_t t, uint32_t& p)
{
    uint32_t i = t & 3;
    t = t & 0xfffffffc;
    switch(i)
    {
        case 0:
            p = m_tetrahedrons[t+3];
            break;
        case 1:
            p = m_tetrahedrons[t+2];
            break;
        case 2:
            p = m_tetrahedrons[t+1];
            break;
        case 3:
            p = m_tetrahedrons[t+0];
            break;
        default:
            throw "wrong face index";
    }
}

void get_tetrahedron_neighbor(uint32_t t, uint32_t i, uint32_t& n)
{
    //cout << t << ", " << i << endl;
    
    if(UNDEFINED_VALUE == m_neighbors[t+i])
    {
        n = UNDEFINED_VALUE;
    }
    else
    {
        n = m_neighbors[t+i] & 0xfffffffc;
    }
}

bool has_vertex(uint32_t t, uint32_t v)
{
    return v == m_tetrahedrons[t+0] || v == m_tetrahedrons[t+1] || v == m_tetrahedrons[t+2] || v == m_tetrahedrons[t+3];
}

bool edge_intersects_triangle(uint32_t e0,uint32_t e1,uint32_t t0,uint32_t t1,uint32_t t2, uint32_t& o0)
{
    int oe01t01 = orient3d(e0,e1,t0,t1);
    int oe01t12 = orient3d(e0,e1,t1,t2);
    int oe01t20 = orient3d(e0,e1,t2,t0);
    if(oe01t01 != oe01t12 || oe01t12 != oe01t20)
    {
        return false;
    }
    int oe0t012 = orient3d(e0,t0,t1,t2);
    int oe1t012 = orient3d(e1,t0,t1,t2);
    o0 = UNDEFINED_VALUE;
    if(0 == oe0t012)
    {
        o0 = e0;
    }
    if(0 == oe1t012)
    {
        o0 = e1;
    }
    if(0 == oe0t012 && 0 == oe1t012)
    {
        o0 = UNDEFINED_VALUE;
        return true;
    }
    return oe0t012 != oe1t012;
}

void add_tetrahedron_incident(uint32_t p0, unordered_set<uint32_t>& visited, queue<uint32_t>& to_be_visited, unordered_set<uint32_t>& res_set, queue<uint32_t>& res_queue)
{
    visited.clear();
    visited.insert(UNDEFINED_VALUE);
    clear_queue(to_be_visited);
    to_be_visited.push(m_vertices_incidents[p0]);
    while(!to_be_visited.empty())
    {
        uint32_t t = to_be_visited.front();
        to_be_visited.pop();
        if(visited.end() != visited.find(t))
        {
            continue;
        }
        visited.insert(t);

        if(INFINITE_VERTEX == m_tetrahedrons[t+3])
        {
            continue;
        }
        if(!has_vertex(t,p0))
        {
            continue;
        }
        res_set.insert(t);
        res_queue.push(t);
        uint32_t n;
        for(uint32_t f=0; f<4; f++)
        {
            get_tetrahedron_neighbor(t, f, n);
            to_be_visited.push(n);
        }
    }
}
void add_tetrahedron_incident(uint32_t p0,uint32_t p1, unordered_set<uint32_t>& visited, queue<uint32_t>& to_be_visited, unordered_set<uint32_t>& res_set, queue<uint32_t>& res_queue)
{
    visited.clear();
    visited.insert(UNDEFINED_VALUE);
    clear_queue(to_be_visited);
    to_be_visited.push(m_vertices_incidents[p0]);
    while(!to_be_visited.empty())
    {
        uint32_t t = to_be_visited.front();
        to_be_visited.pop();
        if(visited.end() != visited.find(t))
        {
            continue;
        }
        visited.insert(t);

        if(INFINITE_VERTEX == m_tetrahedrons[t+3])
        {
            continue;
        }
        bool b0 = has_vertex(t,p0);
        bool b1 = has_vertex(t,p1);
        if(!b0 && !b1)
        {
            continue;
        }
        if(b0 && b1)
        {
            res_set.insert(t);
            res_queue.push(t);
        }
        uint32_t n;
        for(uint32_t f=0; f<4; f++)
        {
            get_tetrahedron_neighbor(t, f, n);
            to_be_visited.push(n);
        }
    }
}
