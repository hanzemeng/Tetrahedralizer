using System;
using UnityEngine.Rendering;


namespace Hanzzz.Tetrahedralizer
{
    public static class VertexAttributeDescriptorSerializer
    {
        public static Int32 ToInt32(VertexAttributeDescriptor v)
        {
            Int32 res = 0;
            Int32 mask = 0x000000ff;
            res |= (Int32)v.attribute & mask;
            res <<= 8;
            res |= (Int32)v.format & mask;
            res <<= 8;
            res |= (Int32)v.dimension & mask;
            res <<= 8;
            res |= (Int32)v.stream & mask;
            return res;
        }
    
        public static VertexAttributeDescriptor ToVertexAttributeDescriptor(Int32 v)
        {
            Int32 mask = 0x000000ff;
            Int32 r0 = v&mask;
            v >>= 8;
            Int32 r1 = v&mask;
            v >>= 8;
            Int32 r2 = v&mask;
            v >>= 8;
    
            return new VertexAttributeDescriptor(
                (VertexAttribute)v,
                (VertexAttributeFormat)r2,
                (int)r1,
                (int)r0);
        }
    }
    
}