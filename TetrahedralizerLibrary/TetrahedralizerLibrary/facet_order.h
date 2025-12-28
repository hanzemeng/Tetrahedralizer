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
    std::unordered_map<uint32_t, std::unordered_map<uint32_t, int>> constraints_facets_cache;
    std::unordered_map<uint32_t, std::unordered_map<uint32_t, int>> constraints_vertices_cache;
    
    uint32_t res = order_tree.size();
    order_tree.push_back(FacetOrder());
    
    uint32_t best_c = UNDEFINED_VALUE;
    std::vector<uint32_t> best_top;
    std::vector<uint32_t> best_bot;
    std::vector<uint32_t> best_both;
    
    std::vector<uint32_t> search_indexes;
    if(facets_indexes.size() < 20)
    {
        search_indexes = facets_indexes;
    }
    else
    {
        search_indexes = vector_random_elements(facets_indexes, 10);
    }
    for(uint32_t i=0; i<search_indexes.size(); i++)
    {
        uint32_t c = search_indexes[i];
        uint32_t cg = facets[c].ip0;
        uint32_t c0 = facets[c].p0;
        uint32_t c1 = facets[c].p1;
        uint32_t c2 = facets[c].p2;
        if(constraints_facets_cache.end() == constraints_facets_cache.find(cg))
        {
            constraints_facets_cache[cg] = std::unordered_map<uint32_t, int>();
            constraints_vertices_cache[cg] = std::unordered_map<uint32_t, int>();
        }
        
        std::vector<uint32_t> top;
        std::vector<uint32_t> bot;
        std::vector<uint32_t> both;
        for(uint32_t j=0; j<facets_indexes.size(); j++)
        {
            uint32_t nc = facets_indexes[j];
            uint32_t ncg = facets[nc].ip0;
            if(c==nc)
            {
                continue;
            }
            
            if(constraints_facets_cache[cg].end() == constraints_facets_cache[cg].find(nc))
            {
                if(cg == ncg)
                {
                    constraints_facets_cache[cg][nc] = 69; // coplanar
                    goto GET_FACET_TYPE;
                }
                
                auto [n,d] = planes_equations[cg];
                auto [c,r] = facets_spheres[nc];
                double dis = n.dot(c)+d;
                if(dis > r)
                {
                    constraints_facets_cache[cg][nc] = 1;
                }
                else if(dis < -r)
                {
                    constraints_facets_cache[cg][nc] = -1;
                }
                else
                {
                    bool has_top = false;
                    bool has_bot = false;
                    std::vector<uint32_t> vs = facets[nc].get_vertices(segments);
                    for(uint32_t v : vs)
                    {
                        if(constraints_vertices_cache[cg].end() == constraints_vertices_cache[cg].find(v))
                        {
                            constraints_vertices_cache[cg][v] = orient3d(c0,c1,c2,v,vertices.data());
                        }
                        
                        has_top |= 1==constraints_vertices_cache[cg][v];
                        has_bot |= -1==constraints_vertices_cache[cg][v];
                        if(has_top && has_bot)
                        {
                            break;
                        }
                    }
                    if(has_top && has_bot)
                    {
                        constraints_facets_cache[cg][nc] = 0;
                    }
                    else if(has_top && !has_bot)
                    {
                        constraints_facets_cache[cg][nc] = 1;
                    }
                    else if(!has_top && has_bot)
                    {
                        constraints_facets_cache[cg][nc] = -1;
                    }
                    else
                    {
                        constraints_facets_cache[cg][nc] = 69; // coplanar, should not happen
                    }
                }
            }
            
            GET_FACET_TYPE:
            if(0 == constraints_facets_cache[cg][nc])
            {
                both.push_back(nc);
            }
            else if(1 == constraints_facets_cache[cg][nc])
            {
                top.push_back(nc);
            }
            else if(-1 == constraints_facets_cache[cg][nc])
            {
                bot.push_back(nc);
            }
            if(UNDEFINED_VALUE!=best_c && both.size()>=best_both.size())
            {
                break;
            }
        }
        if(UNDEFINED_VALUE==best_c || best_both.size() > both.size())
        {
            best_c = c;
            best_top = std::move(top);
            best_bot = std::move(bot);
            best_both = std::move(both);
            if(0 == best_both.size())
            {
                break;
            }
        }
    }
    
    uint32_t c = best_c;
    uint32_t cg = facets[c].ip0;
    uint32_t c0 = facets[c].p0;
    uint32_t c1 = facets[c].p1;
    uint32_t c2 = facets[c].p2;
    order_tree[res].f = cg;
    
    std::unordered_map<uint32_t,std::tuple<uint32_t,uint32_t,uint32_t>> split_segments; // (intersection vertex, top edge, bottom edge)
    for(uint32_t nc : best_both)
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
                auto [i_p,top_e,bot_e,vs] = Segment::slice_segment_with_plane(s, c0, c1, c2, vertices, segments, constraints_vertices_cache[cg], false);
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
        best_top.push_back(top_facet);
        best_bot.push_back(bot_facet);
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
    
    order_tree[res].top = order_facets(best_top, vertices, segments, facets, order_tree, facets_spheres, planes_equations);
    order_tree[res].bot = order_facets(best_bot, vertices, segments, facets, order_tree, facets_spheres, planes_equations);
    
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
