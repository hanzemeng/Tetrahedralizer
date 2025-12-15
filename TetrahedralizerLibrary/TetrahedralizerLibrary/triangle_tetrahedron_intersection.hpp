#ifndef triangle_tetrahedron_intersection_hpp
#define triangle_tetrahedron_intersection_hpp

#include "common_header.h"
#include "common_function.h"
#include "geometric_object/segment.h"

class TriangleTetrahedronIntersection
{
public:
    // <c0,c1,c2> is the triangle, <t0,t1,t2,t3> is the tetrahedron, all right hand oriented
    // 0 no intersection, 1 intersection in exterior, 2 intersection in interior
    // also returns the edges of the triangle after getting clipped by the tetrahedron and the points used to form the edge
    static std::pair<int, std::vector<Segment>> triangle_tetrahedron_intersection(uint32_t c0,uint32_t c1,uint32_t c2,uint32_t t0,uint32_t t1,uint32_t t2,uint32_t t3, std::vector<std::shared_ptr<genericPoint>>& vertices);
private:
    
    
    TriangleTetrahedronIntersection() = delete;
    static void polygon_plane_intersection_helper(uint32_t& i0,uint32_t& i1,uint32_t e);
    static void polygon_plane_intersection(uint32_t c0,uint32_t c1,uint32_t c2,uint32_t t0,uint32_t t1,uint32_t t2, std::vector<std::shared_ptr<genericPoint>>& vertices, std::vector<Segment>& edges);
};

#endif
