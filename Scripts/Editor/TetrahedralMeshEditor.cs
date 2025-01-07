#if UNITY_EDITOR
namespace Hanzzz.Tetrahedralizer
{

using System;
using UnityEngine.Rendering;
using UnityEditor;

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

}
#endif
