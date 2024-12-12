using System;
using System.Collections.Generic;

[Serializable]
public struct NineInt32
{
    public Int32 p0,p1,p2,p3,p4,p5,p6,p7,p8;

    public Int32 this[Int32 index]
    {
        get => GetValue(index);
        set => SetValue(index,value);
    }

    public Int32 GetValue(Int32 index)
    {
        switch(index) 
        {
            case 0:
            return p0;
            case 1:
            return p1;
            case 2:
            return p2;
            case 3:
            return p3;
            case 4:
            return p4;
            case 5:
            return p5;
            case 6:
            return p6;
            case 7:
            return p7;
            case 8:
            return p8;
            default:
            throw new Exception($"index {index} out of range.");
        }
    }
    public void SetValue(Int32 index, Int32 value)
    {
        switch(index) 
        {
            case 0:
            p0 = value;
            break;
            case 1:
            p1 = value;
            break;
            case 2:
            p2 = value;
            break;
            case 3:
            p3 = value;
            break;
            case 4:
            p4 = value;
            break;
            case 5:
            p5 = value;
            break;
            case 6:
            p6 = value;
            break;
            case 7:
            p7 = value;
            break;
            case 8:
            p8 = value;
            break;
            default:
            throw new Exception($"index {index} out of range.");
        }
    }
}


[Serializable]
public class ListInt32
{
    public List<Int32> list;

    public ListInt32()
    {
        list = new List<Int32>();
    }

    public Int32 this[Int32 index]
    {
        get => list[index];
        set => list[index] = value;
    }

    public void Add(Int32 v)
    {
        list.Add(v);
    }
    public void Clear()
    {
        list.Clear();
    }
}
