using System;
using System.Linq;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.Rendering;
#if UNITY_EDITOR
using UnityEditor;
#endif

[CreateAssetMenu(fileName = SHORT_NAME, menuName = TetrahedralizerLibraryConstant.SCRIPTABLE_OBJECT_PATH + SHORT_NAME)]
[PreferBinarySerialization]
public class TetrahedralMesh : ScriptableObject
{
    public const string SHORT_NAME = "TetrahedralMesh_SO";

    public List<Int32> tetrahedrons;
    public List<Vector3> vertices;

    public List<Int32> vertexAttributeDescriptors;

    public List<Color32> colors;
    public List<Vector4> uvs0; // uvs are usually vector2, but they can be vector3 or vector4
    public List<Vector4> uvs1;
    public List<Vector4> uvs2;
    public List<Vector4> uvs3;
    public List<Vector4> uvs4;
    public List<Vector4> uvs5;
    public List<Vector4> uvs6;
    public List<Vector4> uvs7;

    public List<Int32> facetsSubmeshes;


    public TetrahedralMesh()
    {
        tetrahedrons = new List<Int32>();
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
    }

    public void Clear()
    {
        tetrahedrons.Clear();
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

        if(0 != tetrahedralMesh.tetrahedrons.Count)
        {
            EditorGUILayout.LabelField($"Tetrahedrons Count: {tetrahedralMesh.tetrahedrons.Count / 4}");
            EditorGUILayout.LabelField($"Vertices Count: {tetrahedralMesh.vertices.Count}");
        }
        else
        {
            EditorGUILayout.LabelField($"Tetrahedrons Count: {tetrahedralMesh.facetsSubmeshes.Count / 4}");
            EditorGUILayout.LabelField($"Vertices Count: {tetrahedralMesh.vertices.Count}");
            EditorGUILayout.Space();
            EditorGUILayout.LabelField("Vertex Data:");
            foreach(Int32 i in tetrahedralMesh.vertexAttributeDescriptors)
            {
                VertexAttributeDescriptor v = VertexAttributeDescriptorSerializer.ToVertexAttributeDescriptor(i);
                EditorGUILayout.LabelField(v.ToString());
            }
        }
    }
}
#endif
