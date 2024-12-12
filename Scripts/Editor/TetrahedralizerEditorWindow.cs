#if UNITY_EDITOR

using UnityEngine;
using UnityEngine.UIElements;
using UnityEditor;
using UnityEditor.UIElements;

public class TetrahedralizerEditorWindow : EditorWindow
{
    private Tetrahedralizer tetrahedralizer;

    private ObjectField meshObjectField;
    private ObjectField tetrahedralizedMeshObjectField;
    private ObjectField tetrahedralMeshObjectField;

    [MenuItem("Window/Tetrahedralizer")]
    public static void Open()
    {
        TetrahedralizerEditorWindow wnd = GetWindow<TetrahedralizerEditorWindow>();
        wnd.titleContent = new GUIContent("Tetrahedralizer");
    }

    public void CreateGUI()
    {
        tetrahedralizer = new Tetrahedralizer();

        meshObjectField = new ObjectField();
        meshObjectField.allowSceneObjects = true;
        meshObjectField.objectType = typeof(Mesh);

        tetrahedralizedMeshObjectField = new ObjectField();
        tetrahedralizedMeshObjectField.allowSceneObjects = false;
        tetrahedralizedMeshObjectField.objectType = typeof(TetrahedralizedMesh);

        tetrahedralMeshObjectField = new ObjectField();
        tetrahedralMeshObjectField.allowSceneObjects = false;
        tetrahedralMeshObjectField.objectType = typeof(TetrahedralMesh);

        Button b0 = new Button();
        b0.name = "Mesh to Tetrahedralized Mesh";
        b0.text = "Mesh to Tetrahedralized Mesh";
        b0.clicked += OnB0Clicked;

        Button b1 = new Button();
        b1.name = "Tetrahedralized Mesh to Tetrahedral Mesh (position only)";
        b1.text = "Tetrahedralized Mesh to Tetrahedral Mesh (position only)";
        b1.clicked += OnB1Clicked;

        Button b2 = new Button();
        b2.name = "Tetrahedralized Mesh to Tetrahedral Mesh";
        b2.text = "Tetrahedralized Mesh to Tetrahedral Mesh";
        b2.clicked += OnB2Clicked;

        VisualElement root = rootVisualElement;
        root.Add(meshObjectField);
        root.Add(tetrahedralizedMeshObjectField);
        root.Add(tetrahedralMeshObjectField);
        root.Add(b0);
        root.Add(b1);
        root.Add(b2);
    }



    private void OnB0Clicked()
    {
        Mesh mesh = (Mesh)meshObjectField.value;
        TetrahedralizedMesh tetrahedralizedMesh = (TetrahedralizedMesh)tetrahedralizedMeshObjectField.value;

        tetrahedralizer.MeshToTetrahedralizedMesh(mesh, tetrahedralizedMesh);

        EditorUtility.SetDirty(tetrahedralizedMesh);
        AssetDatabase.SaveAssets();

        Debug.Log("Mesh to Tetrahedralized Mesh Completed");
    }

    private void OnB1Clicked()
    {
        TetrahedralizedMesh tetrahedralizedMesh = (TetrahedralizedMesh)tetrahedralizedMeshObjectField.value;
        TetrahedralMesh tetrahedralMesh = (TetrahedralMesh)tetrahedralMeshObjectField.value;

        tetrahedralizer.TetrahedralizedMeshToTetrahedralMesh(tetrahedralizedMesh, tetrahedralMesh, false);

        EditorUtility.SetDirty(tetrahedralMesh);
        AssetDatabase.SaveAssets();

        Debug.Log("Tetrahedralized Mesh to Tetrahedral Mesh (position only) Completed");
    }

    private void OnB2Clicked()
    {
        TetrahedralizedMesh tetrahedralizedMesh = (TetrahedralizedMesh)tetrahedralizedMeshObjectField.value;
        TetrahedralMesh tetrahedralMesh = (TetrahedralMesh)tetrahedralMeshObjectField.value;

        tetrahedralizer.TetrahedralizedMeshToTetrahedralMesh(tetrahedralizedMesh, tetrahedralMesh, true);

        EditorUtility.SetDirty(tetrahedralMesh);
        AssetDatabase.SaveAssets();

        Debug.Log("Tetrahedralized Mesh to Tetrahedral Mesh Completed");
    }
}

#endif