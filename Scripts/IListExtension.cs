using System;
using System.Collections.Generic;

public static class IListExtension
{
    public static void ReplaceWith<T>(this IList<T> des, IList<T> src)
    {
        des.Clear();
        for(Int32 i=0; i<src.Count; i++)
        {
            des.Add(src[i]);
        }
    }
}
