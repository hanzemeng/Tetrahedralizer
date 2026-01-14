#ifndef segment_h
#define segment_h

#include "common_header.h"

class Segment
{
    public:
    uint32_t e0, e1; // two endpoints
    uint32_t p0, p1, p2, p3, p4, p5; // p0, p1, p2 is a plane, p3, p4, p5 is a plane. The line is the intersection of the two plane. Only use p0, p1 if both are explicit points.

    Segment()
    {
        e0 = e1 = p0 = p1 = p2 = p3 = p4 = p5 = UNDEFINED_VALUE;
    }
    Segment(uint32_t p) // not a segment, just a point
    {
        e0 = p;
        e1 = p0 = p1 = p2 = p3 = p4 = p5 = UNDEFINED_VALUE;
    }
    Segment(uint32_t p, uint32_t q)
    {
        e0 = p;
        e1 = q;
        p0 = p;
        p1 = q;
        p2 = p3 = p4 = p5 = UNDEFINED_VALUE;
    }
    Segment(uint32_t ie0, uint32_t ie1, uint32_t ip0, uint32_t ip1, uint32_t ip2, uint32_t ip3, uint32_t ip4, uint32_t ip5)
    {
        e0 = ie0;
        e1 = ie1;
        p0 = ip0;
        p1 = ip1;
        p2 = ip2;
        p3 = ip3;
        p4 = ip4;
        p5 = ip5;
    }
    Segment(const Segment& other)
    {
        e0 = other.e0;
        e1 = other.e1;
        p0 = other.p0;
        p1 = other.p1;
        p2 = other.p2;
        p3 = other.p3;
        p4 = other.p4;
        p5 = other.p5;
    }
    Segment& operator=(const Segment& other)
    {
        e0 = other.e0;
        e1 = other.e1;
        p0 = other.p0;
        p1 = other.p1;
        p2 = other.p2;
        p3 = other.p3;
        p4 = other.p4;
        p5 = other.p5;
        return *this;
    }
    
    void increase_vertices_indexes(uint32_t threshold, uint32_t amount)
    {
        auto increase = [&](uint32_t o) -> uint32_t
        {
            if(UNDEFINED_VALUE==o || o<threshold)
            {
                return o;
            }
            return o+amount;
        };
        e0 = increase(e0);
        e1 = increase(e1);
        p0 = increase(p0);
        p1 = increase(p1);
        p2 = increase(p2);
        p3 = increase(p3);
        p4 = increase(p4);
        p5 = increase(p5);
    }
    
    static void sort_segments(std::vector<Segment>& segments)
    {
        for(uint32_t i=0; i<segments.size(); i++)
        {
            for(uint32_t j=i+1; j<segments.size(); j++)
            {
                if(segments[i].e0 == segments[j].e0 ||
                   segments[i].e0 == segments[j].e1 ||
                   segments[i].e1 == segments[j].e0 ||
                   segments[i].e1 == segments[j].e1)
                {
                    std::swap(segments[i+1],segments[j]);
                    break;
                }
            }
        }
    }
    //assume sorted
    static std::vector<uint32_t> get_segments_vertices(const std::vector<Segment>& segments)
    {
        std::deque<uint32_t> res;
        for(uint32_t i=0; i<segments.size()-1; i++)
        {
            uint32_t e0 = segments[i].e0;
            uint32_t e1 = segments[i].e1;
            if(0 == i)
            {
                res.push_back(e0);
                res.push_back(e1);
                continue;
            }
            uint32_t be0 = res.front();
            uint32_t be1 = res.back();
         
            if(be0 == e0)
            {
                res.push_front(e1);
            }
            else if(be0 == e1)
            {
                res.push_front(e0);
            }
            else if(be1 == e0)
            {
                res.push_back(e1);
            }
            else if(be1 == e1)
            {
                res.push_back(e0);
            }
            else
            {
                throw "something is wrong";
            }
        }
        
        if((segments.back().e0 != res[0] && segments.back().e0 != res.back()) ||
           (segments.back().e1 != res[0] && segments.back().e1 != res.back()))
        {
            throw "something is wrong";
        }
        return std::vector<uint32_t>(res.begin(),res.end());
    }
    
    // return intersection, top segment, bot segment, vertices made up the intersection (if any).
    static std::tuple<uint32_t, uint32_t, uint32_t, std::vector<uint32_t>> slice_segment_with_plane(uint32_t s, uint32_t c0,uint32_t c1,uint32_t c2, std::vector<std::shared_ptr<genericPoint>>& vertices, std::vector<Segment>& segments, std::unordered_map<uint32_t, int>& orientation_cache, bool modify_same_segment=true)
    {
        uint32_t e0 = segments[s].e0;
        uint32_t e1 = segments[s].e1;
        if(orientation_cache.end() == orientation_cache.find(e0))
        {
            orientation_cache[e0] = orient3d(c0,c1,c2,e0,vertices.data());
        }
        if(orientation_cache.end() == orientation_cache.find(e1))
        {
            orientation_cache[e1] = orient3d(c0,c1,c2, e1, vertices.data());
        }
        int o0 = orientation_cache[e0];
        int o1 = orientation_cache[e1];
        
        if(0 == o0)
        {
            if(0 == o1)
            {
                return std::make_tuple(UNDEFINED_VALUE,UNDEFINED_VALUE,UNDEFINED_VALUE,std::vector<uint32_t>());
            }
            else if(1 == o1)
            {
                return std::make_tuple(e0,s,UNDEFINED_VALUE,std::vector<uint32_t>());
            }
            else
            {
                return std::make_tuple(e0,UNDEFINED_VALUE,s,std::vector<uint32_t>());
            }
        }
        else if(1 == o0)
        {
            if(0 == o1)
            {
                return std::make_tuple(e1,s,UNDEFINED_VALUE,std::vector<uint32_t>());
            }
            else if(1 == o1)
            {
                return std::make_tuple(UNDEFINED_VALUE,s,UNDEFINED_VALUE,std::vector<uint32_t>());
            }
            else
            {
                uint32_t new_i = vertices.size();
                std::vector<uint32_t> new_vs;
                new_vs.push_back(segments[s].p0);
                new_vs.push_back(segments[s].p1);
                if(UNDEFINED_VALUE == segments[s].p2)
                {
                    new_vs.push_back(c0);
                    new_vs.push_back(c1);
                    new_vs.push_back(c2);
                    vertices.push_back(std::make_shared<implicitPoint3D_LPI>(
                                                                             vertices[new_vs[0]]->toExplicit3D(),vertices[new_vs[1]]->toExplicit3D(),
                                                                             vertices[new_vs[2]]->toExplicit3D(),vertices[new_vs[3]]->toExplicit3D(),vertices[new_vs[4]]->toExplicit3D()));
                }
                else
                {
                    new_vs.push_back(segments[s].p2);
                    new_vs.push_back(segments[s].p3);
                    new_vs.push_back(segments[s].p4);
                    new_vs.push_back(segments[s].p5);
                    new_vs.push_back(c0);
                    new_vs.push_back(c1);
                    new_vs.push_back(c2);
                    vertices.push_back(std::make_shared<implicitPoint3D_TPI>(
                                                                             vertices[new_vs[0]]->toExplicit3D(),vertices[new_vs[1]]->toExplicit3D(),vertices[new_vs[2]]->toExplicit3D(),
                                                                             vertices[new_vs[3]]->toExplicit3D(),vertices[new_vs[4]]->toExplicit3D(),vertices[new_vs[5]]->toExplicit3D(),
                                                                             vertices[new_vs[6]]->toExplicit3D(),vertices[new_vs[7]]->toExplicit3D(),vertices[new_vs[8]]->toExplicit3D()));
                }
                
                uint32_t t_s;
                if(modify_same_segment)
                {
                    t_s = s;
                }
                else
                {
                    t_s = segments.size();
                    segments.push_back(Segment(segments[s]));
                }
                segments[t_s].e0 = e0;
                segments[t_s].e1 = new_i;
                uint32_t b_s = segments.size();
                segments.push_back(Segment(segments[t_s]));
                segments[b_s].e0 = new_i;
                segments[b_s].e1 = e1;
                return std::make_tuple(new_i,t_s,b_s,new_vs);
            }
        }
        else
        {
            if(0 == o1)
            {
                return std::make_tuple(e1,UNDEFINED_VALUE,s,std::vector<uint32_t>());
            }
            else if(1 == o1)
            {
                uint32_t new_i = vertices.size();
                std::vector<uint32_t> new_vs;
                new_vs.push_back(segments[s].p0);
                new_vs.push_back(segments[s].p1);
                if(UNDEFINED_VALUE == segments[s].p2)
                {
                    new_vs.push_back(c0);
                    new_vs.push_back(c1);
                    new_vs.push_back(c2);
                    vertices.push_back(std::make_shared<implicitPoint3D_LPI>(
                                                                             vertices[new_vs[0]]->toExplicit3D(),vertices[new_vs[1]]->toExplicit3D(),
                                                                             vertices[new_vs[2]]->toExplicit3D(),vertices[new_vs[3]]->toExplicit3D(),vertices[new_vs[4]]->toExplicit3D()));
                }
                else
                {
                    new_vs.push_back(segments[s].p2);
                    new_vs.push_back(segments[s].p3);
                    new_vs.push_back(segments[s].p4);
                    new_vs.push_back(segments[s].p5);
                    new_vs.push_back(c0);
                    new_vs.push_back(c1);
                    new_vs.push_back(c2);
                    vertices.push_back(std::make_shared<implicitPoint3D_TPI>(
                                                                             vertices[new_vs[0]]->toExplicit3D(),vertices[new_vs[1]]->toExplicit3D(),vertices[new_vs[2]]->toExplicit3D(),
                                                                             vertices[new_vs[3]]->toExplicit3D(),vertices[new_vs[4]]->toExplicit3D(),vertices[new_vs[5]]->toExplicit3D(),
                                                                             vertices[new_vs[6]]->toExplicit3D(),vertices[new_vs[7]]->toExplicit3D(),vertices[new_vs[8]]->toExplicit3D()));
                }
                uint32_t t_s;
                if(modify_same_segment)
                {
                    t_s = s;
                }
                else
                {
                    t_s = segments.size();
                    segments.push_back(Segment(segments[s]));
                }
                segments[t_s].e0 = e1;
                segments[t_s].e1 = new_i;
                uint32_t b_s = segments.size();
                segments.push_back(Segment(segments[t_s]));
                segments[b_s].e0 = new_i;
                segments[b_s].e1 = e0;
                return std::make_tuple(new_i,t_s,b_s,new_vs);
            }
            else
            {
                return std::make_tuple(UNDEFINED_VALUE,UNDEFINED_VALUE,s,std::vector<uint32_t>());
            }
        }
    }
};

extern "C"
{
    struct SegmentInteropData
    {
        public:
        uint32_t e0, e1;
        uint32_t p0, p1, p2, p3, p4, p5;
            
        SegmentInteropData& operator=(const Segment& other)
        {
            e0 = other.e0;
            e1 = other.e1;
            p0 = other.p0;
            p1 = other.p1;
            p2 = other.p2;
            p3 = other.p3;
            p4 = other.p4;
            p5 = other.p5;
            return *this;
        }
        Segment to_segment()
        {
            return Segment(e0,e1,p0,p1,p2,p3,p4,p5);
        }
    };
}


#endif
