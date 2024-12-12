using System.Collections.Generic;
using UnityEngine;

public static class VerticesRemover 
{
    private static Dictionary<Vector3, int> m_mapping = new Dictionary<Vector3, int>();


    public static void RemoveDuplicateVertices(List<Vector3> vertices, List<int> indexes)
    {
        m_mapping.Clear();

        for(int i=0; i<vertices.Count; i++)
        {
            if(!m_mapping.ContainsKey(vertices[i]))
            {
                m_mapping[vertices[i]] = m_mapping.Count;
            }
        }

        for(int i = 0; i<indexes.Count; i++)
        {
            indexes[i] = m_mapping[vertices[indexes[i]]];
        }
        foreach(var kvp in m_mapping)
        {
            vertices[kvp.Value] = kvp.Key;
        }
        vertices.RemoveRange(m_mapping.Count,vertices.Count-m_mapping.Count);
    }
}
