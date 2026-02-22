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
#include <random>
#include <fstream>
#include <thread>
#include <chrono>
#include "geometric_object/double3.h"
#include "indirect_predicates/implicit_point.h"

struct ii32_hash
{
    std::size_t operator()(const std::pair<uint32_t, uint32_t>& p) const
    {
        std::size_t h1 = std::hash<uint32_t>()(p.first);
        std::size_t h2 = std::hash<uint32_t>()(p.second);

        return h1 ^ (h2 << 1);
    }
};
struct iii32_hash
{
    std::size_t operator()(const std::tuple<uint32_t, uint32_t, uint32_t> &x) const
    {
        std::size_t h1 = std::hash<uint32_t>()(std::get<0>(x));
        std::size_t h2 = std::hash<uint32_t>()(std::get<1>(x));
        std::size_t h3 = std::hash<uint32_t>()(std::get<2>(x));

        return h1 ^ (h2 << 1) ^ (h3 << 2);
    }
};
struct iiii32_hash
{
    std::size_t operator()(const std::tuple<uint32_t, uint32_t, uint32_t, uint32_t> &x) const
    {
        std::size_t h1 = std::hash<uint32_t>()(std::get<0>(x));
        std::size_t h2 = std::hash<uint32_t>()(std::get<1>(x));
        std::size_t h3 = std::hash<uint32_t>()(std::get<2>(x));
        std::size_t h4 = std::hash<uint32_t>()(std::get<3>(x));

        return h1 ^ (h2 << 1) ^ (h3 << 2) ^ (h4 << 3);
    }
};
struct iiii64_hash
{
    std::size_t operator()(const std::tuple<uint32_t, uint32_t, uint32_t, uint32_t> &x) const
    {
        std::size_t h1 = std::hash<uint64_t>()(std::get<0>(x));
        std::size_t h2 = std::hash<uint64_t>()(std::get<1>(x));
        std::size_t h3 = std::hash<uint64_t>()(std::get<2>(x));
        std::size_t h4 = std::hash<uint64_t>()(std::get<3>(x));

        return h1 ^ (h2 << 1) ^ (h3 << 2) ^ (h4 << 3);
    }
};

const uint32_t UNDEFINED_VALUE = 0xFFFFFFFF;

class uint32_t9
{
public:
    uint32_t p0,p1,p2,p3,p4,p5,p6,p7,p8;

    
    uint32_t9()
    {
        p0 = UNDEFINED_VALUE;
        p1 = UNDEFINED_VALUE;
        p2 = UNDEFINED_VALUE;
        p3 = UNDEFINED_VALUE;
        p4 = UNDEFINED_VALUE;
        p5 = UNDEFINED_VALUE;
        p6 = UNDEFINED_VALUE;
        p7 = UNDEFINED_VALUE;
        p8 = UNDEFINED_VALUE;
    }
    void push_back_to_vector(std::vector<uint32_t>& vec)
    {
        if(UNDEFINED_VALUE != p5)
        {
            vec.push_back(9);
            vec.push_back(p0);
            vec.push_back(p1);
            vec.push_back(p2);
            vec.push_back(p3);
            vec.push_back(p4);
            vec.push_back(p5);
            vec.push_back(p6);
            vec.push_back(p7);
            vec.push_back(p8);
        }
        else
        {
            vec.push_back(5);
            vec.push_back(p0);
            vec.push_back(p1);
            vec.push_back(p2);
            vec.push_back(p3);
            vec.push_back(p4);
        }
    }
};

#ifdef _WIN32
    #define LIBRARY_EXPORT __declspec(dllexport)
#else
    #define LIBRARY_EXPORT __attribute__((visibility("default")))
#endif

#endif
