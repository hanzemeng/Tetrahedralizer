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
        return std::vector<uint32_t>(res.begin(),res.end());
    }
    
    // the vertex on the plane, the segments on and above plane, the segments on and below plane
    // s0,s1,s2 is the plane defining the segments, c0,c1,c2 is the slicing plane
    static std::tuple<uint32_t, std::vector<Segment>, std::vector<Segment>> slice_segments_with_plane(const std::vector<Segment>& segments, uint32_t s0,uint32_t s1,uint32_t s2, uint32_t c0,uint32_t c1,uint32_t c2, std::vector<std::shared_ptr<genericPoint>>& vertices)
    {
        bool has_coplanar_segment = false;
        uint32_t i0(UNDEFINED_VALUE),i1(UNDEFINED_VALUE);
        std::vector<Segment> top_segments;
        std::vector<Segment> bot_segments;
        for(uint32_t i=0; i<segments.size(); i++)
        {
            uint32_t e0 = segments[i].e0;
            uint32_t e1 = segments[i].e1;
            int o0 = orient3d(c0,c1,c2, e0, vertices.data());
            int o1 = orient3d(c0,c1,c2, e1, vertices.data());
            
            if(0 == o0)
            {
                slice_segments_with_plane_helper(i0,i1,e0);
                if(0 == o1)
                {
                    has_coplanar_segment = true;
                    slice_segments_with_plane_helper(i0,i1,e1);
                    top_segments.push_back(segments[i]);
                    bot_segments.push_back(segments[i]);
                }
                else if(1 == o1)
                {
                    top_segments.push_back(segments[i]);
                }
                else
                {
                    bot_segments.push_back(segments[i]);
                }
            }
            else if(1 == o0)
            {
                if(0 == o1)
                {
                    slice_segments_with_plane_helper(i0,i1,e1);
                    top_segments.push_back(segments[i]);
                }
                else if(1 == o1)
                {
                    top_segments.push_back(segments[i]);
                }
                else
                {
                    uint32_t new_i = vertices.size();
                    if(UNDEFINED_VALUE == segments[i].p2)
                    {
                        vertices.push_back(std::make_shared<implicitPoint3D_LPI>(
                                                                                 vertices[segments[i].p0]->toExplicit3D(),vertices[segments[i].p1]->toExplicit3D(),
                                                                                 vertices[c0]->toExplicit3D(),vertices[c1]->toExplicit3D(),vertices[c2]->toExplicit3D()));
                    }
                    else
                    {
                        vertices.push_back(std::make_shared<implicitPoint3D_TPI>(
                                                                                 vertices[c0]->toExplicit3D(),vertices[c1]->toExplicit3D(),vertices[c2]->toExplicit3D(),
                                                                                 vertices[segments[i].p0]->toExplicit3D(),vertices[segments[i].p1]->toExplicit3D(),vertices[segments[i].p2]->toExplicit3D(),
                                                                                 vertices[segments[i].p3]->toExplicit3D(),vertices[segments[i].p4]->toExplicit3D(),vertices[segments[i].p5]->toExplicit3D()));
                    }
                    
                    slice_segments_with_plane_helper(i0,i1,new_i);
                    Segment top_segment(segments[i]);
                    top_segment.e0 = e0;
                    top_segment.e1 = new_i;
                    top_segments.push_back(top_segment);
                    Segment bot_segment(segments[i]);
                    bot_segment.e0 = new_i;
                    bot_segment.e1 = e1;
                    bot_segments.push_back(bot_segment);
                }
            }
            else
            {
                if(0 == o1)
                {
                    slice_segments_with_plane_helper(i0,i1,e1);
                }
                else if(1 == o1)
                {
                    uint32_t new_i = vertices.size();
                    if(UNDEFINED_VALUE == segments[i].p2)
                    {
                        vertices.push_back(std::make_shared<implicitPoint3D_LPI>(
                                                                                 vertices[segments[i].p0]->toExplicit3D(),vertices[segments[i].p1]->toExplicit3D(),
                                                                                 vertices[c0]->toExplicit3D(),vertices[c1]->toExplicit3D(),vertices[c2]->toExplicit3D()));
                    }
                    else
                    {
                        vertices.push_back(std::make_shared<implicitPoint3D_TPI>(
                                                                                 vertices[c0]->toExplicit3D(),vertices[c1]->toExplicit3D(),vertices[c2]->toExplicit3D(),
                                                                                 vertices[segments[i].p0]->toExplicit3D(),vertices[segments[i].p1]->toExplicit3D(),vertices[segments[i].p2]->toExplicit3D(),
                                                                                 vertices[segments[i].p3]->toExplicit3D(),vertices[segments[i].p4]->toExplicit3D(),vertices[segments[i].p5]->toExplicit3D()));
                    }
                    slice_segments_with_plane_helper(i0,i1,new_i);
                    Segment top_segment(segments[i]);
                    top_segment.e0 = e1;
                    top_segment.e1 = new_i;
                    top_segments.push_back(top_segment);
                    Segment bot_segment(segments[i]);
                    bot_segment.e0 = new_i;
                    bot_segment.e1 = e0;
                    bot_segments.push_back(bot_segment);
                }
                else
                {
                    bot_segments.push_back(segments[i]);
                }
            }
        }
        
        uint32_t plane_vertex = UNDEFINED_VALUE;
        if(UNDEFINED_VALUE == i1)
        {
            plane_vertex = i0; // may still be UNDEFINED_VALUE
        }
        else
        {
            if(!has_coplanar_segment)
            {
                top_segments.push_back(Segment(i0,i1,c0,c1,c2,s0,s1,s2));
                bot_segments.push_back(Segment(i0,i1,c0,c1,c2,s0,s1,s2));
            }
        }
        return std::make_tuple(plane_vertex,top_segments,bot_segments);
    }
    static void slice_segments_with_plane_helper(uint32_t& i0,uint32_t& i1,uint32_t e)
    {
        if(UNDEFINED_VALUE == i0 || e == i0)
        {
            i0 = e;
        }
        else if(UNDEFINED_VALUE == i1 || e == i1)
        {
            i1 = e;
        }
    }
};



#endif
