using System;
using System.Collections.Generic;

public static class ListExtension
{
    public static T PopBack<T>(this List<T> list)
    {
        int i = list.Count-1;
        T res = list[i];
        list.RemoveAt(i);
        return res;
    }
    public static void PopBackRepeat<T>(this List<T> list, int n)
    {
        for(int i=0; i<n; i++)
        {
            list.PopBack();
        }
    }

    public static void ReplaceWith<T>(this IList<T> des, IList<T> src)
    {
        des.Clear();
        for(Int32 i=0; i<src.Count; i++)
        {
            des.Add(src[i]);
        }
    }

    public static void Add<T>(this List<T> list, params T[] items)
    {
        for(int i=0; i<items.Length; i++)
        {
            list.Add(items[i]);
        }
    }
}
