using System;
using System.Collections.Generic;
using Unity.Collections;


namespace Hanzzz.Tetrahedralizer
{
    public static class NativeArrayExtension
    {
        public static void Resize<T>(this ref NativeArray<T> array, int size, Allocator allocator, bool copyOldElements=false) where T : struct
        {
            if(array.Length == size)
            {
                return;
            }
            NativeArray<T> newArray = new NativeArray<T>(size, allocator);
            if(copyOldElements)
            {
                int n = Math.Min(array.Length, size);
                for(int i=0; i<n; i++)
                {
                    newArray[i] = array[i];
                }
            }
            array.Dispose();
            array = newArray;
        }

        public static void Assign<T>(this ref NativeArray<T> array, IList<T> src, Allocator allocator) where T : struct
        {
            array.Resize(src.Count, allocator, false);
            for(int i=0; i<src.Count; i++)
            {
                array[i] = src[i];
            }
        }
    }
    
}