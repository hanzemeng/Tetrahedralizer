#if UNITY_EDITOR
namespace Hanzzz.Tetrahedralizer
{

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
        EditorGUILayout.ObjectField("Generated Mesh: ", tetrahedralMesh.mesh, typeof(Mesh), false);

        EditorGUILayout.Space();
        EditorGUILayout.LabelField($"WARNING: Do not modify this scriptable object in the inspector.");
        EditorGUILayout.LabelField($"Please use the tetrahedralizer window in");
        EditorGUILayout.LabelField($"{TetrahedralizerEditorWindow.MENU_PATH} to modify.");
    }
}

}
#endif
