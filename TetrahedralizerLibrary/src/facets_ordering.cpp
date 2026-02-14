#include "facets_ordering.hpp"
using namespace std;

FacetsOrderingHandle::FacetsOrderingHandle(uint32_t candidate_facets_count,uint32_t examine_facets_count,uint32_t change_scheme_threshold, uint32_t random_seed)
{
    m_candidate_facets_count = candidate_facets_count;
    m_examine_facets_count = examine_facets_count;
    m_change_scheme_threshold = change_scheme_threshold;
    m_gen.seed(random_seed);
    
    if(candidate_facets_count >= examine_facets_count)
    {
        throw "wtf";
    }
}

// facet ip0 is coplanar group index, p0 p1 p2 is coplanar group triangle
// order_tree[i+0] is coplanar group index, order_tree[i+1] is top node, order_tree[i+2] is bot node
std::vector<uint32_t> FacetsOrderingHandle::order_facets(std::vector<std::shared_ptr<genericPoint>>& vertices, std::vector<double3>& approximated_vertices, std::vector<Segment>& segments, std::vector<Facet>& facets, std::vector<uint32_t>& facets_coplanar_groups)
{
    uint32_t vn = vertices.size();
    uint32_t sn = segments.size();
    uint32_t fn = facets.size();
    
    std::vector<uint32_t> res;
    std::vector<uint32_t> facets_indexes;
    std::vector<std::pair<double3,double>> facets_spheres;
    std::unordered_map<uint32_t, std::pair<double3,double>> planes_equations;
    for(uint32_t i=0; i<facets.size(); i++)
    {
        facets_indexes.push_back(i);
        facets_spheres.push_back(facets[i].get_bounding_sphere(approximated_vertices, segments, false));
        uint32_t cg = facets_coplanar_groups[i];
        if(planes_equations.end() != planes_equations.find(cg))
        {
            continue;
        }
        planes_equations[cg] = facets[i].get_plane_equation(approximated_vertices);
    }
    
    shuffle(facets_indexes.begin(),facets_indexes.end(),m_gen);
    order_facets(facets_indexes, vertices, approximated_vertices, segments, facets, facets_coplanar_groups, res, facets_spheres, planes_equations);
    
    vertices.resize(vn);
    approximated_vertices.resize(vn);
    segments.resize(sn);
    facets.resize(fn);
    facets_coplanar_groups.resize(fn);
    return res;
}

// facet ip0 is coplanar group index, p0 p1 p2 is coplanar group triangle
// order_tree[i+0] is coplanar group index, order_tree[i+1] is top node, order_tree[i+2] is bot node
void FacetsOrderingHandle::order_facets(std::vector<uint32_t>& all_facets_indexes, std::vector<std::shared_ptr<genericPoint>>& vertices, std::vector<double3>& approximated_vertices, std::vector<Segment>& segments, std::vector<Facet>& facets, std::vector<uint32_t>& facets_coplanar_groups,
                             std::vector<uint32_t>& order_tree,
                             std::vector<std::pair<double3,double>>& facets_spheres, std::unordered_map<uint32_t, std::pair<double3,double>>& planes_equations)
{
    std::stack<std::tuple<uint32_t,uint32_t, std::vector<uint32_t>>> call_stack; // parent index, depth, current facets
    call_stack.push(std::make_tuple(UNDEFINED_VALUE, 1, std::move(all_facets_indexes)));
    
    std::vector<uint32_t> depth_inserted_count = std::vector<uint32_t>(3, 0); // depth_inserted_count[i+0] inserted vertices, depth_inserted_count[i+1] inserted segments, depth_inserted_count[i+2] inserted facets
    uint32_t last_depth = 0;
    
    while(!call_stack.empty())
    {
        uint32_t parent = std::get<0>(call_stack.top());
        uint32_t depth = std::get<1>(call_stack.top());
        std::vector<uint32_t> cur_facets = std::move(std::get<2>(call_stack.top()));
        call_stack.pop();
        
        if(0==cur_facets.size())
        {
            if(UNDEFINED_VALUE != parent)
            {
                order_tree[parent] = UNDEFINED_VALUE;
            }
            continue;
        }
        
        if(depth >= depth_inserted_count.size()/3)
        {
            depth_inserted_count.push_back(0);
            depth_inserted_count.push_back(0);
            depth_inserted_count.push_back(0);
        }
        for(uint32_t i=depth; i<=last_depth; i++)
        {
            uint32_t vn = depth_inserted_count[3*i+0];
            uint32_t sn = depth_inserted_count[3*i+1];
            uint32_t fn = depth_inserted_count[3*i+2];
            depth_inserted_count[3*i+0] = 0;
            depth_inserted_count[3*i+1] = 0;
            depth_inserted_count[3*i+2] = 0;
            
            vertices.resize(vertices.size() - vn);
            approximated_vertices.resize(approximated_vertices.size() - vn);
            segments.resize(segments.size() - sn);
            facets.resize(facets.size() - fn);
            facets_coplanar_groups.resize(facets_coplanar_groups.size() - fn);
            facets_spheres.resize(facets_spheres.size() - fn);
        }
        last_depth = depth;
        
        uint32_t cur_order_tree_node = order_tree.size();;
        order_tree.push_back(UNDEFINED_VALUE);
        order_tree.push_back(UNDEFINED_VALUE);
        order_tree.push_back(UNDEFINED_VALUE);
        if(UNDEFINED_VALUE != parent)
        {
            order_tree[parent] = cur_order_tree_node;
        }
        
//        m_vertices_cache.clear();
        bool use_bisecting_sheme = cur_facets.size() >= m_change_scheme_threshold;
        uint32_t best_c = UNDEFINED_VALUE;
        std::vector<uint32_t> best_top;
        std::vector<uint32_t> best_bot;
        std::vector<uint32_t> best_both;
        
        uint32_t i_n = min(m_candidate_facets_count, (uint32_t)cur_facets.size());
        uint32_t j_n = min(m_examine_facets_count, (uint32_t)cur_facets.size());
        for(uint32_t i=0; i<i_n; i++)
        {
            uint32_t c = cur_facets[i];
//            uint32_t cg = facets_coplanar_groups[c];
//            if(m_vertices_cache.end() != m_vertices_cache.find(cg)) // checked a coplanar facet
//            {
//                continue;
//            }
//            m_vertices_cache[cg] = std::unordered_map<uint32_t, int>();
            
            std::vector<uint32_t> top;
            std::vector<uint32_t> bot;
            std::vector<uint32_t> both;
            for(uint32_t j=0; j<j_n; j++)
            {
                if(i == j)
                {
                    continue;
                }
                uint32_t nc = cur_facets[j];
                int int_type = check_plane_facet_intersection(c, nc, vertices, segments, facets, facets_coplanar_groups, facets_spheres, planes_equations);
                if(0 == int_type)
                {
                    both.push_back(nc);
                }
                else if(1 == int_type)
                {
                    top.push_back(nc);
                }
                else if(-1 == int_type)
                {
                    bot.push_back(nc);
                }
                
                if(UNDEFINED_VALUE!=best_c)
                {
                    if(!use_bisecting_sheme)
                    {
                        if(both.size()>=best_both.size())
                        {
                            break;
                        }
                    }
                }
            }
            if(UNDEFINED_VALUE==best_c)
            {
                best_c = c;
                best_top = std::move(top);
                best_bot = std::move(bot);
                best_both = std::move(both);
                if(!use_bisecting_sheme)
                {
                    if(0==best_both.size())
                    {
                        break;
                    }
                }
            }
            else
            {
                if(!use_bisecting_sheme)
                {
                    if(both.size()<best_both.size())
                    {
                        best_c = c;
                        best_top = std::move(top);
                        best_bot = std::move(bot);
                        best_both = std::move(both);
                        if(0==best_both.size())
                        {
                            break;
                        }
                    }
                }
                else
                {
                    if(abs((int32_t)top.size()-(int32_t)bot.size()) < abs((int32_t)best_top.size()-(int32_t)best_bot.size()))
                    {
                        best_c = c;
                        best_top = std::move(top);
                        best_bot = std::move(bot);
                        best_both = std::move(both);
                        if(0==abs((int32_t)best_top.size()-(int32_t)best_bot.size()))
                        {
                            break;
                        }
                    }
                }
            }
        }
        
        uint32_t c = best_c;
        for(uint32_t j=j_n; j<cur_facets.size(); j++)
        {
            uint32_t nc = cur_facets[j];
            int int_type = check_plane_facet_intersection(c, nc, vertices, segments, facets, facets_coplanar_groups, facets_spheres, planes_equations);
            if(0 == int_type)
            {
                best_both.push_back(nc);
            }
            else if(1 == int_type)
            {
                best_top.push_back(nc);
            }
            else if(-1 == int_type)
            {
                best_bot.push_back(nc);
            }
        }
        
        uint32_t cg = facets_coplanar_groups[c];
        uint32_t c0 = facets[c].p0;
        uint32_t c1 = facets[c].p1;
        uint32_t c2 = facets[c].p2;
        order_tree[cur_order_tree_node] = cg;
        
        std::unordered_map<uint32_t,std::tuple<uint32_t,uint32_t,uint32_t>> split_segments; // (intersection vertex, top edge, bottom edge)
        for(uint32_t nc : best_both)
        {
            uint32_t nc0 = facets[nc].p0;
            uint32_t nc1 = facets[nc].p1;
            uint32_t nc2 = facets[nc].p2;
            
            uint32_t top_facet = facets.size();
            facets.push_back(Facet(nc0, nc1, nc2));
            uint32_t bot_facet = facets.size();
            facets.push_back(Facet(nc0, nc1, nc2));
            facets_coplanar_groups.push_back(facets_coplanar_groups[nc]);
            facets_coplanar_groups.push_back(facets_coplanar_groups[nc]);
            
            uint32_t i0(UNDEFINED_VALUE), i1(UNDEFINED_VALUE);
            for(uint32_t s : facets[nc].segments)
            {
                if(split_segments.end() == split_segments.find(s))
                {
                    int o0 = orient3d(c0,c1,c2,segments[s].e0,vertices.data());
                    int o1 = orient3d(c0,c1,c2,segments[s].e1,vertices.data());
                    auto [i_p,top_e,bot_e,vs] = Segment::slice_segment_with_plane(s, c0, c1, c2, vertices, segments, o0,o1, false);
                    split_segments[s] = std::make_tuple(i_p,top_e,bot_e);
                    if(UNDEFINED_VALUE != vs.p0)
                    {
                        depth_inserted_count[3*depth+0]++;
                        depth_inserted_count[3*depth+1] += 2;
                    }
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
            approximate_verteices(approximated_vertices, vertices);
            facets_spheres.push_back(facets[top_facet].get_bounding_sphere(approximated_vertices, segments, false));
            facets_spheres.push_back(facets[bot_facet].get_bounding_sphere(approximated_vertices, segments, false));
            
            depth_inserted_count[3*depth+1]++;
            depth_inserted_count[3*depth+2] += 2;
        }
        
        //    for(uint32_t f : best_top)
        //    {
        //        facets[f].get_sorted_vertices(segments);
        //    }
        //    for(uint32_t f : best_bot)
        //    {
        //        facets[f].get_sorted_vertices(segments);
        //    }
        
        if(0 != best_top.size())
        {
            call_stack.push(std::make_tuple(cur_order_tree_node+1, depth+1, std::move(best_top)));
        }
        if(0 != best_bot.size())
        {
            call_stack.push(std::make_tuple(cur_order_tree_node+2, depth+1, std::move(best_bot)));
        }
    }
}

// c is the plane, nc is the facet
// 0 if intersect, 1 if facet above plane, -1 if facet below plane, 69 if coplanar
int FacetsOrderingHandle::check_plane_facet_intersection(uint32_t c, uint32_t nc, std::vector<std::shared_ptr<genericPoint>>& vertices, std::vector<Segment>& segments, std::vector<Facet>& facets, std::vector<uint32_t>& facets_coplanar_groups, std::vector<std::pair<double3,double>>& facets_spheres, std::unordered_map<uint32_t, std::pair<double3,double>>& planes_equations)
{
    uint32_t cg = facets_coplanar_groups[c];
    uint32_t ncg = facets_coplanar_groups[nc];
    if(cg == ncg)
    {
        return 69;
    }

    auto [p_n,p_d] = planes_equations[cg];
    auto [s_c,s_r] = facets_spheres[nc];
    double dis = p_n.dot(s_c)+p_d;
    if(dis > s_r)
    {
        return 1;
    }
    else if(dis < -s_r)
    {
        return -1;
    }
    
    uint32_t c0 = facets[c].p0;
    uint32_t c1 = facets[c].p1;
    uint32_t c2 = facets[c].p2;
    bool has_top = false;
    bool has_bot = false;
    facets[nc].get_vertices(segments, Facet::m_get_vertices_res);
    for(uint32_t v : Facet::m_get_vertices_res)
    {
        int o = orient3d(c0,c1,c2,v,vertices.data());;
//        auto it = m_vertices_cache[cg].find(v);
//        if(it == m_vertices_cache[cg].end())
//        {
//            o = orient3d(c0,c1,c2,v,vertices.data());
//            m_vertices_cache[cg][v] = o;
//        }
//        else
//        {
//            o = it->second;
//        }
        
        has_top |= 1==o;
        has_bot |= -1==o;
        if(has_top && has_bot)
        {
            break;
        }
    }
    if(has_top && has_bot)
    {
        return 0;
    }
    else if(has_top && !has_bot)
    {
        return 1;
    }
    else if(!has_top && has_bot)
    {
        return -1;
    }
    return 69;
}


void FacetsOrderingHandle::shuffle_indexes(std::vector<uint32_t>& indexes, uint32_t k)
{
    uint32_t n = indexes.size();
    if(k >= n)
    {
        return;
    }
    
    for(uint32_t i=0; i<k; i++)
    {
        m_dist.param(std::uniform_int_distribution<uint32_t>::param_type(i, n - 1));
        uint32_t j = m_dist(m_gen);
        std::swap(indexes[i],indexes[j]);
    }
}
