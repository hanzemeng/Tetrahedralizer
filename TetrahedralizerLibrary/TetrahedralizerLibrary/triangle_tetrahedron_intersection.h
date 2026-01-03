#ifndef triangle_tetrahedron_intersection_hpp
#define triangle_tetrahedron_intersection_hpp

#include "common_header.h"
#include "common_function.h"
#include "geometric_object/segment.h"
#include "geometric_object/facet.h"

inline bool triangle_tetrahedron_intersection_coarse(std::vector<std::shared_ptr<genericPoint>>& vertices, uint32_t c0, uint32_t c1, uint32_t c2, uint32_t cg, uint32_t t0,uint32_t t1,uint32_t t2,uint32_t t3, std::unordered_map<std::pair<uint32_t,uint32_t>, int, ii32_hash>& orient_cache)
{
    auto get_orient = [&](uint32_t p) -> int
    {
        auto it = orient_cache.find(std::make_pair(cg,p));
        if(orient_cache.end() == it)
        {
            int o = orient3d(c0,c1,c2,p,vertices.data());
            orient_cache[std::make_pair(cg,p)] = o;
            return o;
        }
        return it->second;
    };
    
    int o0 = get_orient(t0);
    int o1 = get_orient(t1);
    int o2 = get_orient(t2);
    int o3 = get_orient(t3);
    
    return (o0<0||o1<0||o2<0||o3<0) && (o0>0||o1>0||o2>0||o3>0);
}

// 0th plane is the triangle plane
inline std::pair<int, std::vector<uint32_t>> triangle_tetrahedron_intersection(std::vector<std::shared_ptr<genericPoint>>& vertices, std::vector<Segment>& segments, std::vector<uint32_t>& planes, std::vector<uint32_t>& planes_groups)
{
    uint32_t t0 = planes[0];
    uint32_t t1 = planes[1];
    uint32_t t2 = planes[2];
    std::vector<uint32_t> facet = std::vector<uint32_t>{0,1,2};
    uint32_t top_vertex = UNDEFINED_VALUE;
    for(uint32_t i=1; i<5; i++)
    {
        uint32_t c0 = planes[3*i+0];
        uint32_t c1 = planes[3*i+1];
        uint32_t c2 = planes[3*i+2];
        
        if(top_vertex != UNDEFINED_VALUE)
        {
            if(-1 == orient3d(c0,c1,c2,top_vertex,vertices.data()))
            {
                return std::make_pair(0, facet);
            }
            continue;
        }
        
        std::unordered_map<uint32_t, int> orientation_cache;
        bool has_coplanar_segment = false;
        uint32_t i0(UNDEFINED_VALUE), i1(UNDEFINED_VALUE);
        std::vector<uint32_t> new_facet;
        for(uint32_t s : facet)
        {
            auto [i_p, top_s, bot_s, vs] = Segment::slice_segment_with_plane(s, c0, c1, c2, vertices, segments, orientation_cache);
            
            if(UNDEFINED_VALUE == i_p && top_s == UNDEFINED_VALUE && bot_s == UNDEFINED_VALUE)
            {
                has_coplanar_segment = true;
                new_facet.push_back(s);
                continue;
            }
            
            if(UNDEFINED_VALUE != i_p)
            {
                if(UNDEFINED_VALUE == i0 || i_p == i0)
                {
                    i0 = i_p;
                }
                else
                {
                    i1 = i_p;
                }
            }
            if(UNDEFINED_VALUE != top_s)
            {
                new_facet.push_back(top_s);
            }
        }
        
        if(!has_coplanar_segment && i1 != UNDEFINED_VALUE)
        {
            uint32_t i_e = segments.size();
            segments.push_back(Segment(i0,i1,t0,t1,t2,c0,c1,c2));
            new_facet.push_back(i_e);
        }
        else
        {
            if(0 == new_facet.size())
            {
                top_vertex = i0;
            }
        }
        facet = std::move(new_facet);
    }
    
    if(UNDEFINED_VALUE != top_vertex)
    {
        return std::make_pair(1, facet);
    }
    if(0 == facet.size())
    {
        return std::make_pair(0, facet);
    }
    else if(facet.size() < 3)
    {
        return std::make_pair(1, facet);
    }
    
    // check if the triangle is coplanar with a tetrahedron facet
    for(uint32_t i=1; i<5; i++)
    {
        if(planes_groups[0] == planes_groups[i])
        {
            return std::make_pair(1, facet);
        }
    }
    
    return std::make_pair(2, facet);
}

#endif
