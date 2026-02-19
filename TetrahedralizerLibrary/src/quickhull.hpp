#ifndef quickhull_hpp
#define quickhull_hpp

#include "common_header.h"
#include "common_function.h"

class Quickhull
{
public:
    std::vector<uint32_t> calculate(std::vector<std::shared_ptr<genericPoint>>& vertices, std::vector<double3>& approximated_vertices);
};

#endif
