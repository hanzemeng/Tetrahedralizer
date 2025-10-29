using System.Linq;
using System.Collections.Generic;
using UnityEngine;

public static class MeshExtension
{
    public static int GetIndexCount(this Mesh mesh)
    {
        int res = 0;
        for(int i=mesh.subMeshCount-1; i>=0; i--)
        {
            res += (int)mesh.GetIndexCount(i); // now we use uint?
        }
        return res;
    }

    
}

public static class MeshUtility
{
    private static Mesh m_emptyMesh = new Mesh();
    public static Mesh GetEmptyMesh()
    {
        if(null == m_emptyMesh)
        {
            m_emptyMesh = new Mesh();
        }
        return m_emptyMesh;
    }

    // GPT generated
    public static void GenerateFacetsUVs(Mesh mesh, List<List<int>> facetIndices)
    {
        Vector3[] vertices = mesh.vertices;
        Vector3[] normals = mesh.normals;
        Vector2[] uvs = new Vector2[vertices.Length];

        // Step 1: project facets to local 2D planes
        List<(List<Vector2> shape, float area)> facetShapes = new List<(List<Vector2> shape, float area)>();
        foreach (var indices in facetIndices)
        {
            Vector3 n = normals[indices[0]];
            Vector3 u = Vector3.zero, v = Vector3.zero;
            Vector3.OrthoNormalize(ref n, ref u, ref v);

            List<Vector2> shape = new List<Vector2>();
            foreach (int i in indices)
            {
                float x = Vector3.Dot(vertices[i], u);
                float y = Vector3.Dot(vertices[i], v);
                shape.Add(new Vector2(x, y));
            }

            // compute signed area (shoelace formula)
            float area = 0;
            for (int i = 0; i < shape.Count; i++)
            {
                Vector2 a = shape[i];
                Vector2 b = shape[(i + 1) % shape.Count];
                area += (a.x * b.y - a.y * b.x);
            }
            area = Mathf.Abs(area) * 0.5f;

            facetShapes.Add((shape, area));
        }

        // Step 2: normalize relative areas
        float totalArea = facetShapes.Sum(f => f.area);
        for(int i=0; i<facetShapes.Count; i++)
        {
            facetShapes[i] = (facetShapes[i].shape, facetShapes[i].area/ totalArea);
        }

        // Step 3: simple packing layout
        float cursorX = 0, cursorY = 0, maxRowHeight = 0;
        const float padding = 0.01f;
        const float atlasSize = 1.0f;

        for (int f = 0; f < facetIndices.Count; f++)
        {
            var (shape, area) = facetShapes[f];
            var indices = facetIndices[f];

            // scale facet by sqrt of its relative area (so area ratio preserved)
            float scale = Mathf.Sqrt(area) * 0.8f; // 0.8 leaves margin
            var scaled = shape.Select(p => p * scale).ToList();

            // compute bbox
            float minX = scaled.Min(p => p.x);
            float minY = scaled.Min(p => p.y);
            float maxX = scaled.Max(p => p.x);
            float maxY = scaled.Max(p => p.y);
            float width = maxX - minX;
            float height = maxY - minY;

            // wrap to next row if needed
            if (cursorX + width + padding > atlasSize)
            {
                cursorX = 0;
                cursorY += maxRowHeight + padding;
                maxRowHeight = 0;
            }

            // apply offset to place facet in atlas
            Vector2 offset = new Vector2(cursorX - minX, cursorY - minY);
            for (int i = 0; i < indices.Count; i++)
            {
                Vector2 uv = scaled[i] + offset;
                uvs[indices[i]] = uv;
            }

            cursorX += width + padding;
            maxRowHeight = Mathf.Max(maxRowHeight, height);
        }

        // Step 4: normalize UVs to [0,1]
        float maxU = uvs.Max(p => p.x);
        float maxV = uvs.Max(p => p.y);
        for (int i = 0; i < uvs.Length; i++)
            uvs[i] = new Vector2(uvs[i].x / maxU, uvs[i].y / maxV);

        mesh.uv = uvs;
    }
}
