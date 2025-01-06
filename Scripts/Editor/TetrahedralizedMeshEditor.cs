#if UNITY_EDITOR
namespace Hanzzz.Tetrahedralizer
{

using UnityEngine;
using UnityEditor;

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

        EditorGUILayout.ObjectField("Original Mesh: ",tetrahedralizedMesh.originalMesh,typeof(Mesh),false);
        EditorGUILayout.LabelField($"Original Vertices Count: {tetrahedralizedMesh.originalVerticesMappings.Count}");
        EditorGUILayout.LabelField($"New Vertices Count: {tetrahedralizedMesh.newVerticesMappings.Count}");
        EditorGUILayout.LabelField($"Tetrahedrons Count: {tetrahedralizedMesh.tetrahedrons.Count / 4}");

        EditorGUILayout.Space();
        EditorGUILayout.LabelField($"WARNING: Do not modify this scriptable object in the inspector.");
        EditorGUILayout.LabelField($"Please use the tetrahedralizer window in");
        EditorGUILayout.LabelField($"{TetrahedralizerEditorWindow.MENU_PATH} to modify.");
    }
}

}
#endif
