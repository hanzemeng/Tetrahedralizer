using System.Linq;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.Rendering;

public class MeshTriangleFinder
{
    private List<Vector3> m_sourcePositions;
    private List<int> m_sourceTriangles;
    private List<Vector3> m_sourceTrianglesNormal;
    private Dictionary<Vector3, List<(int, float)>> m_distances;


    public MeshTriangleFinder()
    {
        m_sourcePositions = new List<Vector3>();
        //m_sourceTriangles = new List<int>();
        m_sourceTrianglesNormal = new List<Vector3>();
        m_distances = new Dictionary<Vector3, List<(int, float)>>();
    }

    public void AssignSourceMesh(Mesh sourceMesh)
    {
        sourceMesh.GetVertices(m_sourcePositions);
        m_sourceTriangles = sourceMesh.triangles.ToList();
        for(int i=0; i<m_sourceTriangles.Count; i+=3)
        {
            Vector3 v0 = m_sourcePositions[m_sourceTriangles[i+0]];
            Vector3 v1 = m_sourcePositions[m_sourceTriangles[i+1]];
            Vector3 v2 = m_sourcePositions[m_sourceTriangles[i+2]];

            m_sourceTrianglesNormal.Add(Vector3.Cross(v1-v0,v2-v1).normalized);
        }
    }

    public int FindClosestTriangle(Vector3 point, Vector3 pointNormal, double normalAlignRequirement=0.95d)
    {
        pointNormal = pointNormal.normalized;
        if(Vector3.zero == pointNormal)
        {
            return -1;
        }

        List<(int, float)> distance;
        if(!m_distances.TryGetValue(point, out distance))
        {
            distance = new List<(int, float)>();
            for(int i=0; i<m_sourceTriangles.Count; i+=3)
            {
                Vector3 v0 = m_sourcePositions[m_sourceTriangles[i+0]];
                Vector3 v1 = m_sourcePositions[m_sourceTriangles[i+1]];
                Vector3 v2 = m_sourcePositions[m_sourceTriangles[i+2]];

                Vector3 closest = ClosestPointOnTriangle(point, v0, v1, v2);
                float sqrDist = (point - closest).sqrMagnitude;
                distance.Add((i/3, sqrDist));
            }
            distance = distance.OrderBy(i=>i.Item2).ToList();
            m_distances[point] = distance;
        }

        for(int i=0; i<distance.Count; i++)
        {
            Vector3 triNormal = m_sourceTrianglesNormal[distance[i].Item1];
            if(Vector3.Dot(triNormal, pointNormal) >= normalAlignRequirement)
            {
                return distance[i].Item1;
            }
        }
        return -1;
    }


    private Vector3 ClosestPointOnTriangle(Vector3 p, Vector3 a, Vector3 b, Vector3 c)
    {
        const float epsilon = 1e-8f;

        Vector3 ab = b - a;
        Vector3 ac = c - a;
        Vector3 ap = p - a;

        float d1 = Vector3.Dot(ab, ap);
        float d2 = Vector3.Dot(ac, ap);
        if (d1 <= 0f && d2 <= 0f) return a; // vertex region A

        Vector3 bp = p - b;
        float d3 = Vector3.Dot(ab, bp);
        float d4 = Vector3.Dot(ac, bp);
        if (d3 >= 0f && d4 <= d3) return b; // vertex region B

        Vector3 cp = p - c;
        float d5 = Vector3.Dot(ab, cp);
        float d6 = Vector3.Dot(ac, cp);
        if (d6 >= 0f && d5 <= d6) return c; // vertex region C

        // Check edge AB
        float vc = d1 * d4 - d3 * d2;
        float edgeDenom = d1 - d3;
        if (vc <= 0f && d1 >= 0f && d3 <= 0f)
        {
            if (Mathf.Abs(edgeDenom) < epsilon) return a;
            float v = d1 / edgeDenom;
            return a + v * ab;
        }

        // Check edge AC
        float vb = d5 * d2 - d1 * d6;
        edgeDenom = d2 - d6;
        if (vb <= 0f && d2 >= 0f && d6 <= 0f)
        {
            if (Mathf.Abs(edgeDenom) < epsilon) return a;
            float w = d2 / edgeDenom;
            return a + w * ac;
        }

        // Check edge BC
        float va = d3 * d6 - d5 * d4;
        edgeDenom = (d4 - d3) + (d5 - d6);
        if (va <= 0f && (d4 - d3) >= 0f && (d5 - d6) >= 0f)
        {
            if (Mathf.Abs(edgeDenom) < epsilon) return b;
            float w = (d4 - d3) / edgeDenom;
            return b + w * (c - b);
        }

        // Inside face region
        float denom = va + vb + vc;
        if (Mathf.Abs(denom) < epsilon) return a; // fallback for degenerate triangle
        float invDenom = 1f / denom;
        float vFinal = vb * invDenom;
        float wFinal = vc * invDenom;
        return a + ab * vFinal + ac * wFinal;
    }

}
