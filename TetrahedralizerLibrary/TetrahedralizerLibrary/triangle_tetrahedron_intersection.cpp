#include "triangle_tetrahedron_intersection.hpp"
using namespace std;



void TriangleTetrahedronIntersection::polygon_plane_intersection_helper(uint32_t& i0,uint32_t& i1,uint32_t e)
{
    if(UNDEFINED_VALUE == i0 || e == i0)
    {
        i0 = e;
    }
    else if(UNDEFINED_VALUE == i1 || e == i1)
    {
        i1 = e;
    }
}
// slice edges, will insert vertices at intersections, will keep edges that are on or above the plane.
void TriangleTetrahedronIntersection::polygon_plane_intersection(uint32_t c0,uint32_t c1,uint32_t c2,uint32_t t0,uint32_t t1,uint32_t t2, vector<shared_ptr<genericPoint>>& vertices, vector<Segment>& edges)
{
    if(0 == edges.size())
    {
        return;
    }
    // just a point
    if(1 == edges.size() && UNDEFINED_VALUE == edges[0].e1)
    {
        if(-1 == orient3d(t0,t1,t2, edges[0].e0, vertices.data()))
        {
            edges.pop_back();
        }
        return;
    }
    
    auto [vertex, top_segments, bot_segments] = Segment::slice_segments_with_plane(edges, c0, c1, c2, t0, t1, t2, vertices);
    
    edges = top_segments;
    if(0 == top_segments.size() && UNDEFINED_VALUE != vertex)
    {
        edges.push_back(Segment(vertex)); // just add a point
    }
}

pair<int, vector<Segment>> TriangleTetrahedronIntersection::triangle_tetrahedron_intersection(uint32_t c0,uint32_t c1,uint32_t c2,uint32_t t0,uint32_t t1,uint32_t t2,uint32_t t3, vector<shared_ptr<genericPoint>>& vertices)
{
    vector<Segment> edges;
    edges.push_back(Segment(c0,c1));
    edges.push_back(Segment(c1,c2));
    edges.push_back(Segment(c2,c0));

    polygon_plane_intersection(c0,c1,c2,t0,t1,t2,vertices,edges);
    polygon_plane_intersection(c0,c1,c2,t1,t0,t3,vertices,edges);
    polygon_plane_intersection(c0,c1,c2,t0,t2,t3,vertices,edges);
    polygon_plane_intersection(c0,c1,c2,t2,t1,t3,vertices,edges);

    int res = 0;
    if(0 == edges.size())
    {
        res = 0;
    }
    else if(edges.size() < 3)
    {
        res = 1;
    }
    else
    {
        // check if the triangle is coplanar with a tetrahedron facet
        int oc0t012 = orient3d(t0, t1, t2, c0, vertices.data());
        int oc1t012 = orient3d(t0, t1, t2, c1, vertices.data());
        int oc2t012 = orient3d(t0, t1, t2, c2, vertices.data());
        int oc0t103 = orient3d(t1, t0, t3, c0, vertices.data());
        int oc1t103 = orient3d(t1, t0, t3, c1, vertices.data());
        int oc2t103 = orient3d(t1, t0, t3, c2, vertices.data());
        int oc0t023 = orient3d(t0, t2, t3, c0, vertices.data());
        int oc1t023 = orient3d(t0, t2, t3, c1, vertices.data());
        int oc2t023 = orient3d(t0, t2, t3, c2, vertices.data());
        int oc0t213 = orient3d(t2, t1, t3, c0, vertices.data());
        int oc1t213 = orient3d(t2, t1, t3, c1, vertices.data());
        int oc2t213 = orient3d(t2, t1, t3, c2, vertices.data());
        if((0==oc0t012 && oc0t012==oc1t012 && oc0t012==oc2t012) ||
           (0==oc0t103 && oc0t103==oc1t103 && oc0t103==oc2t103) ||
           (0==oc0t023 && oc0t023==oc1t023 && oc0t023==oc2t023) ||
           (0==oc0t213 && oc0t213==oc1t213 && oc0t213==oc2t213))
        {
            res = 1;
        }
        else
        {
            res = 2;
        }
    }
    return make_pair(res, edges);
}

