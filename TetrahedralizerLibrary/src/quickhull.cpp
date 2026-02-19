#include "quickhull.hpp"
using namespace std;

std::vector<uint32_t> Quickhull::calculate(std::vector<std::shared_ptr<genericPoint>>& vertices, std::vector<double3>& approximated_vertices)
{
    auto approximate_normal = [&](uint32_t p0, uint32_t p1, uint32_t p2) -> double3
    {
        return (approximated_vertices[p1]-approximated_vertices[p0]).cross(approximated_vertices[p2]-approximated_vertices[p0]);
    };
    auto approximate_signed_volume = [&](double3 norm, uint32_t p_p, uint32_t p) -> double
    {
        return norm.dot(approximated_vertices[p]-approximated_vertices[p_p]);
    };
    
    vector<uint32_t> triangles;
    vector<uint32_t> neighbors;
    vector<double3> triangles_normals;
    vector<vector<uint32_t>> triangles_outside_vertices;
    queue<uint32_t> triangles_gaps;
    
    auto add_triangle = [&](uint32_t p0, uint32_t p1, uint32_t p2) -> uint32_t
    {
        uint32_t res;
        if(triangles_gaps.empty())
        {
            res = triangles.size()/3;
            triangles.push_back(p0);
            triangles.push_back(p1);
            triangles.push_back(p2);
            neighbors.push_back(UNDEFINED_VALUE);
            neighbors.push_back(UNDEFINED_VALUE);
            neighbors.push_back(UNDEFINED_VALUE);
            triangles_normals.push_back(approximate_normal(p0, p1, p2));
            triangles_outside_vertices.push_back(vector<uint32_t>());
        }
        else
        {
            res = triangles_gaps.front();
            triangles_gaps.pop();
            triangles[3*res+0] = p0;
            triangles[3*res+1] = p1;
            triangles[3*res+2] = p2;
            neighbors[3*res+0] = UNDEFINED_VALUE;
            neighbors[3*res+1] = UNDEFINED_VALUE;
            neighbors[3*res+2] = UNDEFINED_VALUE;
            triangles_normals[res] = approximate_normal(p0, p1, p2);
            triangles_outside_vertices[res].clear();
        }
        return res;
    };
    auto remove_triangle = [&](uint32_t t)
    {
        triangles_gaps.push(t);
//        triangles_normals[t] = double3();
        triangles_outside_vertices[t].clear();
        for(uint32_t i=0; i<3; i++)
        {
            triangles[3*t+i] = UNDEFINED_VALUE;
            uint32_t n = neighbors[3*t+i];
            neighbors[3*t+i] = UNDEFINED_VALUE;
            if(UNDEFINED_VALUE==n)
            {
                continue;
            }
            neighbors[n] = UNDEFINED_VALUE;
        }
    };
    
    {
        uint32_t t0 = 0;
        uint32_t t1 = 0;
        double min_x = approximated_vertices[0].x;
        double max_x = approximated_vertices[0].x;
        for(uint32_t i=1; i<approximated_vertices.size(); i++)
        {
            if(min_x > approximated_vertices[i].x)
            {
                t0 = i;
                min_x = approximated_vertices[i].x;
            }
            if(max_x < approximated_vertices[i].x)
            {
                t1 = i;
                max_x = approximated_vertices[i].x;
            }
        }
        
        uint32_t t2 = UNDEFINED_VALUE;
        double3 norm;
        double dis = -1.0;
        for(uint32_t i=0; i<approximated_vertices.size(); i++)
        {
            if(t0 == i || t1 == i)
            {
                continue;
            }
            
            double3 cur_norm = approximate_normal(t0, t1, i);
            double cur_dis = cur_norm.length_squared();
            if(UNDEFINED_VALUE==t2 || cur_dis>dis)
            {
                t2 = i;
                norm = cur_norm;
                dis = cur_dis;
            }
        }
        
        uint32_t t3 = UNDEFINED_VALUE;
        dis = -1.0;
        for(uint32_t i=0; i<approximated_vertices.size(); i++)
        {
            if(t0==i || t1==i || t2==i)
            {
                continue;
            }
            
            double cur_dis = abs(approximate_signed_volume(norm, t0, i));
            if(UNDEFINED_VALUE==t3 || cur_dis>dis)
            {
                t3 = i;
                dis = cur_dis;
            }
        }
        
        if(UNDEFINED_VALUE==t2 || UNDEFINED_VALUE==t3)
        {
            throw "wtf";
        }
        int orient = orient3d(t0,t1,t2,t3,vertices.data());
        if(0 == orient)
        {
            throw "wtf";
        }
        if(-1 == orient)
        {
            swap(t2,t3);
        }
        
        triangles.push_back(t0);
        triangles.push_back(t2);
        triangles.push_back(t1);
        
        triangles.push_back(t0);
        triangles.push_back(t1);
        triangles.push_back(t3);
        
        triangles.push_back(t1);
        triangles.push_back(t2);
        triangles.push_back(t3);
        
        triangles.push_back(t0);
        triangles.push_back(t3);
        triangles.push_back(t2);
        
        neighbors.push_back(9+2);
        neighbors.push_back(6+0);
        neighbors.push_back(3+0);
        neighbors.push_back(0+2);
        neighbors.push_back(6+2);
        neighbors.push_back(9+0);
        neighbors.push_back(0+1);
        neighbors.push_back(9+1);
        neighbors.push_back(3+1);
        neighbors.push_back(3+2);
        neighbors.push_back(6+1);
        neighbors.push_back(0+0);
        
        for(uint32_t i=0; i<4; i++)
        {
            uint32_t c0 = triangles[3*i+0];
            uint32_t c1 = triangles[3*i+1];
            uint32_t c2 = triangles[3*i+2];
            triangles_normals.push_back(approximate_normal(c0,c1,c2));
            triangles_outside_vertices.push_back(vector<uint32_t>());
            dis = 0.0;
            for(uint32_t j=0; j<vertices.size(); j++)
            {
                if(1 == orient3d(c0,c1,c2,j,vertices.data()))
                {
                    triangles_outside_vertices[i].push_back(j);
                    double cur_dis = approximate_signed_volume(triangles_normals[i], c0, j);
                    if(cur_dis>dis)
                    {
                        dis = cur_dis;
                        uint32_t n = triangles_outside_vertices[i].size();
                        if(n>1)
                        {
                            swap(triangles_outside_vertices[i][0],triangles_outside_vertices[i][n-1]);
                        }
                    }
                }
            }
        }
    }
    
    queue<uint32_t> search_triangles;
    vector<uint32_t> remove_triangles;
    unordered_set<uint32_t> bounding_segments;
    vector<uint32_t> outside_vertices;
    vector<uint32_t> vertices_cache = vector<uint32_t>(vertices.size(), UNDEFINED_VALUE);
    vector<uint32_t> triangles_cache;
    vector<pair<uint32_t,uint32_t>> neighbors_cache = vector<pair<uint32_t,uint32_t>>(vertices.size(), make_pair(UNDEFINED_VALUE, 0));
    uint32_t cache_index = 0;
    bool has_active_vertex = true;
    while(has_active_vertex)
    {
        has_active_vertex = false;
        for(uint32_t i=0; i<triangles.size()/3; i++)
        {
            if(UNDEFINED_VALUE==triangles[3*i+0] || 0==triangles_outside_vertices[i].size())
            {
                continue;
            }
            
            has_active_vertex = true;
            remove_triangles.clear();
            bounding_segments.clear();
            outside_vertices.clear();
            triangles_cache.resize(triangles.size()/3, UNDEFINED_VALUE);
            
            uint32_t p = triangles_outside_vertices[i][0];
            search_triangles.push(i);
            while(!search_triangles.empty())
            {
                uint32_t t = search_triangles.front();
                search_triangles.pop();
                if(cache_index == triangles_cache[t])
                {
                    continue;
                }
                triangles_cache[t] = cache_index;
                
                uint32_t t0 = triangles[3*t+0];
                uint32_t t1 = triangles[3*t+1];
                uint32_t t2 = triangles[3*t+2];
                if(1 != orient3d(t0,t1,t2,p,vertices.data()))
                {
                    continue;
                }
                
                remove_triangles.push_back(t);
                for(uint32_t j=0; j<3; j++)
                {
                    search_triangles.push(neighbors[3*t+j]/3);
                    if(bounding_segments.end() != bounding_segments.find(3*t+j))
                    {
                        bounding_segments.erase(3*t+j);
                    }
                    else
                    {
                        bounding_segments.insert(neighbors[3*t+j]);
                    }
                }
            }
            
            for(uint32_t t : remove_triangles)
            {
                for(uint32_t v : triangles_outside_vertices[t])
                {
                    if(cache_index == vertices_cache[v])
                    {
                        continue;
                    }
                    vertices_cache[v] = cache_index;
                    outside_vertices.push_back(v);
                }
                
                remove_triangle(t);
            }
            
            for(uint32_t s : bounding_segments)
            {
                uint32_t t = s/3;
                uint32_t t_s = s%3;
                uint32_t s0 = triangles[3*t+(t_s+1)%3];
                uint32_t s1 = triangles[3*t+t_s];
                
//                if(s0 == UNDEFINED_VALUE || s1==UNDEFINED_VALUE)
//                {
//                    throw "wtf";
//                }
                
                uint32_t nt = add_triangle(p, s0, s1);
                neighbors[s] = 3*nt+1;
                neighbors[3*nt+1] = s;
                
                auto connect_neighbor = [&](uint32_t p0, uint32_t cur)
                {
                    if(cache_index != neighbors_cache[p0].first)
                    {
                        neighbors_cache[p0] = make_pair(cache_index, cur);
                    }
                    else
                    {
                        uint32_t n = neighbors_cache[p0].second;
                        neighbors[cur] = n;
                        neighbors[n] = cur;
                    }
                };
                connect_neighbor(s0, 3*nt+0);
                connect_neighbor(s1, 3*nt+2);
                
                double dis = numeric_limits<double>::min();
                for(uint32_t v : outside_vertices)
                {
                    if(1 == orient3d(p,s0,s1,v,vertices.data()))
                    {
                        triangles_outside_vertices[nt].push_back(v);
                        
                        double cur_dis = approximate_signed_volume(triangles_normals[nt], p, v);
                        if(cur_dis>dis)
                        {
                            dis = cur_dis;
                            uint32_t n = triangles_outside_vertices[i].size();
                            if(n>1)
                            {
                                swap(triangles_outside_vertices[i][0],triangles_outside_vertices[i][n-1]);
                            }
                        }
                    }
                }
            }
            cache_index++;
        }
    }
    
    for(uint32_t i=0; i<triangles.size(); i+=3)
    {
        if(UNDEFINED_VALUE != triangles[i])
        {
            continue;
        }
        
        uint32_t j = triangles.size() - 3;
        swap(triangles[i+0], triangles[j+0]);
        swap(triangles[i+1], triangles[j+1]);
        swap(triangles[i+2], triangles[j+2]);
        triangles.pop_back();
        triangles.pop_back();
        triangles.pop_back();
        
        i-=3;
    }
    return triangles;
}
