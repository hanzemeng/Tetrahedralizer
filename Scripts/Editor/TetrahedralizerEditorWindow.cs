#if UNITY_EDITOR

using UnityEngine;
using UnityEditor;

public class TetrahedralizerEditorWindow : EditorWindow
{
    private Tetrahedralizer m_tetrahedralizer;

    private Mesh m_mesh;
    private TetrahedralizedMesh m_tetrahedralizedMesh0;
    private TetrahedralizedMesh m_tetrahedralizedMesh1;
    private TetrahedralMesh m_tetrahedralMesh;
    private bool m_remapVertxData;
    private float m_degenerateTetrahedronRatio;


    [MenuItem("Window/Tetrahedralizer")]
    public static void Open()
    {
        TetrahedralizerEditorWindow wnd = GetWindow<TetrahedralizerEditorWindow>();
        wnd.titleContent = new GUIContent("Tetrahedralizer");
    }


    private void OnEnable()
    {
        m_tetrahedralizer = new Tetrahedralizer();

        m_remapVertxData = false;
        m_degenerateTetrahedronRatio = 0.05f;
        Tetrahedralizer.Settings settings = new Tetrahedralizer.Settings(m_remapVertxData, m_degenerateTetrahedronRatio);
        m_tetrahedralizer.SetSettings(settings);
    }

    
    private void OnGUI()
    {
        var style = new GUIStyle(GUI.skin.label) {alignment = TextAnchor.MiddleCenter};

        GUILayout.Space(20);

        m_mesh = (Mesh)EditorGUILayout.ObjectField("Mesh:", m_mesh, typeof(Mesh), true);
        m_tetrahedralizedMesh0 = (TetrahedralizedMesh)EditorGUILayout.ObjectField("Tetrahedralized Mesh:", m_tetrahedralizedMesh0, typeof(TetrahedralizedMesh), true);
        if(GUILayout.Button("Mesh to Tetrahedralized Mesh"))
        {
            OnB0Clicked();
        }

        GUILayout.Space(20);
        EditorGUILayout.LabelField("-----------------------------------", style, GUILayout.ExpandWidth(true));
        GUILayout.Space(20);

        m_tetrahedralizedMesh1 = (TetrahedralizedMesh)EditorGUILayout.ObjectField("Tetrahedralized Mesh:", m_tetrahedralizedMesh1, typeof(TetrahedralizedMesh), true);
        m_tetrahedralMesh = (TetrahedralMesh)EditorGUILayout.ObjectField("Tetrahedral Mesh:", m_tetrahedralMesh, typeof(TetrahedralMesh), true);
        m_remapVertxData = EditorGUILayout.Toggle("Remap Vertex Data:",m_remapVertxData);
        m_degenerateTetrahedronRatio = EditorGUILayout.FloatField("Degenerate Tetrahedron Ratio:", m_degenerateTetrahedronRatio);

        if(GUILayout.Button("Tetrahedralized Mesh to Tetrahedral Mesh"))
        {
            OnB1Clicked();
        }
    }

    private void OnB0Clicked()
    {
        m_tetrahedralizer.MeshToTetrahedralizedMesh(m_mesh, m_tetrahedralizedMesh0);

        EditorUtility.SetDirty(m_tetrahedralizedMesh0);
        AssetDatabase.SaveAssets();

        Debug.Log("Mesh to Tetrahedralized Mesh Completed");
    }

    private void OnB1Clicked()
    {
        Tetrahedralizer.Settings settings = new Tetrahedralizer.Settings(m_remapVertxData, m_degenerateTetrahedronRatio);

        m_tetrahedralizer.SetSettings(settings);
        m_tetrahedralizer.TetrahedralizedMeshToTetrahedralMesh(m_tetrahedralizedMesh1,m_tetrahedralMesh);

        EditorUtility.SetDirty(m_tetrahedralMesh);
        AssetDatabase.SaveAssets();

        Debug.Log("Tetrahedralized Mesh to Tetrahedral Mesh Completed");
    }
}

#endif