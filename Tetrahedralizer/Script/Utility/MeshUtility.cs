using System.Linq;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.Rendering;


namespace Hanzzz.Tetrahedralizer
{
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
    
        // for every triangle, identify the uv island index it belongs in
        public static int[] GetUV0IslandsIndexes(this Mesh mesh)
        {
            if(!mesh.HasVertexAttribute(VertexAttribute.TexCoord0))
            {
                return Enumerable.Repeat(0,mesh.GetIndexCount()/3).ToArray();
            }
            Vector2[] uvs = mesh.uv;
            int[] tris = mesh.triangles;
            List<HashSet<Vector2>> uvIslands = new List<HashSet<Vector2>>();
    
            void IslandHelper(Vector2 uv0, Vector2 uv1)
            {
                int i0 = uvIslands.FindIndex(i=>i.Contains(uv0));
                int i1 = uvIslands.FindIndex(i=>i.Contains(uv1));
    
                if(i0 < 0)
                {
                    i0 = uvIslands.Count;
                    uvIslands.Add(new HashSet<Vector2>());
                    uvIslands[i0].Add(uv0);
                }
                if(i1 < 0)
                {
                    i1 = uvIslands.Count;
                    uvIslands.Add(new HashSet<Vector2>());
                    uvIslands[i1].Add(uv1);
                }
    
                if(i0 != i1)
                {
                    uvIslands[i0].UnionWith(uvIslands[i1]);
                    uvIslands.RemoveAt(i1);
                }
            }
    
            for(int i=0; i<tris.Length; i+=3)
            {
                Vector2 uv0 = uvs[tris[i+0]];
                Vector2 uv1 = uvs[tris[i+1]];
                Vector2 uv2 = uvs[tris[i+2]];
    
                IslandHelper(uv0, uv1);
                IslandHelper(uv0, uv2);
                IslandHelper(uv1, uv2);
            }
    
            int[] res = new int[tris.Length/3];
            for(int i=0; i<res.Length; i++)
            {
                res[i] = uvIslands.FindIndex(j=>j.Contains(uvs[tris[3*i+0]]));
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
    }
    
}