using System;
using System.Linq;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.Rendering;
#if UNITY_EDITOR
using UnityEditor;
#endif

namespace Hanzzz.Tetrahedralizer
{
    [CreateAssetMenu(fileName = nameof(TetrahedralMesh), menuName = TetrahedralizerConstant.SCRIPTABLE_OBJECT_PATH + nameof(TetrahedralMesh))]
    [PreferBinarySerialization]
    public class TetrahedralMesh : ScriptableObject
    {
        public List<Vector3> vertices; // every 3 elements correspond to a triangle, and every 4 triangles correspond to a tetrahedron
        public List<Int32> vertexAttributeDescriptors; // note that normals and tangents are calculated, but not stored
        public List<Color32> colors;
        public List<Vector4> uvs0; // uvs are usually vector2, but they can be vector3 or vector4
        public List<Vector4> uvs1;
        public List<Vector4> uvs2;
        public List<Vector4> uvs3;
        public List<Vector4> uvs4;
        public List<Vector4> uvs5;
        public List<Vector4> uvs6;
        public List<Vector4> uvs7;
    
        public List<Int32> facetsSubmeshes; // every 4 elements correspond to the 4 facets of a tetrahedron
        public List<Int32> neighbors; // the ith element is the ith facet's neighboring facet, UNDEFINED_VALUE if no neighbor
        // neighbors order: 0,1,2  0,1,3  0,3,2,  1,2,3
    
    
        // each element is a tetrahedron, and the center of the tetrahedron in world space
        public List<(Mesh mesh, Vector3 center)> ToMeshes()
        {
            if(null == vertices || 0 == vertices.Count)
            {
                return null;
            }
    
            List<(Mesh, Vector3)> res = new List<(Mesh, Vector3)>();
            List<Int32> tempInt = new List<Int32>();
            List<Vector3> tempVector3s = new List<Vector3>();
            MeshVertexDataMapper mvdm = new MeshVertexDataMapper();
            mvdm.AssignSourceTetrahedralMesh(this);
            Int32 submeshCount = GetSubmeshesCount();
    
            for(Int32 i=0; i<facetsSubmeshes.Count; i+=4)
            {
                for(Int32 j=0; j<12; j++)
                {
                    mvdm.CopyVertexData(3*i+j);
                }
                Mesh mesh = mvdm.MakeMesh();
                mesh.subMeshCount = submeshCount;
                mvdm.ClearTarget();
    
                for(Int32 j=0; j<submeshCount; j++)
                {
                    tempInt.Clear();
                    for(Int32 k=0; k<4; k++)
                    {
                        if(j == facetsSubmeshes[i+k])
                        {
                            tempInt.Add(3*k+0);
                            tempInt.Add(3*k+1);
                            tempInt.Add(3*k+2);
                        }
                    }
                    mesh.SetTriangles(tempInt,j);
                }
    
                mesh.GetVertices(tempVector3s);
                Vector3 center = TetrahedralizerUtility.CenterVertices(tempVector3s);
                mesh.SetVertices(tempVector3s);
                mesh.RecalculateBounds();
                mesh.RecalculateNormals();
                mesh.RecalculateTangents();
    
                res.Add((mesh, center));
            }
    
            return res;
        }
        public (Mesh mesh, Vector3 center) ToMesh()
        {
            if(null == vertices || 0 == vertices.Count)
            {
                return (null, Vector3.zero);
            }
    
            bool[] exteriorFlags = GetFacetsExteriorFlags();
            int[] mappings = Enumerable.Repeat(TetrahedralizerConstant.UNDEFINED_VALUE, facetsSubmeshes.Count).ToArray();
            
            MeshVertexDataMapper mvdm = new MeshVertexDataMapper();
            mvdm.AssignSourceTetrahedralMesh(this);
            for(Int32 i=0; i<facetsSubmeshes.Count; i++)
            {
                if(!exteriorFlags[i])
                {
                    continue;
                }
                mappings[i] = mvdm.GetTargetVertexCount()/3;
                mvdm.CopyVertexData(3*i+0);
                mvdm.CopyVertexData(3*i+1);
                mvdm.CopyVertexData(3*i+2);
            }
    
            Int32 submeshCount = GetSubmeshesCount();
            Mesh mesh = mvdm.MakeMesh();
            mesh.subMeshCount = submeshCount;
            List<Int32> tempInt = new List<Int32>();
            for(Int32 i=0; i<submeshCount; i++)
            {
                tempInt.Clear();
                for(Int32 j=0; j<facetsSubmeshes.Count; j++)
                {
                    if(exteriorFlags[j] && i==facetsSubmeshes[j])
                    {
                        tempInt.Add(3*mappings[j]+0);
                        tempInt.Add(3*mappings[j]+1);
                        tempInt.Add(3*mappings[j]+2);
                    }
                }
                mesh.SetTriangles(tempInt,i);
            }
    
            List<Vector3> tempVector3s = new List<Vector3>();
            mesh.GetVertices(tempVector3s);
            Vector3 center = TetrahedralizerUtility.CenterVertices(tempVector3s);
            mesh.SetVertices(tempVector3s);
            mesh.RecalculateBounds();
            mesh.RecalculateNormals();
            mesh.RecalculateTangents();
    
            return (mesh, center);
        }
    
        public bool[] GetFacetsExteriorFlags()
        {
            return neighbors.Select(i=>TetrahedralizerConstant.UNDEFINED_VALUE==i).ToArray();
        }
    
        public TetrahedralMesh()
        {
            vertices = new List<Vector3>();
    
            vertexAttributeDescriptors = new List<Int32>();
    
            colors = new List<Color32>();
            uvs0 = new List<Vector4>();
            uvs1 = new List<Vector4>();
            uvs2 = new List<Vector4>();
            uvs3 = new List<Vector4>();
            uvs4 = new List<Vector4>();
            uvs5 = new List<Vector4>();
            uvs6 = new List<Vector4>();
            uvs7 = new List<Vector4>();
    
            facetsSubmeshes = new List<Int32>();
            neighbors = new List<Int32>();
        }
        public void Assign(TetrahedralMesh tetrahedralMesh)
        {
            vertices = tetrahedralMesh.vertices;
            vertexAttributeDescriptors = tetrahedralMesh.vertexAttributeDescriptors;
            colors = tetrahedralMesh.colors;
            uvs0 = tetrahedralMesh.uvs0;
            uvs1 = tetrahedralMesh.uvs1;
            uvs2 = tetrahedralMesh.uvs2;
            uvs3 = tetrahedralMesh.uvs3;
            uvs4 = tetrahedralMesh.uvs4;
            uvs5 = tetrahedralMesh.uvs5;
            uvs6 = tetrahedralMesh.uvs6;
            uvs7 = tetrahedralMesh.uvs7;
    
            facetsSubmeshes = tetrahedralMesh.facetsSubmeshes;
            neighbors = tetrahedralMesh.neighbors;
        }
    
        public void Clear()
        {
            vertices.Clear();
            vertexAttributeDescriptors.Clear();
            colors.Clear();
            uvs0.Clear();
            uvs1.Clear();
            uvs2.Clear();
            uvs3.Clear();
            uvs4.Clear();
            uvs5.Clear();
            uvs6.Clear();
            uvs7.Clear();
            facetsSubmeshes.Clear();
            neighbors.Clear();
        }
    
        public int GetTetrahedronsCount()
        {
            return vertices.Count / 12;
        }
    
        public void SetVertexAttributeDescriptors(VertexAttributeDescriptor[] source)
        {
            vertexAttributeDescriptors.Clear();
            for(Int32 i=0; i<source.Length; i++)
            {
                vertexAttributeDescriptors.Add(VertexAttributeDescriptorSerializer.ToInt32(source[i]));
            }
        }
        public VertexAttributeDescriptor[] GetVertexAttributeDescriptors()
        {
            return vertexAttributeDescriptors.Select(i=>VertexAttributeDescriptorSerializer.ToVertexAttributeDescriptor(i)).ToArray();
        }
        public bool HasVertexAttribute(VertexAttribute val)
        {
            foreach(Int32 i in vertexAttributeDescriptors)
            {
                VertexAttributeDescriptor v = VertexAttributeDescriptorSerializer.ToVertexAttributeDescriptor(i);
                if(v.attribute == val)
                {
                    return true;
                }
            }
            return false;
        }
    
        public void SetVertices(IList<Vector3> source)
        {
            vertices.ReplaceWith(source);
        }
        public void GetVertices(IList<Vector3> target)
        {
            target.ReplaceWith(vertices);
        }
        public void SetColors(IList<Color32> source)
        {
            colors.ReplaceWith(source);
        }
        public void GetColors(IList<Color32> target)
        {
            target.ReplaceWith(colors);
        }
        public void SetUVs(Int32 c, IList<Vector4> source)
        {
            switch(c)
            {
                case 0:
                    uvs0.ReplaceWith(source);
                break;
                case 1:
                    uvs1.ReplaceWith(source);
                break;
                case 2:
                    uvs2.ReplaceWith(source);
                break;
                case 3:
                    uvs3.ReplaceWith(source);
                break;
                case 4:
                    uvs4.ReplaceWith(source);
                break;
                case 5:
                    uvs5.ReplaceWith(source);
                break;
                case 6:
                    uvs6.ReplaceWith(source);
                break;
                case 7:
                    uvs7.ReplaceWith(source);
                break;
                default:
                    throw new Exception($"UV channel {c} does not exist.");
            }
        }
        public void GetUVs(Int32 c, IList<Vector4> target)
        {
            switch(c)
            {
                case 0:
                    target.ReplaceWith(uvs0);
                break;
                case 1:
                    target.ReplaceWith(uvs1);
                break;
                case 2:
                    target.ReplaceWith(uvs2);
                break;
                case 3:
                    target.ReplaceWith(uvs3);
                break;
                case 4:
                    target.ReplaceWith(uvs4);
                break;
                case 5:
                    target.ReplaceWith(uvs5);
                break;
                case 6:
                    target.ReplaceWith(uvs6);
                break;
                case 7:
                    target.ReplaceWith(uvs7);
                break;
                default:
                    throw new Exception($"UV channel {c} does not exist.");
            }
        }
        public void SetFacetsSubmeshes(IList<Int32> source)
        {
            facetsSubmeshes.ReplaceWith(source);
        }
        public void GetFacetsSubmeshes(IList<Int32> target)
        {
            target.ReplaceWith(facetsSubmeshes);
        }
        public void SetNeighbors(IList<Int32> source)
        {
            neighbors.ReplaceWith(source);
        }
        public void GetNeighbors(IList<Int32> target)
        {
            target.ReplaceWith(neighbors);
        }
        public Int32 GetSubmeshesCount()
        {
            return facetsSubmeshes.Max()+1;
        }
    }
    
    
    #if UNITY_EDITOR
    [CustomEditor(typeof(TetrahedralMesh))]
    public class TetrahedralMeshEditor : Editor
    {
        TetrahedralMesh tetrahedralMesh;
        private void OnEnable()
        {
            tetrahedralMesh = (TetrahedralMesh)target;
        }
    
        public override void OnInspectorGUI()
        {
            //base.OnInspectorGUI();
    
            EditorGUILayout.LabelField($"Tetrahedrons Count: {tetrahedralMesh.GetTetrahedronsCount()}");
            EditorGUILayout.Space();
            EditorGUILayout.LabelField("Vertex Data:");
            foreach(Int32 i in tetrahedralMesh.vertexAttributeDescriptors)
            {
                VertexAttributeDescriptor v = VertexAttributeDescriptorSerializer.ToVertexAttributeDescriptor(i);
                EditorGUILayout.LabelField(v.ToString());
            }
        }
    }
    #endif
    
}