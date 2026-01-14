using System;
using System.Linq;
using System.Collections.Generic;
using System.Runtime.InteropServices;

namespace Hanzzz.Tetrahedralizer
{
    [Serializable]
    public class Segment
    {
        public int e0, e1; // two endpoints
        public int p0, p1, p2, p3, p4, p5; // p0, p1, p2 is a plane, p3, p4, p5 is a plane. The line is the intersection of the two plane. Only use p0, p1 if both are explicit points.

        public Segment(SegmentInteropData segmentInteropData)
        {
            e0 = segmentInteropData.e0;
            e1 = segmentInteropData.e1;
            p0 = segmentInteropData.p0;
            p1 = segmentInteropData.p1;
            p2 = segmentInteropData.p2;
            p3 = segmentInteropData.p3;
            p4 = segmentInteropData.p4;
            p5 = segmentInteropData.p5;
        }

        public static void SortSegments(List<Segment> segments)
        {
            for(int i=0; i<segments.Count; i++)
            {
                for(int j=i+1; j<segments.Count; j++)
                {
                    if(segments[i].e0 == segments[j].e0 ||
                       segments[i].e0 == segments[j].e1 ||
                       segments[i].e1 == segments[j].e0 ||
                       segments[i].e1 == segments[j].e1)
                    {
                        Segment temp = segments[i+1];
                        segments[i+1] = segments[j];
                        segments[j] = temp;
                        break;
                    }
                }
            }
        }
        //assume sorted
        public static List<int> GetSegmentsVertices(List<Segment> segments)
        {
            LinkedList<int> res = new LinkedList<int>();
            for(int i=0; i<segments.Count-1; i++)
            {
                int e0 = segments[i].e0;
                int e1 = segments[i].e1;
                if(0 == i)
                {
                    res.AddLast(e0);
                    res.AddLast(e1);
                    continue;
                }
                int be0 = res.First.Value;
                int be1 = res.Last.Value;
             
                if(be0 == e0)
                {
                    res.AddFirst(e1);
                }
                else if(be0 == e1)
                {
                    res.AddFirst(e0);
                }
                else if(be1 == e0)
                {
                    res.AddLast(e1);
                }
                else if(be1 == e1)
                {
                    res.AddLast(e0);
                }
                else
                {
                    throw new Exception("something is wrong");
                }
            }
            return res.ToList();
        }
    }

    [StructLayout(LayoutKind.Sequential)]
    public struct SegmentInteropData
    {
        public int e0, e1;
        public int p0, p1, p2, p3, p4, p5;

        public SegmentInteropData(Segment segment)
        {
            e0 = segment.e0;
            e1 = segment.e1;
            p0 = segment.p0;
            p1 = segment.p1;
            p2 = segment.p2;
            p3 = segment.p3;
            p4 = segment.p4;
            p5 = segment.p5;
        }
    }
}