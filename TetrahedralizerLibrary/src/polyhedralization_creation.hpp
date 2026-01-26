#ifndef polyhedralization_creation_hpp
#define polyhedralization_creation_hpp

#include "common_header.h"
#include "common_function.h"
#include "geometric_object/facet.h"
#include "geometric_object/segment.h"
#include "delaunay_tetrahedralization.hpp"
#include "convex_hull_partition.hpp"

class PolyhedralizationCreationHandle
{
public:
    // vertices are unique
    // constraints may have duplicate or be degenerate, such constraints are skipped
    void calculate(std::vector<std::shared_ptr<genericPoint>>& vertices, std::vector<uint32_t>& constraints);
};

#endif
