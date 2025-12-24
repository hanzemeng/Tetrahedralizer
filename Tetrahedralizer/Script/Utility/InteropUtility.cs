using System;
using System.Collections.Generic;

namespace Hanzzz.Tetrahedralizer
{
    public static class InteropUtility
    {
        public static List<T> GetList<T>(IntPtr handle, Func<IntPtr, int> sizeFunction, Action<IntPtr, T[]> function)
        {
            T[] buffer = new T[sizeFunction(handle)];
            function(handle, buffer);
            return new List<T>(buffer);
        }
    }
}