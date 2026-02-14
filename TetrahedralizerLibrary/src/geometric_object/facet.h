#ifndef facet_h
#define facet_h

#include "../common_header.h"
#include "segment.h"

class Facet
{
    public:
    std::vector<uint32_t> segments; // segments forming the facet
    uint32_t p0,p1,p2; // three explicit vetices that define the facet

    Facet()
    {
        p0=p1=p2=UNDEFINED_VALUE;
    }
    Facet(uint32_t p0, uint32_t p1, uint32_t p2)
    {
        this->p0 = p0;
        this->p1 = p1;
        this->p2 = p2;
    }

    Facet(uint32_t s0, uint32_t s1, uint32_t s2, uint32_t p0, uint32_t p1, uint32_t p2)
    {
        this->segments.push_back(s0);
        this->segments.push_back(s1);
        this->segments.push_back(s2);
        this->p0 = p0;
        this->p1 = p1;
        this->p2 = p2;
    }
    Facet(const Facet& other)
    {
        this->segments = other.segments;
        this->p0 = other.p0;
        this->p1 = other.p1;
        this->p2 = other.p2;
    }
    
    uint32_t write_to_byte_buffer_size()
    {
        return write_vector_to_byte_buffer_size(segments) + 3*4;
    }
    void write_to_byte_buffer(uint8_t* buffer)
    {
        uint32_t sn = write_vector_to_byte_buffer_size(segments);
        write_vector_to_byte_buffer(segments, buffer);
        memcpy(buffer+sn+0*4, &p0, 4);
        memcpy(buffer+sn+1*4, &p1, 4);
        memcpy(buffer+sn+2*4, &p2, 4);
    }
    
    void increase_segments_indexes(uint32_t n)
    {
        for(uint32_t i=0; i<segments.size(); i++)
        {
            segments[i] += n;
        }
    }
    
    bool contains_segment(uint32_t segment)
    {
        return segments.end() != std::find(segments.begin(), segments.end(), segment);
    }
    
    bool intersects_segment(uint32_t s0, uint32_t s1, int normal, std::vector<std::shared_ptr<genericPoint>>& all_vertices, std::vector<Segment>& all_segments)
    {
        for(uint32_t s : segments)
        {
            uint32_t p0 = all_segments[s].e0;
            uint32_t p1 = all_segments[s].e1;
            if(segment_cross_segment(s0, s1, p0, p1, normal, all_vertices.data()))
            {
                return true;
            }
        }
        
        std::vector<uint32_t> vs = get_sorted_vertices(all_segments);
        bool b0 = true;
        bool b1 = true;
        int o0 = 0;
        int o1 = 0;
        for(uint32_t i=0; i<vs.size(); i++)
        {
            uint32_t p0 = vs[i];
            uint32_t p1 = vs[(i+1)%vs.size()];
            
            if(b0)
            {
                int o = orient3d_ignore_axis(p0,p1,s0,normal,all_vertices.data());
                if(0 == o0)
                {
                    o0 = o;
                }
                if(0 != o && o0 != o)
                {
                    b0 = false;
                }
            }
            if(b1)
            {
                int o = orient3d_ignore_axis(p0,p1,s1,normal,all_vertices.data());
                if(0 == o1)
                {
                    o1 = o;
                }
                if(0 != o && o1 != o)
                {
                    b1 = false;
                }
            }
        }
        
        return b0 || b1;
    }
    
    inline static std::vector<uint8_t> m_get_vertices_cache;
    inline static std::vector<uint32_t> m_get_vertices_res;
    std::vector<uint32_t> get_sorted_vertices(std::vector<Segment>& all_segments)
    {
        std::vector<Segment> res;
        for(uint32_t s : segments)
        {
            res.push_back(all_segments[s]);
        }
        Segment::sort_segments(res);
        return Segment::get_segments_vertices(res);
    }
    void get_vertices(std::vector<Segment>& all_segments, std::vector<uint32_t>& res)
    {
        res.clear();
        
        for(uint32_t s : segments)
        {
            uint32_t v0 = all_segments[s].e0;
            uint32_t v1 = all_segments[s].e1;
            if(m_get_vertices_cache.size()<=v0)
            {
                m_get_vertices_cache.resize(v0+1,0);
            }
            if(m_get_vertices_cache.size()<=v1)
            {
                m_get_vertices_cache.resize(v1+1,0);
            }
            
            if(0 == m_get_vertices_cache[v0])
            {
                res.push_back(v0);
                m_get_vertices_cache[v0] = 1;
            }
            if(0 == m_get_vertices_cache[v1])
            {
                res.push_back(v1);
                m_get_vertices_cache[v1] = 1;
            }
        }
        
        for(uint32_t v : res)
        {
            m_get_vertices_cache[v] = 0;
        }
    }
    std::pair<double3,double> get_plane_equation(std::vector<double3>& approximated_vertices)
    {
        double3 t0 = approximated_vertices[p0];
        double3 t1 = approximated_vertices[p1];
        double3 t2 = approximated_vertices[p2];
        double3 n = ((t1-t0).cross(t2-t0)).normalized();
        double d = -n.dot(t0);
        return std::make_pair(n,d);
    }
    std::pair<double3, double> get_bounding_sphere(std::vector<double3>& approximated_vertices, std::vector<Segment>& segments, bool square_radius=true, double padding=0.05)
    {
        get_vertices(segments, m_get_vertices_res);
        double3 centroid(0.0,0.0,0.0);
        for(uint32_t i : m_get_vertices_res)
        {
            centroid += approximated_vertices[i];
        }
        centroid /= (double)m_get_vertices_res.size();
        
        double radius = -1.0;
        for(uint32_t i : m_get_vertices_res)
        {
            radius = std::max(radius, (centroid-approximated_vertices[i]).length_squared());
        }
        if(!square_radius)
        {
            radius = std::sqrt(radius);
        }
        return std::make_pair(centroid, padding*radius+radius);
    }
    
    bool intersects_coplanar_triangle(uint32_t c0,uint32_t c1,uint32_t c2, int ignore_axis, std::vector<std::shared_ptr<genericPoint>>& all_vertices, std::vector<Segment>& all_segments)
    {
        if(0 == orient3d_ignore_axis(c0, c1, c2, ignore_axis, all_vertices.data()))
        {
            return false; // should never happen
        }
        uint32_t vn = all_vertices.size();
        uint32_t sn = all_segments.size();
        
        uint32_t c3 = UNDEFINED_VALUE;
        for(uint32_t i=0; i<all_vertices.size(); i++)
        {
            if(!all_vertices[i]->isExplicit3D())
            {
                continue;
            }
            int o = orient3d(c0,c1,c2,i,all_vertices.data());
            if(0 != o)
            {
                c3 = i;
                if(-1 == o)
                {
                    std::swap(c1,c2);
                }
                break;
            }
        }
        if(UNDEFINED_VALUE == c3)
        {
            throw "wtf";
        }
        
        std::vector<uint32_t> active_segments = segments;
        std::vector<uint32_t> active_segments_temp;
        
        auto slice_with_plane = [&](uint32_t p0, uint32_t p1, uint32_t p2)
        {
            bool has_edge_on_constraint = false;
            uint32_t i0(UNDEFINED_VALUE), i1(UNDEFINED_VALUE);
            for(uint32_t s : active_segments)
            {
                int o0 = orient3d(p0, p1, p2, all_segments[s].e0, all_vertices.data());
                int o1 = orient3d(p0, p1, p2, all_segments[s].e1, all_vertices.data());
                auto [i_p,top_s,bot_s,vs] = Segment::slice_segment_with_plane(s, p0, p1, p2, all_vertices, all_segments, o0, o1, false);
                
                if(UNDEFINED_VALUE == i_p && top_s == UNDEFINED_VALUE && bot_s == UNDEFINED_VALUE)
                {
                    has_edge_on_constraint = true;
                    active_segments_temp.push_back(s);
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
                    active_segments_temp.push_back(top_s);
                }
            }
            
            if(!has_edge_on_constraint && i1 != UNDEFINED_VALUE)
            {
                uint32_t i_e = all_segments.size();
                all_segments.push_back(Segment(i0,i1,this->p0,this->p1,this->p2,p0,p1,p2));
                active_segments_temp.push_back(i_e);
            }
            active_segments = active_segments_temp;
            active_segments_temp.clear();
        };
        
        bool res = true;
        slice_with_plane(c0,c3,c1);
        if(active_segments.size()<=2)
        {
            res = false;
            goto RETURN;
        }
        slice_with_plane(c1,c3,c2);
        if(active_segments.size()<=2)
        {
            res = false;
            goto RETURN;
        }
        slice_with_plane(c2,c3,c0);
        if(active_segments.size()<=2)
        {
            res = false;
            goto RETURN;
        }
        
        RETURN:
        all_vertices.resize(vn);
        all_segments.resize(sn);
        return res;
    }
    
    bool check_validity(std::vector<std::shared_ptr<genericPoint>>& all_vertices, std::vector<Segment>& all_segments)
    {
        std::vector<uint32_t> vs = get_sorted_vertices(all_segments);
        int max_norm = max_component_in_triangle_normal(p0,p1,p2,all_vertices.data());
        int o = 0;
        for(uint32_t j=0; j<vs.size(); j++)
        {
            if(0 != orient3d(p0,p1,p2,vs[j],all_vertices.data()))
            {
                throw "wtf";
            }
            uint32_t p0 = vs[j];
            uint32_t p1 = vs[(j+1)%vs.size()];
            uint32_t p2 = vs[(j+2)%vs.size()];

            int new_o = orient3d_ignore_axis(p0, p1, p2, max_norm, all_vertices.data());
            if(0 == new_o)
            {
                continue;
            }
            if(0 == o)
            {
                o = new_o;
                continue;
            }
            if(new_o != o)
            {
                throw "wtf";
            }
        }
        if(0 == o)
        {
            throw "wtf";
        }
        return true;
    }
};

extern "C"
{
    struct FacetInteropData
    {
        public:
        const uint32_t* segments;
        uint32_t segments_count;
        uint32_t p0, p1, p2;
            
        FacetInteropData& operator=(const Facet& other)
        {
            this->segments = other.segments.data();
            segments_count = other.segments.size();
            this->p0 = other.p0;
            this->p1 = other.p1;
            this->p2 = other.p2;
            return *this;
        }
        
        Facet to_facet()
        {
            Facet res;
            res.segments = std::vector<uint32_t>(segments,segments+segments_count);
            res.p0 = p0;
            res.p1 = p1;
            res.p2 = p2;
            return res;
        }
    };
}

#endif
