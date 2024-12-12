#if UNITY_EDITOR

using UnityEngine;
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

        EditorGUILayout.LabelField($"Vertices Count: {tetrahedralMesh.vertices.Count}");
        EditorGUILayout.LabelField($"Tetrahedrons Count: {tetrahedralMesh.tetrahedrons.Count / 4}");
        EditorGUILayout.ObjectField("Generated Mesh", tetrahedralMesh.mesh, typeof(Mesh), false);
    }
}

#endif
