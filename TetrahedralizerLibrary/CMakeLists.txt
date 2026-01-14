cmake_minimum_required(VERSION 3.16)

project(Tetrahedralizer
    VERSION 1.0
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(Tetrahedralizer
    src/binary_space_partition.cpp
    src/convex_hull_partition.cpp
    src/delaunay_tetrahedralization.cpp
    src/facet_partition.cpp
    src/generic_point_approximation.cpp
    src/generic_point_predicate.cpp
    src/geometric_object/polyhedralization.cpp
    src/geometric_object/tetrahedralization.cpp
    src/graph_cut/GCoptimization.cpp
    src/graph_cut/LinkedBlockList.cpp
    src/graph_cut/graph.cpp
    src/graph_cut/maxflow.cpp
    src/interior_characterization.cpp
    src/main.cpp
    src/polyhedralization_tetrahedralization.cpp
)

set(CMAKE_CXX_FLAGS_RELEASE "-O3")
set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-O2")
set(CMAKE_CXX_FLAGS_MINSIZEREL "-Os")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -ffp-model=strict")
