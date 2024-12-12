#if UNITY_EDITOR

using UnityEngine;
using UnityEngine.UIElements;
using UnityEditor;
using UnityEditor.UIElements;

[CustomEditor(typeof(TetrahedralizedMesh))]
public class TetrahedralizedMeshEditor : Editor
{
    TetrahedralizedMesh tetrahedralizedMesh;
    private void OnEnable()
    {
        tetrahedralizedMesh = (TetrahedralizedMesh)target;
    }

    public override void OnInspectorGUI()
    {
        //base.OnInspectorGUI();

        EditorGUILayout.ObjectField("Original Mesh", tetrahedralizedMesh.originalMesh, typeof(Mesh), false);
        EditorGUILayout.LabelField($"Original Vertices Count: {tetrahedralizedMesh.originalVerticesMappings.Count}");
        EditorGUILayout.LabelField($"New Vertices Count: {tetrahedralizedMesh.newVerticesMappings.Count}");
        EditorGUILayout.LabelField($"Tetrahedrons Count: {tetrahedralizedMesh.tetrahedrons.Count / 4}");
    }
}

#endif
