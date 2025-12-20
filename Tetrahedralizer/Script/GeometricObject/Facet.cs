using System;
using System.Collections.Generic;
using System.Runtime.InteropServices;

namespace Hanzzz.Tetrahedralizer
{
    [Serializable]
    public class Facet
    {
        public List<int> segments; // segments forming the facet
        public int p0,p1,p2; // three explicit vetices that define the facet
        public double w0,w1; // w0+w1+w2==1 and w0*p0+w1*p1+w2*p2 is the facet centroid
        public int ip0, ip1; // two incident polyhedrons
        public List<int> constarints; // coplanar constraints that may intersect the facet

        public Facet(FacetInteropData facetInteropData)
        {
            segments = facetInteropData.segments.ReadInt32Repeat(facetInteropData.segmentCount);
            p0 = facetInteropData.p0;
            p1 = facetInteropData.p1;
            p2 = facetInteropData.p2;
            w0 = facetInteropData.w0;
            w1 = facetInteropData.w1;
            ip0 = facetInteropData.ip0;
            ip1 = facetInteropData.ip1;
            constarints = facetInteropData.constarints.ReadInt32Repeat(facetInteropData.constarintsCount);
        }
    }

    [StructLayout(LayoutKind.Sequential)]
    public struct FacetInteropData
    {
        public IntPtr segments;
        public int segmentCount;
        public int p0, p1, p2;
        public double w0, w1;
        public int ip0, ip1;
        public IntPtr constarints;
        public int constarintsCount;


        public FacetInteropData(Facet facet, out GCHandle segmentsHandle, out GCHandle constarintsHandle)
        {
            segmentsHandle = GCHandle.Alloc(facet.segments.ToArray(), GCHandleType.Pinned);
            segments = segmentsHandle.AddrOfPinnedObject();
            segmentCount = facet.segments.Count;
            p0 = facet.p0;
            p1 = facet.p1;
            p2 = facet.p2;
            w0 = facet.w0;
            w1 = facet.w1;
            ip0 = facet.ip0;
            ip1 = facet.ip1;
            constarintsHandle = GCHandle.Alloc(facet.constarints.ToArray(), GCHandleType.Pinned);
            constarints = constarintsHandle.AddrOfPinnedObject();
            constarintsCount = facet.constarints.Count;
        }
    }
}