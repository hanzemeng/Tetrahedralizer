using System;
using System.Collections.Generic;
using System.Runtime.InteropServices;

public static class IntPtrExtension
{
    public static byte ReadByte(this ref IntPtr ptr)
    {
        byte res = Marshal.ReadByte(ptr);
        ptr =  IntPtr.Add(ptr, 1);
        return res;
    }
    public static int ReadInt32(this ref IntPtr ptr)
    {
        int res = Marshal.ReadInt32(ptr);
        ptr =  IntPtr.Add(ptr, 4);
        return res;
    }

    public static float ReadFloat(this ref IntPtr ptr)
    {
        float res = BitConverter.Int32BitsToSingle(Marshal.ReadInt32(ptr));
        ptr =  IntPtr.Add(ptr, 4);
        return res;
    }
    public static double ReadDouble(this ref IntPtr ptr)
    {
        double res = BitConverter.Int64BitsToDouble(Marshal.ReadInt64(ptr));
        ptr =  IntPtr.Add(ptr, 8);
        return res;
    }

    public static List<int> ReadInt32Repeat(this ref IntPtr ptr, int n)
    {
        List<int> res = new List<int>(n);
        for(int i=0; i<n; i++)
        {
            res.Add(ptr.ReadInt32());
        }
        return res;
    }
    public static List<double> ReadDoubleRepeat(this ref IntPtr ptr, int n)
    {
        List<double> res = new List<double>(n);
        for(int i=0; i<n; i++)
        {
            res.Add(ptr.ReadDouble());
        }
        return res;
    }

    public static List<int> ReadInt32NestedRepeat(this ref IntPtr ptr, int n)
    {
        List<int> res = new List<int>();
        for(int i=0; i<n; i++)
        {
            int k = ptr.ReadInt32();
            res.Add(k);
            for(int j=0; j<k; j++)
            {
                res.Add(ptr.ReadInt32());
            }
        }
        return res;
    }
}
