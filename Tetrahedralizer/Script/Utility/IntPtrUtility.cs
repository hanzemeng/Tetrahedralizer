using System;
using System.Runtime.InteropServices;

public static class IntPtrUtility
{
    public static int ReadInt32(this ref IntPtr ptr)
    {
        int res = Marshal.ReadInt32(ptr);
        ptr =  IntPtr.Add(ptr, 4);
        return res;
    }

    public static double ReadDouble(this ref IntPtr ptr)
    {
        double res = BitConverter.Int64BitsToDouble(Marshal.ReadInt64(ptr));
        ptr =  IntPtr.Add(ptr, 8);
        return res;
    }
}
