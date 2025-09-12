using System;
using System.Linq;
using System.Collections.Generic;
using UnityEngine;

public class TetrahedralizerUtility
{
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
    }

    public static bool DoubleIsSpecial(double value)
    {
        return double.IsNaN(value) || double.IsInfinity(value);
    }

    public static List<double> UnpackVector3s(IList<Vector3> input)
    {
        List<double> res = new List<double>();
        UnpackVector3s(input, res);
        return res;
    }
    public static void UnpackVector3s(IList<Vector3> input, IList<double> output)
    {
        // output.Clear();

        for(int i=0; i<input.Count; i++)
        {
            output.Add(input[i].x);
            output.Add(input[i].y);
            output.Add(input[i].z);
        }
    }

    public static List<Vector3> PackDoubles(IList<double> input)
    {
        List<Vector3> res = new List<Vector3>();
        PackDoubles(input, res);
        return res;
    }
    public static void PackDoubles(IList<double> input, IList<Vector3> output)
    {
        // output.Clear();

        for(int i=0; i<input.Count; i+=3)
        {
            output.Add(new Vector3((float)input[i+0],(float)input[i+1],(float)input[i+2]));
        }
    }

    // a flat list is: number of element, element_0, element_1...
    public static int CountFlatIListElements(IList<int> list)
    {
        if(null == list)
        {
            return 0;
        }

        int res = 0;
        for(int i=0; i<list.Count; i+=list[i]+1)
        {
            res++;
        }
        return res;
    }
    public static int CountFlatIListLength(IList<int> list)
    {
        if(null == list)
        {
            return 0;
        }

        int i;
        for(i=0; i<list.Count; i+=list[i]+1){}
        return i;
    }
    public static List<List<int>> FlatIListToNestedList(IList<int> list)
    {
        if(null == list)
        {
            return null;
        }

        List<List<int>> res = new List<List<int>>();
        for(int i=0; i<list.Count; i+=list[i]+1)
        {
            List<int> temp = new List<int>();
            for(int j=i+1; j<i+list[i]+1; j++)
            {
                temp.Add(list[j]);
            }
            res.Add(temp);
        }

        return res;
    }
    public static List<int> NestedListToFlatList(List<List<int>> nestedList)
    {
        if(null == nestedList)
        {
            return null;
        }

        List<int> res = new List<int>();
        for(int i=0; i<nestedList.Count; i++)
        {
            res.Add(nestedList[i].Count);
            for(int j=0; j<nestedList[i].Count; j++)
            {
                res.Add(nestedList[i][j]);
            }
        }

        return res;
    }

    // swap the last two elements of an interval, used to convert between left and right hand coordinate system
    public static void SwapElementsByInterval<T>(IList<T> data, int interval)
    {
        for(int i=0; i<data.Count; i+=interval)
        {
            T temp = data[i+interval-2];
            data[i+interval-2] = data[i+interval-1];
            data[i+interval-1] = temp;
        }
    }

    // modify vertices and indexes, returns List<List<int>> that maps new vertices indxes to old vertices indexes
    public static List<List<int>> RemoveDuplicateVertices(List<Vector3> vertices, IList<int> indexes)
    {
        List<List<int>> res = new List<List<int>>();
        Dictionary<Vector3, int> mapping = new Dictionary<Vector3, int>();

        for(int i=0; i<vertices.Count; i++)
        {
            int index;
            if(!mapping.TryGetValue(vertices[i], out index))
            {
                mapping[vertices[i]] = mapping.Count;
                res.Add(new List<int>{i});
            }
            else
            {
                res[index].Add(i);
            }
        }

        for(int i = 0; i<indexes.Count; i++)
        {
            indexes[i] = mapping[vertices[indexes[i]]];
        }
        foreach(var kvp in mapping)
        {
            vertices[kvp.Value] = kvp.Key;
        }

        vertices.PopBackRepeat(vertices.Count-mapping.Count);
        return res;
    }

    public static Vector3 CenterVertices(IList<Vector3> vertices)
    {
        Vector3 center = vertices.Skip(1).Aggregate(new Bounds(vertices[0], Vector3.zero), (i,j)=>{i.Encapsulate(j);return i;}).center;
        for(Int32 i=0; i<vertices.Count; i++)
        {
            vertices[i] = vertices[i] - center;
        }

        return center;
    }

    // return true if p is on or inside the triangle, false otherwise
    // always return false triangle is degenerate
    public static bool BarycentricWeight(Vector3 t0, Vector3 t1, Vector3 t2, Vector3 p, out double w0, out double w1, out double w2)
    {
        Vector3 v0 = t1 - t0;
        Vector3 v1 = t2 - t0;
        Vector3 v2 = p  - t0;

        double d00 = Vector3.Dot(v0, v0);
        double d01 = Vector3.Dot(v0, v1);
        double d11 = Vector3.Dot(v1, v1);
        double d20 = Vector3.Dot(v2, v0);
        double d21 = Vector3.Dot(v2, v1);

        double denom = d00 * d11 - d01 * d01;
        w1 = (d11 * d20 - d01 * d21) / denom;
        w2 = (d00 * d21 - d01 * d20) / denom;
        w0 = 1.0f - w1 - w2;

        return !Mathf.Approximately((float)denom, 0f) && w0>=0f && w1>=0f && w2>=0f;
    }

    public static string UpperSnakeCaseToCapitalizedWords(string input)
    {
        if(string.IsNullOrEmpty(input))
        {
            return input;
        }

        string[] parts = input.Split('_');

        for(int i=0; i<parts.Length; i++)
        {
            if(parts[i].Length > 0)
            {
                parts[i] = char.ToUpper(parts[i][0]) + parts[i].Substring(1).ToLower();
            }
        }

        return string.Join(" ", parts);
    }

    public static GameObject CreateGameObject(Mesh mesh, List<Material> materials, Transform parent=null, Vector3 position=default)
    {
        GameObject g = new GameObject();
        g.transform.SetParent(parent, false);
        g.transform.localPosition = position;
        g.AddComponent<MeshFilter>().mesh = mesh;
        g.AddComponent<MeshRenderer>().SetMaterials(materials);

        return g;
    }
}
