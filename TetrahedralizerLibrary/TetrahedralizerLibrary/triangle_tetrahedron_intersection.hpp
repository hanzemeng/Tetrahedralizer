#ifndef triangle_tetrahedron_intersection_hpp
#define triangle_tetrahedron_intersection_hpp

#include "common_header.h"
#include "common_function.h"

class TriangleTetrahedronIntersection
{
public:
    // <c0,c1,c2> is the triangle, <t0,t1,t2,t3> is the tetrahedron, all right hand oriented
    // 0 no intersection, 1 intersection in exterior, 2 intersection in interior
    // also return the edges of the triangle after getting clipped by the tetrahedron
    static std::pair<int, std::vector<std::shared_ptr<genericPoint>>> triangle_tetrahedron_intersection(std::shared_ptr<genericPoint> pc0, std::shared_ptr<genericPoint> pc1,std::shared_ptr<genericPoint> pc2,std::shared_ptr<genericPoint> pt0,std::shared_ptr<genericPoint> pt1,std::shared_ptr<genericPoint> pt2,std::shared_ptr<genericPoint> pt3);
    static std::pair<int, std::vector<std::shared_ptr<genericPoint>>> triangle_tetrahedron_intersection(uint32_t c0,uint32_t c1,uint32_t c2,uint32_t t0,uint32_t t1,uint32_t t2,uint32_t t3, std::shared_ptr<genericPoint>* vertices);
private:
    class Edge
    {
        public:
        uint32_t e0, e1; // two endpoints
        uint32_t p0, p1, p2, p3, p4, p5; // p0, p1, p2 is a plane, p3, p4, p5 is a plane. The line is the intersection of the two plane. Only use p0, p1 if both are explicit points.

        Edge();
        Edge(uint32_t p); // not an edge, just a point
        Edge(uint32_t p, uint32_t q);
        Edge(uint32_t ie0, uint32_t ie1, uint32_t ip0, uint32_t ip1, uint32_t ip2, uint32_t ip3, uint32_t ip4, uint32_t ip5);
        Edge(const Edge& other);
    };
    
    TriangleTetrahedronIntersection() = delete;
    static void polygon_plane_intersection_helper(uint32_t& i0,uint32_t& i1,uint32_t e);
    static void polygon_plane_intersection(uint32_t c0,uint32_t c1,uint32_t c2,uint32_t t0,uint32_t t1,uint32_t t2, std::vector<std::shared_ptr<genericPoint>>& vertices, std::vector<Edge>& edges);
};

#endif
