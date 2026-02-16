using System;
using System.IO;
using System.Collections.Generic;
using System.Runtime.InteropServices;

namespace Hanzzz.Tetrahedralizer
{
    [Serializable]
    public class Facet
    {
        public List<int> segments; // segments forming the facet
        public int p0,p1,p2; // three explicit vetices that define the facet

        public Facet(BinaryReader reader)
        {
            int segmentsCount = reader.ReadInt32();
            segments = new List<int>(segmentsCount);
            for(int i=0; i<segmentsCount; i++)
            {
                segments.Add(reader.ReadInt32());
            }
            p0 = reader.ReadInt32();
            p1 = reader.ReadInt32();
            p2 = reader.ReadInt32();
        }

        public Facet(FacetInteropData facetInteropData)
        {
            segments = facetInteropData.segments.ReadInt32Repeat(facetInteropData.segmentCount);
            p0 = facetInteropData.p0;
            p1 = facetInteropData.p1;
            p2 = facetInteropData.p2;
        }
    }

    [StructLayout(LayoutKind.Sequential)]
    public struct FacetInteropData
    {
        public IntPtr segments;
        public int segmentCount;
        public int p0, p1, p2;


        public FacetInteropData(Facet facet, out GCHandle segmentsHandle)
        {
            segmentsHandle = GCHandle.Alloc(facet.segments.ToArray(), GCHandleType.Pinned);
            segments = segmentsHandle.AddrOfPinnedObject();
            segmentCount = facet.segments.Count;
            p0 = facet.p0;
            p1 = facet.p1;
            p2 = facet.p2;
        }
    }
}