#ifndef triangle_tetrahedron_intersection_h
#define triangle_tetrahedron_intersection_h

#include "triangle_tetrahedron_intersection.hpp"
using namespace std;

TriangleTetrahedronIntersection::Edge::Edge()
{
    e0 = e1 = p0 = p1 = p2 = p3 = p4 = p5 = UNDEFINED_VALUE;
}
TriangleTetrahedronIntersection::Edge::Edge(uint32_t p) // not an edge, just a point
{
    e0 = p;
    e1 = p0 = p1 = p2 = p3 = p4 = p5 = UNDEFINED_VALUE;
}
TriangleTetrahedronIntersection::Edge::Edge(uint32_t p, uint32_t q)
{
    e0 = p;
    e1 = q;
    p0 = p;
    p1 = q;
    p2 = p3 = p4 = p5 = UNDEFINED_VALUE;
}
TriangleTetrahedronIntersection::Edge::Edge(uint32_t ie0, uint32_t ie1, uint32_t ip0, uint32_t ip1, uint32_t ip2, uint32_t ip3, uint32_t ip4, uint32_t ip5)
{
    e0 = ie0;
    e1 = ie1;
    p0 = ip0;
    p1 = ip1;
    p2 = ip2;
    p3 = ip3;
    p4 = ip4;
    p5 = ip5;
}
TriangleTetrahedronIntersection::Edge::Edge(const Edge& other)
{
    e0 = other.e0;
    e1 = other.e1;
    p0 = other.p0;
    p1 = other.p1;
    p2 = other.p2;
    p3 = other.p3;
    p4 = other.p4;
    p5 = other.p5;
}

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
void TriangleTetrahedronIntersection::polygon_plane_intersection(uint32_t c0,uint32_t c1,uint32_t c2,uint32_t t0,uint32_t t1,uint32_t t2, vector<shared_ptr<genericPoint>>& vertices, vector<Edge>& edges)
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
    
    uint32_t i0(UNDEFINED_VALUE),i1(UNDEFINED_VALUE); // store new edge
    vector<Edge> new_edges;
    for(uint32_t i=0; i<edges.size(); i++)
    {
        uint32_t e0 = edges[i].e0;
        uint32_t e1 = edges[i].e1;
        int o0 = orient3d(t0,t1,t2, e0, vertices.data());
        int o1 = orient3d(t0,t1,t2, e1, vertices.data());
        
        if(0 == o0)
        {
            polygon_plane_intersection_helper(i0,i1,e0);
            if(0 == o1)
            {
                polygon_plane_intersection_helper(i0,i1,e1);
                new_edges.push_back(edges[i]);
            }
            else if(1 == o1)
            {
                new_edges.push_back(edges[i]);
            }
        }
        else if(1 == o0)
        {
            if(0 == o1)
            {
                polygon_plane_intersection_helper(i0,i1,e1);
                new_edges.push_back(edges[i]);
            }
            else if(1 == o1)
            {
                new_edges.push_back(edges[i]);
            }
            else
            {
                uint32_t new_i = vertices.size();
                if(UNDEFINED_VALUE == edges[i].p2)
                {
                    vertices.push_back(make_shared<implicitPoint3D_LPI>(
                                                                             vertices[edges[i].p0]->toExplicit3D(),vertices[edges[i].p1]->toExplicit3D(),
                                                                             vertices[t0]->toExplicit3D(),vertices[t1]->toExplicit3D(),vertices[t2]->toExplicit3D()));
                }
                else
                {
                    vertices.push_back(make_shared<implicitPoint3D_TPI>(
                                                                             vertices[t0]->toExplicit3D(),vertices[t1]->toExplicit3D(),vertices[t2]->toExplicit3D(),
                                                                             vertices[edges[i].p0]->toExplicit3D(),vertices[edges[i].p1]->toExplicit3D(),vertices[edges[i].p2]->toExplicit3D(),
                                                                             vertices[edges[i].p3]->toExplicit3D(),vertices[edges[i].p4]->toExplicit3D(),vertices[edges[i].p5]->toExplicit3D()));
                }
                
                polygon_plane_intersection_helper(i0,i1,new_i);
                Edge new_edge(edges[i]);
                new_edge.e0 = e0;
                new_edge.e1 = new_i;
                new_edges.push_back(new_edge);
            }
        }
        else
        {
            if(0 == o1)
            {
                polygon_plane_intersection_helper(i0,i1,e1);
            }
            else if(1 == o1)
            {
                uint32_t new_i = vertices.size();
                if(UNDEFINED_VALUE == edges[i].p2)
                {
                    vertices.push_back(make_shared<implicitPoint3D_LPI>(
                                                                             vertices[edges[i].p0]->toExplicit3D(),vertices[edges[i].p1]->toExplicit3D(),
                                                                             vertices[t0]->toExplicit3D(),vertices[t1]->toExplicit3D(),vertices[t2]->toExplicit3D()));
                }
                else
                {
                    vertices.push_back(make_shared<implicitPoint3D_TPI>(
                                                                             vertices[t0]->toExplicit3D(),vertices[t1]->toExplicit3D(),vertices[t2]->toExplicit3D(),
                                                                             vertices[edges[i].p0]->toExplicit3D(),vertices[edges[i].p1]->toExplicit3D(),vertices[edges[i].p2]->toExplicit3D(),
                                                                             vertices[edges[i].p3]->toExplicit3D(),vertices[edges[i].p4]->toExplicit3D(),vertices[edges[i].p5]->toExplicit3D()));
                }
                polygon_plane_intersection_helper(i0,i1,new_i);
                Edge new_edge(edges[i]);
                new_edge.e0 = e1;
                new_edge.e1 = new_i;
                new_edges.push_back(new_edge);
            }
        }
    }
    
    if(UNDEFINED_VALUE == i1)
    {
        if(UNDEFINED_VALUE != i0 && 0 == new_edges.size())
        {
            new_edges.push_back(Edge(i0));
        }
    }
    else
    {
        bool should_add = true;
        for(uint32_t i=0; i<new_edges.size(); i++)
        {
            if((i0 == new_edges[i].e0 || i0 == new_edges[i].e1) && (i1 == new_edges[i].e0 || i1 == new_edges[i].e1))
            {
                should_add = false;
                break;
            }
        }
        
        if(should_add)
        {
            new_edges.push_back(Edge(i0,i1,c0,c1,c2,t0,t1,t2));
        }
    }
    
    edges = new_edges;
}

pair<int, vector<shared_ptr<genericPoint>>> TriangleTetrahedronIntersection::triangle_tetrahedron_intersection(shared_ptr<genericPoint> pc0, shared_ptr<genericPoint> pc1,shared_ptr<genericPoint> pc2,shared_ptr<genericPoint> pt0,shared_ptr<genericPoint> pt1,shared_ptr<genericPoint> pt2,shared_ptr<genericPoint> pt3)
{
    int res = 0;
    vector<shared_ptr<genericPoint>> vertices;
    vertices.push_back(pc0);
    vertices.push_back(pc1);
    vertices.push_back(pc2);
    vertices.push_back(pt0);
    vertices.push_back(pt1);
    vertices.push_back(pt2);
    vertices.push_back(pt3);
    int c0(0),c1(1),c2(2),t0(3),t1(4),t2(5),t3(6);
    vector<Edge> edges;
    edges.push_back(Edge(c0,c1));
    edges.push_back(Edge(c1,c2));
    edges.push_back(Edge(c2,c0));
    
//
//    // vertex inside
//    if((1==oc0t012 && oc0t012==oc0t103 && oc0t012==oc0t023 && oc0t012==oc0t213) ||
//       (1==oc1t012 && oc1t012==oc1t103 && oc1t012==oc1t023 && oc1t012==oc1t213) ||
//       (1==oc2t012 && oc2t012==oc2t103 && oc2t012==oc2t023 && oc2t012==oc2t213))
//    {
//        res = 2;
//        goto EXIT;
//    }
    // coplanar triangle and tetrahedron's facet
//    if((0==oc0t012 && oc0t012==oc1t012 && oc0t012==oc2t012) ||
//       (0==oc0t103 && oc0t103==oc1t103 && oc0t103==oc2t103) ||
//       (0==oc0t023 && oc0t023==oc1t023 && oc0t023==oc2t023) ||
//       (0==oc0t213 && oc0t213==oc1t213 && oc0t213==oc2t213))
//    {
//        res =
//        segment_cross_triangle(t0,t1,c0,c1,c2,vertices.data()) ||
//        segment_cross_triangle(t0,t2,c0,c1,c2,vertices.data()) ||
//        segment_cross_triangle(t0,t3,c0,c1,c2,vertices.data()) ||
//        segment_cross_triangle(t1,t2,c0,c1,c2,vertices.data()) ||
//        segment_cross_triangle(t1,t3,c0,c1,c2,vertices.data()) ||
//        segment_cross_triangle(t2,t3,c0,c1,c2,vertices.data()) ||
//        segment_cross_triangle(c0,c1,t1,t2,t3,vertices.data()) ||
//        segment_cross_triangle(c1,c2,t1,t2,t3,vertices.data()) ||
//        segment_cross_triangle(c2,c0,t1,t2,t3,vertices.data()) ||
//        segment_cross_triangle(c0,c1,t0,t2,t3,vertices.data()) ||
//        segment_cross_triangle(c1,c2,t0,t2,t3,vertices.data()) ||
//        segment_cross_triangle(c2,c0,t0,t2,t3,vertices.data()) ||
//        segment_cross_triangle(c0,c1,t0,t1,t3,vertices.data()) ||
//        segment_cross_triangle(c1,c2,t0,t1,t3,vertices.data()) ||
//        segment_cross_triangle(c2,c0,t0,t1,t3,vertices.data()) ||
//        segment_cross_triangle(c0,c1,t0,t1,t2,vertices.data()) ||
//        segment_cross_triangle(c1,c2,t0,t1,t2,vertices.data()) ||
//        segment_cross_triangle(c2,c0,t0,t1,t2,vertices.data());
//        goto EXIT;
//    }

    polygon_plane_intersection(c0,c1,c2,t0,t1,t2,vertices,edges);
    polygon_plane_intersection(c0,c1,c2,t1,t0,t3,vertices,edges);
    polygon_plane_intersection(c0,c1,c2,t0,t2,t3,vertices,edges);
    polygon_plane_intersection(c0,c1,c2,t2,t1,t3,vertices,edges);

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
    vector<bool> keep_vertices = vector<bool>(vertices.size(), false);
    for(uint32_t i=0; i<edges.size(); i++)
    {
        keep_vertices[edges[i].e0] = true;
        if(UNDEFINED_VALUE != edges[i].e1)
        {
            keep_vertices[edges[i].e1] = true;
        }
    }
    for(uint32_t i=0; i<vertices.size(); i++)
    {
        if(keep_vertices[i])
        {
            continue;
        }
        
        uint32_t j = vertices.size()-1;
        swap(vertices[i],vertices[j]);
        swap(keep_vertices[i],keep_vertices[j]);
        vertices.pop_back();
        keep_vertices.pop_back();
        i--;
    }
    return make_pair(res, vertices);
}

pair<int, vector<shared_ptr<genericPoint>>> TriangleTetrahedronIntersection::triangle_tetrahedron_intersection(uint32_t c0,uint32_t c1,uint32_t c2,uint32_t t0,uint32_t t1,uint32_t t2,uint32_t t3, shared_ptr<genericPoint>* vertices)
{
    return triangle_tetrahedron_intersection(vertices[c0],vertices[c1],vertices[c2],vertices[t0],vertices[t1],vertices[t2],vertices[t3]);
}

#endif
