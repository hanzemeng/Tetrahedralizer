using UnityEngine;


namespace Hanzzz.Tetrahedralizer
{
    public static class Sorter
    {
        public static void SortInt2(ref int p0, ref int p1)
        {
            int temp;
            if(p0 > p1)
            {
                temp = p1;
                p1 = p0;
                p0 = temp;
            }
        }
        public static void SortInt3(ref int p0, ref int p1, ref int p2)
        {
            int temp;
            if(p0 > p1)
            {
                temp = p1;
                p1 = p0;
                p0 = temp;
            }
            if(p1 > p2)
            {
                temp = p2;
                p2 = p1;
                p1 = temp;
            }
            if(p0 > p1)
            {
                temp = p1;
                p1 = p0;
                p0 = temp;
            }
        }
        public static void SortVector3(ref Vector3 p0, ref Vector3 p1, ref Vector3 p2)
        {
            Vector3 temp;
            if(p0.GreaterThan(p1) )
            {
                temp = p1;
                p1 = p0;
                p0 = temp;
            }
            if(p1.GreaterThan(p2))
            {
                temp = p2;
                p2 = p1;
                p1 = temp;
            }
            if(p0.GreaterThan(p1))
            {
                temp = p1;
                p1 = p0;
                p0 = temp;
            }
        }
    }
    
}