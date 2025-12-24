#ifndef triangle_tetrahedron_intersection_hpp
#define triangle_tetrahedron_intersection_hpp

#include "common_header.h"
#include "common_function.h"
#include "geometric_object/segment.h"
#include "geometric_object/facet.h"

inline int triangle_tetrahedron_intersection(std::vector<std::shared_ptr<genericPoint>>& vertices, std::vector<Segment>& segments, std::vector<Facet>& facets)
{
    std::vector<uint32_t> top_segments = facets[0].segments;
    uint32_t top_vertex = UNDEFINED_VALUE;
    
    for(uint32_t i=1; i<5; i++)
    {
        uint32_t c0 = facets[i].p0;
        uint32_t c1 = facets[i].p1;
        uint32_t c2 = facets[i].p2;
        
        if(top_vertex != UNDEFINED_VALUE)
        {
            if(-1 == orient3d(c0,c1,c2,top_vertex,vertices.data()))
            {
                return 0;
            }
            continue;
        }
        
        std::unordered_map<uint32_t, int> orientation_cache;
        bool has_coplanar_segment = false;
        uint32_t i0(UNDEFINED_VALUE), i1(UNDEFINED_VALUE);
        std::vector<uint32_t> new_top_segments;
        for(uint32_t s : top_segments)
        {
            auto [i_p, top_s, bot_s, vs] = Segment::slice_segment_with_plane(s, c0, c1, c2, vertices, segments, orientation_cache);
            
            if(UNDEFINED_VALUE == i_p && top_s == UNDEFINED_VALUE && bot_s == UNDEFINED_VALUE)
            {
                has_coplanar_segment = true;
                new_top_segments.push_back(s);
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
                new_top_segments.push_back(top_s);
            }
        }
        
        if(!has_coplanar_segment && i1 != UNDEFINED_VALUE)
        {
            uint32_t i_e = segments.size();
            segments.push_back(Segment(i0,i1,facets[0].p0,facets[0].p1,facets[0].p2,c0,c1,c2));
            new_top_segments.push_back(i_e);
        }
        else
        {
            if(0 == new_top_segments.size())
            {
                top_vertex = i0;
            }
        }
        top_segments = new_top_segments;
    }
    
    if(UNDEFINED_VALUE != top_vertex)
    {
        return 1;
    }
    if(0 == top_segments.size())
    {
        return 0;
    }
    else if(top_segments.size() < 3)
    {
        return 1;
    }
    
    // check if the triangle is coplanar with a tetrahedron facet
    for(uint32_t i=1; i<5; i++)
    {
        if(facets[0].ip0 == facets[i].ip0)
        {
            return 1;
        }
    }
    facets[0].segments = top_segments;
    return 2;
}

#endif
