using System;
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
}
