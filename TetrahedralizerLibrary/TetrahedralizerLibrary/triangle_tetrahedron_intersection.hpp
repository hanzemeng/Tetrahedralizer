#ifndef triangle_tetrahedron_intersection_hpp
#define triangle_tetrahedron_intersection_hpp

#include "common_header.h"
#include "common_function.h"
#include "geometric_object/segment.h"
#include "geometric_object/facet.h"

class TriangleTetrahedronIntersection
{
public:
    // 0 no intersection, 1 intersection in exterior, 2 intersection in interior
    // facets[0] is the triangle, the rest are tetrahedron facets.
    // vertices and segments will be appended. if return is 2, facets[0] will be replaced with the sliced polygon
    static int triangle_tetrahedron_intersection(std::vector<std::shared_ptr<genericPoint>>& vertices, std::vector<Segment>& segments, std::vector<Facet>& facets);
private:
    TriangleTetrahedronIntersection() = delete;
};

#endif
