#ifndef common_header_h
#define common_header_h

#include <stdio.h>
#include <utility>
#include <vector>
#include <queue>
#include <stack>
#include <deque>
#include <unordered_set>
#include <map>
#include <unordered_map>
#include <cassert>
#include <numbers>
#include <memory>
#include "geometric_object/double3.h"
#include "indirect_predicates/implicit_point.h"
#include "graph_cut/GCoptimization.h"

struct pair_ii_hash
{
    std::size_t operator()(const std::pair<uint32_t, uint32_t>& p) const
    {
        std::size_t h1 = std::hash<uint32_t>()(p.first);
        std::size_t h2 = std::hash<uint32_t>()(p.second);

        // combine hashes safely
        return h1 ^ (h2 << 1);
    }
};
struct trio_iii_hash
{
    std::size_t operator()(const std::tuple<uint32_t, uint32_t, uint32_t> &x) const
    {
        std::size_t h1 = std::hash<uint32_t>()(std::get<0>(x));
        std::size_t h2 = std::hash<uint32_t>()(std::get<1>(x));
        std::size_t h3 = std::hash<uint32_t>()(std::get<2>(x));

        // combine hashes safely
        return h1 ^ (h2 << 1) ^ (h3 << 2);
    }
};


const uint32_t UNDEFINED_VALUE = 0xFFFFFFFF;
#define COMMON_FIELDS \
    std::vector<uint32_t> m_vector_i_0; \
    std::vector<uint32_t> m_vector_i_1; \
    std::vector<uint32_t> m_vector_i_2; \
    std::vector<uint32_t> m_vector_i_3; \
    std::queue<uint32_t> m_queue_i_0; \
    std::queue<uint32_t> m_queue_i_1; \
    std::deque<uint32_t> m_deque_i_0; \
    std::unordered_set<uint32_t> m_u_set_i_0; \
    std::unordered_set<uint32_t> m_u_set_i_1; \
    std::unordered_set<uint32_t> m_u_set_i_2; \
    std::unordered_set<uint32_t> m_u_set_i_3; \
    std::unordered_set<std::pair<uint32_t,uint32_t>,pair_ii_hash> m_u_set_ii_0; \
    std::unordered_map<uint32_t,uint32_t> m_u_map_i_i_0; \
    std::unordered_map<uint32_t,int> m_u_map_i_si_0; \
    std::unordered_map<std::pair<uint32_t,uint32_t>,uint32_t,pair_ii_hash> m_u_map_ii_i_0; \
    std::unordered_map<std::pair<uint32_t,uint32_t>,std::vector<uint32_t>,pair_ii_hash> m_u_map_ii_vector_i_0; \
    std::unordered_map<std::tuple<uint32_t,uint32_t,uint32_t>,uint32_t,trio_iii_hash> m_u_map_iii_i_0; \
    std::unordered_map<uint32_t,std::tuple<uint32_t,uint32_t,uint32_t>> m_u_map_i_iii_0;

#ifdef _WIN32
    #define LIBRARY_EXPORT __declspec(dllexport)
#else
    #define LIBRARY_EXPORT __attribute__((visibility("default")))
#endif

#endif
