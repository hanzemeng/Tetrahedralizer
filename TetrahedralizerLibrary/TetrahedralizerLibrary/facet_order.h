#ifndef facet_order_h
#define facet_order_h

#include "common_header.h"
#include "common_function.h"
#include "geometric_object/segment.h"
#include "geometric_object/facet.h"

class FacetOrder
{
public:
    uint32_t f; // facet
    uint32_t top; // top facet node
    uint32_t bot; // bot facet node
};

// facet ip0 is coplanar group index, p0 p1 p2 is coplanar group triangle
inline uint32_t order_facets(std::vector<uint32_t>& facets_indexes, std::vector<std::shared_ptr<genericPoint>>& vertices, std::vector<Segment>& segments, std::vector<Facet>& facets,
                             std::vector<FacetOrder>& order_tree,
                             std::vector<std::pair<double3,double>>& facets_spheres, std::unordered_map<uint32_t, std::pair<double3,double>>& planes_equations)
{
    if(facets_indexes.empty())
    {
        return UNDEFINED_VALUE;
    }
    uint32_t start_vn = vertices.size();
    uint32_t start_sn = segments.size();
    uint32_t start_fn = facets.size();
    std::unordered_map<uint32_t, int> vertices_cache;
    
    uint32_t res = order_tree.size();
    order_tree.push_back(FacetOrder());
    
    std::unordered_map<uint32_t, std::pair<double, uint32_t>> coplanar_groups_areas;
    for(uint32_t i : facets_indexes)
    {
        uint32_t cg = facets[i].ip0;
        if(coplanar_groups_areas.end() == coplanar_groups_areas.find(cg))
        {
            coplanar_groups_areas[cg] = std::make_pair(0.0,i);
        }
        coplanar_groups_areas[cg].first += facets_spheres[i].second;
    }
    
    uint32_t c = UNDEFINED_VALUE;
    uint32_t cg = UNDEFINED_VALUE;
    double area = -1.0;
    for(auto [k,v] : coplanar_groups_areas)
    {
        if(v.first > area)
        {
            area = v.first;
            c = v.second;
            cg = k;
        }
    }
    uint32_t c0 = facets[c].p0;
    uint32_t c1 = facets[c].p1;
    uint32_t c2 = facets[c].p2;

    std::vector<uint32_t> top;
    std::vector<uint32_t> bot;
    std::vector<uint32_t> both;
    for(uint32_t i : facets_indexes)
    {
        uint32_t ncg = facets[i].ip0;
        if(ncg==cg)
        {
            continue;
        }
        
        auto [n,d] = planes_equations[cg];
        auto [c,r] = facets_spheres[i];
        double dis = n.dot(c)+d;
        if(dis > r)
        {
            top.push_back(i);
        }
        else if(dis < -r)
        {
            bot.push_back(i);
        }
        else
        {
            bool has_top = false;
            bool has_bot = false;
            std::vector<uint32_t> vs = facets[i].get_vertices(segments);
            for(uint32_t v : vs)
            {
                if(vertices_cache.end() == vertices_cache.find(v))
                {
                    vertices_cache[v] = orient3d(c0,c1,c2,v,vertices.data());
                }
                
                has_top |= 1==vertices_cache[v];
                has_bot |= -1==vertices_cache[v];
                if(has_top && has_bot)
                {
                    break;
                }
            }
            if(has_top && has_bot)
            {
                both.push_back(i);
            }
            else if(has_top && !has_bot)
            {
                top.push_back(i);
            }
            else if(!has_top && has_bot)
            {
                bot.push_back(i);
            }
        }
    }
    
    order_tree[res].f = cg;
    std::unordered_map<uint32_t,std::tuple<uint32_t,uint32_t,uint32_t>> split_segments; // (intersection vertex, top edge, bottom edge)
    for(uint32_t nc : both)
    {
        uint32_t nc0 = facets[nc].p0;
        uint32_t nc1 = facets[nc].p1;
        uint32_t nc2 = facets[nc].p2;
        
        uint32_t top_facet = facets.size();
        facets.push_back(Facet(facets[nc]));
        facets[top_facet].segments.clear();
        uint32_t bot_facet = facets.size();
        facets.push_back(Facet(facets[top_facet]));
        
        uint32_t i0(UNDEFINED_VALUE), i1(UNDEFINED_VALUE);
        for(uint32_t s : facets[nc].segments)
        {
            if(split_segments.end() == split_segments.find(s))
            {
                auto [i_p,top_e,bot_e,vs] = Segment::slice_segment_with_plane(s, c0, c1, c2, vertices, segments, vertices_cache, false);
                split_segments[s] = std::make_tuple(i_p,top_e,bot_e);
            }
            auto [i_p,top_e,bot_e] = split_segments[s];
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
            if(UNDEFINED_VALUE != top_e)
            {
                facets[top_facet].segments.push_back(top_e);
            }
            if(UNDEFINED_VALUE != bot_e)
            {
                facets[bot_facet].segments.push_back(bot_e);
            }
        }
//        if(UNDEFINED_VALUE == i1)
//        {
//            throw "wtf";
//        }
        
        uint32_t i_e = segments.size();
        segments.push_back(Segment(i0,i1,nc0,nc1,nc2,c0,c1,c2));
        facets[top_facet].segments.push_back(i_e);
        facets[bot_facet].segments.push_back(i_e);
        top.push_back(top_facet);
        bot.push_back(bot_facet);
        facets_spheres.push_back(facets[top_facet].get_bounding_sphere(vertices, segments));
        facets_spheres.push_back(facets[bot_facet].get_bounding_sphere(vertices, segments));
    }
    
//    for(uint32_t f : best_top)
//    {
//        facets[f].get_sorted_vertices(segments);
//    }
//    for(uint32_t f : best_bot)
//    {
//        facets[f].get_sorted_vertices(segments);
//    }
    
    order_tree[res].top = order_facets(top, vertices, segments, facets, order_tree, facets_spheres, planes_equations);
    order_tree[res].bot = order_facets(bot, vertices, segments, facets, order_tree, facets_spheres, planes_equations);
    
    while(vertices.size() > start_vn)
    {
        vertices.pop_back();
    }
    while(segments.size() > start_sn)
    {
        segments.pop_back();
    }
    while(facets.size() > start_fn)
    {
        facets.pop_back();
        facets_spheres.pop_back();
    }
    return res;
}

// facet ip0 is coplanar group index, p0 p1 p2 is coplanar group triangle
inline std::vector<FacetOrder> order_facets(std::vector<std::shared_ptr<genericPoint>>& vertices, std::vector<Segment>& segments, std::vector<Facet>& facets)
{
    std::vector<FacetOrder> res;
    std::vector<uint32_t> facets_indexes;
    std::vector<std::pair<double3,double>> facets_spheres;
    std::unordered_map<uint32_t, std::pair<double3,double>> planes_equations;
    for(uint32_t i=0; i<facets.size(); i++)
    {
        facets_indexes.push_back(i);
        facets_spheres.push_back(facets[i].get_bounding_sphere(vertices, segments));
        uint32_t cg = facets[i].ip0;
        if(planes_equations.end() != planes_equations.find(cg))
        {
            continue;
        }
        planes_equations[cg] = facets[i].get_plane_equation(vertices);
    }
    
    order_facets(facets_indexes, vertices, segments, facets, res, facets_spheres, planes_equations);
    return res;
}

#endif
