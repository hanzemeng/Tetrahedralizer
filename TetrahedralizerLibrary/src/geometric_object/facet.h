#ifndef facet_h
#define facet_h

#include "../common_header.h"
#include "segment.h"

class Facet
{
    public:
    std::vector<uint32_t> segments; // segments forming the facet
    uint32_t p0,p1,p2; // three explicit vetices that define the facet
    double w0,w1; // w0+w1+w2==1 and w0*p0+w1*p1+w2*p2 is the facet centroid
    uint32_t ip0, ip1; // two incident polyhedrons

    Facet()
    {
        p0=p1=p2=ip0=ip1=UNDEFINED_VALUE;
        w0=w1=1.0/3.0;
    }
    Facet(uint32_t s0, uint32_t s1, uint32_t s2, uint32_t p0, uint32_t p1, uint32_t p2, uint32_t cg)
    {
        this->segments.push_back(s0);
        this->segments.push_back(s1);
        this->segments.push_back(s2);
        this->p0 = p0;
        this->p1 = p1;
        this->p2 = p2;
        this->ip0 = cg;
    }
    Facet(uint32_t s0, uint32_t s1, uint32_t s2, uint32_t p0, uint32_t p1, uint32_t p2, uint32_t ip0, uint32_t ip1)
    {
        this->segments.push_back(s0);
        this->segments.push_back(s1);
        this->segments.push_back(s2);
        this->p0 = p0;
        this->p1 = p1;
        this->p2 = p2;
        this->ip0 = ip0;
        this->ip1 = ip1;
    }
    Facet(uint32_t s, uint32_t n, uint32_t p0, uint32_t p1, uint32_t p2, uint32_t cg)
    {
        for(uint32_t i=0; i<n; i++)
        {
            this->segments.push_back(i+s);
        }
        this->p0 = p0;
        this->p1 = p1;
        this->p2 = p2;
        this->ip0 = cg;
    }
    Facet(const Facet& other)
    {
        this->segments = other.segments;
        this->p0 = other.p0;
        this->p1 = other.p1;
        this->p2 = other.p2;
        this->w0 = other.w0;
        this->w1 = other.w1;
        this->ip0 = other.ip0;
        this->ip1 = other.ip1;
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
    std::vector<uint32_t> get_vertices(std::vector<Segment>& all_segments)
    {
        std::unordered_set<uint32_t> res;
        for(uint32_t s : segments)
        {
            res.insert(all_segments[s].e0);
            res.insert(all_segments[s].e1);
        }
        return std::vector<uint32_t>(res.begin(),res.end());
    }
    void calculate_implicit_centroid(std::vector<double3>& approximated_vertices, std::vector<Segment>& segments)
    {
        std::vector<uint32_t> vs = get_vertices(segments);
        double3 centroid;
        for(uint32_t v : vs)
        {
            centroid += approximated_vertices[v];
        }
        centroid /= (double)vs.size();
        double3 pp0 = approximated_vertices[p0];
        double3 pp1 = approximated_vertices[p1];
        double3 pp2 = approximated_vertices[p2];
        auto [is_in, w] = barycentric_weight(pp0,pp1,pp2,centroid);
        w0 = w.x;
        w1 = w.y;
    }
    std::shared_ptr<genericPoint> get_implicit_centroid(std::vector<std::shared_ptr<genericPoint>>& vertices)
    {
        return std::make_shared<implicitPoint3D_BPT>
        (vertices[p0]->toExplicit3D(),vertices[p1]->toExplicit3D(),vertices[p2]->toExplicit3D(),w0,w1);
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
    std::pair<double3, double> get_bounding_sphere(std::vector<double3>& approximated_vertices, std::vector<Segment>& segments, bool square_radius=true, double padding=1.05)
    {
        std::vector<uint32_t> vs = get_vertices(segments);
        double3 centroid(0.0,0.0,0.0);
        for(uint32_t i : vs)
        {
            centroid += approximated_vertices[i];
        }
        centroid /= (double)vs.size();
        
        double radius = -1.0;
        for(uint32_t i : vs)
        {
            radius = std::max(radius, (centroid-approximated_vertices[i]).length_squared());
        }
        if(!square_radius)
        {
            radius = std::sqrt(radius);
        }
        return std::make_pair(centroid, padding*radius);
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
        double w0, w1;
        uint32_t ip0, ip1;
            
        FacetInteropData& operator=(const Facet& other)
        {
            this->segments = other.segments.data();
            segments_count = other.segments.size();
            this->p0 = other.p0;
            this->p1 = other.p1;
            this->p2 = other.p2;
            this->w0 = other.w0;
            this->w1 = other.w1;
            this->ip0 = other.ip0;
            this->ip1 = other.ip1;
            return *this;
        }
        
        Facet to_facet()
        {
            Facet res;
            res.segments = std::vector<uint32_t>(segments,segments+segments_count);
            res.p0 = p0;
            res.p1 = p1;
            res.p2 = p2;
            res.w0 = w0;
            res.w1 = w1;
            res.ip0 = ip0;
            res.ip1 = ip1;
            return res;
        }
    };
}

#endif
