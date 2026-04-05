cmake_minimum_required(VERSION 3.16)

project(Tetrahedralizer
    VERSION 1.0
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(Tetrahedralizer
    src/quickhull.cpp
    src/facets_ordering.cpp
    src/convex_hull_partition.cpp
    src/generic_point_approximation.cpp
    src/generic_point_predicate.cpp
    src/geometric_object/polyhedralization.cpp
    src/geometric_object/tetrahedralization.cpp
    src/graph_cut/GCoptimization.cpp
    src/graph_cut/LinkedBlockList.cpp
    src/graph_cut/graph.cpp
    src/graph_cut/maxflow.cpp
    src/winding_number_approximation.cpp
    src/interior_characterization.cpp
    src/polyhedralization_creation.cpp
    src/main.cpp
    src/polyhedralization_tetrahedralization.cpp
)

set(CMAKE_CXX_FLAGS_RELEASE "-O3")
set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-O2")
set(CMAKE_CXX_FLAGS_MINSIZEREL "-Os")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -frounding-math")


# cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
# cmake --build build
# /usr/bin/time -v ./Tetrahedralizer/TetrahedralizerLibrary/build/Tetrahedralizer ./thingi10k/32770.off
# /usr/bin/time -v ./Tetrahedralizer/TetrahedralizerLibrary/build/Tetrahedralizer ./thingi10k/32770.off > ./thingi10k_report/32770.txt 2>&1
# nohup python3 thingi10k_test.py &