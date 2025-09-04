#if UNITY_EDITOR

using System;
using System.Linq;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.Rendering;
using UnityEditor;
using System.Threading.Tasks;

public class TetrahedralizerEditorWindow : EditorWindow
{
    private enum Page
    {
        OVERVIEW,
        TETRAHEDRALIZED_MESH,
        TETRAHEDRAL_MESH,
    }

    public const string TETRAHEDRALIZER_LINK_DOCUMENTATION = "https://docs.google.com/document/d/1TmDsCK4SiXGq7Y7xVC6l-rAW5G5WxgNKjIlzR9Algpc/edit?usp=sharing";
    public const string TETRAHEDRALIZER_LINK_UNITY_ASSET_STORE = "https://assetstore.unity.com/packages/slug/306196";
    public const string TETRAHEDRALIZER_LINK_GITHUB = "https://github.com/hanzemeng/Tetrahedralizer";

    private List<CustomMeshPreview> m_meshPreviews;
    private const int MESH_PREVIEWS_COUNT = 3;

    private bool m_asyncTaskIsRunning = false;
    private Page m_currentPage = Page.OVERVIEW;
    private Mesh m_mesh;
    private Tetrahedralization m_tetrahedralization;
    private TetrahedralMesh m_tetrahedralMesh;
    private Mesh m_tetrahedralizationMesh;
    private Mesh m_tetrahedralMeshMesh; // genius name, I know


    [MenuItem(TetrahedralizerConstant.TETRAHEDRALIZER_WINDOW_PATH + TetrahedralizerConstant.TETRAHEDRALIZER_NAME)]
    public static void Open()
    {
        GetWindow<TetrahedralizerEditorWindow>().titleContent = new GUIContent(TetrahedralizerConstant.TETRAHEDRALIZER_NAME);
    }

    private void OnEnable()
    {
        m_meshPreviews = Enumerable.Range(0,MESH_PREVIEWS_COUNT).Select(i=>(CustomMeshPreview)null).ToList();
    }
    private void OnDisable()
    {
        foreach(CustomMeshPreview meshPreview in m_meshPreviews)
        {
            if(null != meshPreview)
            {
                meshPreview.Dispose();
            }
        }
    }

    private void OnGUI()
    {
        if(m_asyncTaskIsRunning)
        {
            EditorGUILayout.HelpBox("An async task is running.", MessageType.Warning);
        }
        m_currentPage = (Page)GUILayout.Toolbar((int)m_currentPage, Enum.GetNames(typeof(Page)).Select(i=>TetrahedralizerUtility.UpperSnakeCaseToCapitalizedWords(i)).ToArray());

        EditorGUILayout.Space();

        switch(m_currentPage)
        {
            case Page.OVERVIEW:
                DrawOverviewPage();
                break;
            case Page.TETRAHEDRALIZED_MESH:
                _ = DrawTetrahedralizedMeshPage();
                break;
            case Page.TETRAHEDRAL_MESH:
                _ = DrawTetrahedralMeshPage();
                break;
        }
    }

    private void DrawOverviewPage()
    {
        EditorGUILayout.LabelField("Overview", EditorStyles.boldLabel);
        EditorGUILayout.HelpBox(
            $"Thank you for downloading {TetrahedralizerConstant.TETRAHEDRALIZER_NAME} version {TetrahedralizerConstant.TETRAHEDRALIZER_VERSION}!\n" +
            $"This package is about converting regular meshes into tetrahedral meshes."
            , MessageType.Info);

        if(GUILayout.Button($"Open {TetrahedralizerConstant.TETRAHEDRALIZER_NAME}'s Documentation"))
        {
            Application.OpenURL(TETRAHEDRALIZER_LINK_DOCUMENTATION);
        }
        if(GUILayout.Button($"Open {TetrahedralizerConstant.TETRAHEDRALIZER_NAME} on Unity Asset Store"))
        {
            Application.OpenURL(TETRAHEDRALIZER_LINK_UNITY_ASSET_STORE);
        }
        if(GUILayout.Button($"Open {TetrahedralizerConstant.TETRAHEDRALIZER_NAME} on GitHub"))
        {
            Application.OpenURL(TETRAHEDRALIZER_LINK_GITHUB);
        }
    }

    private async Task DrawTetrahedralizedMeshPage()
    {
        EditorGUILayout.LabelField("Tetrahedralized Mesh Creation", EditorStyles.boldLabel);

        m_mesh = (Mesh)EditorGUILayout.ObjectField("Input Mesh", m_mesh, typeof(Mesh), false);
        EditorGUI.BeginChangeCheck();
        m_tetrahedralization = (Tetrahedralization)EditorGUILayout.ObjectField("Output Tetrahedralization", m_tetrahedralization, typeof(Tetrahedralization), false);
        if(EditorGUI.EndChangeCheck())
        {
            m_tetrahedralizationMesh = null == m_tetrahedralization ? null : m_tetrahedralization.ToMesh().mesh;
        }

        EditorGUILayout.Space();
        if(!m_asyncTaskIsRunning && GUILayout.Button($"Create Tetrahedralized Mesh{(null==m_tetrahedralization ? " (Dry Run)":"")}"))
        {
            TetrahedralizedMeshCreation tetrahedralizedMeshCreation = new TetrahedralizedMeshCreation();
            TetrahedralizedMeshCreation.TetrahedralizedMeshCreationInput input = new TetrahedralizedMeshCreation.TetrahedralizedMeshCreationInput();
            TetrahedralizedMeshCreation.TetrahedralizedMeshCreationOutput output = new TetrahedralizedMeshCreation.TetrahedralizedMeshCreationOutput();
            input.m_mesh = m_mesh;

            m_asyncTaskIsRunning = true;
            try
            {
                await tetrahedralizedMeshCreation.CreateAsync(input, output);
            }
            catch(Exception exception)
            {
                Debug.LogError($"Tetrahedralized Mesh Creation encountered:\n{exception}");
            }
            m_asyncTaskIsRunning = false;
            m_tetrahedralization.Assign(output.m_tetrahedralization);
            EditorUtility.SetDirty(m_tetrahedralization);

            m_tetrahedralizationMesh = m_tetrahedralization.ToMesh().mesh;
        }

        EditorGUILayout.Space();
        EditorGUILayout.LabelField("Input Mesh and Output Tetrahedralization", EditorStyles.boldLabel);
        EditorGUILayout.BeginHorizontal();
        DrawMeshPreview(m_mesh, 0);
        DrawMeshPreview(m_tetrahedralizationMesh, 1);
        EditorGUILayout.EndHorizontal();

        EditorGUILayout.BeginHorizontal();
        if(null != m_mesh)
        {
            EditorGUILayout.LabelField($"Vertices Count: {m_mesh.vertexCount}\nTriangles Count: {m_mesh.GetIndexCount() / 3}", GUILayout.Height(40));
        }
        else
        {
            EditorGUILayout.LabelField($"Vertices Count: {0}\nTriangles Count: {0}", GUILayout.Height(40));
        }
        if(null != m_tetrahedralization)
        {
            EditorGUILayout.LabelField($"Vertices Count: {m_tetrahedralization.GetVerticesCount()}\nTetrahedrons Count: {m_tetrahedralization.GetTetrahedronsCount()}", GUILayout.Height(40));
        }
        else
        {
            EditorGUILayout.LabelField($"Vertices Count: {0}\nTetrahedrons Count: {0}", GUILayout.Height(40));
        }
        EditorGUILayout.EndHorizontal();

        if(!m_asyncTaskIsRunning && GUILayout.Button($"Reset Mesh Preview"))
        {
            ResetMeshPreview();
        }
    }

    private  async Task DrawTetrahedralMeshPage()
    {
        EditorGUILayout.LabelField("Tetrahedral Mesh Creation", EditorStyles.boldLabel);

        m_mesh = (Mesh)EditorGUILayout.ObjectField("Input Mesh", m_mesh, typeof(Mesh), false);
        EditorGUI.BeginChangeCheck();
        m_tetrahedralization = (Tetrahedralization)EditorGUILayout.ObjectField("Input Tetrahedralization", m_tetrahedralization, typeof(Tetrahedralization), false);
        if(EditorGUI.EndChangeCheck())
        {
            m_tetrahedralizationMesh = null == m_tetrahedralization ? null : m_tetrahedralization.ToMesh().mesh;
        }
        EditorGUI.BeginChangeCheck();
        m_tetrahedralMesh = (TetrahedralMesh)EditorGUILayout.ObjectField("Output Tetrahedral Mesh", m_tetrahedralMesh, typeof(TetrahedralMesh), false);
        if(EditorGUI.EndChangeCheck())
        {
            m_tetrahedralMeshMesh = null == m_tetrahedralMesh ? null : m_tetrahedralMesh.ToMesh().mesh;
        }

        EditorGUILayout.Space();
        if(!m_asyncTaskIsRunning && GUILayout.Button($"Create Tetrahedral Mesh{(null==m_tetrahedralMesh ? " (Dry Run)":"")}"))
        {
            TetrahedralMeshCreation tetrahedralMeshCreation = new TetrahedralMeshCreation();
            TetrahedralMeshCreation.TetrahedralMeshCreationInput input = new TetrahedralMeshCreation.TetrahedralMeshCreationInput();
            TetrahedralMeshCreation.TetrahedralMeshCreationOutput output = new TetrahedralMeshCreation.TetrahedralMeshCreationOutput();
            input.m_mesh = m_mesh;
            input.m_tetrahedralization = m_tetrahedralization;

            m_asyncTaskIsRunning = true;
            try
            {
                await tetrahedralMeshCreation.CreateAsync(input, output);
            }
            catch(Exception exception)
            {
                Debug.LogError($"Tetrahedral Mesh Creation encountered:\n{exception}");
            }
            m_asyncTaskIsRunning = false;
            m_tetrahedralMesh.Assign(output.m_tetrahedralMesh);
            EditorUtility.SetDirty(m_tetrahedralMesh);

            m_tetrahedralMeshMesh = m_tetrahedralMesh.ToMesh().mesh;
        }

        EditorGUILayout.Space();
        EditorGUILayout.LabelField("Input Mesh and Tetrahedralization", EditorStyles.boldLabel);
        EditorGUILayout.BeginHorizontal();
        DrawMeshPreview(m_mesh, 0);
        DrawMeshPreview(m_tetrahedralizationMesh, 1);
        EditorGUILayout.EndHorizontal();

        EditorGUILayout.BeginHorizontal();
        if(null != m_mesh)
        {
            EditorGUILayout.LabelField($"Vertices Count: {m_mesh.vertexCount}\nTriangles Count: {m_mesh.GetIndexCount() / 3}", GUILayout.Height(40));
        }
        else
        {
            EditorGUILayout.LabelField($"Vertices Count: {0}\nTriangles Count: {0}", GUILayout.Height(40));
        }
        if(null != m_tetrahedralization)
        {
            EditorGUILayout.LabelField($"Vertices Count: {m_tetrahedralization.GetVerticesCount()}\nTetrahedrons Count: {m_tetrahedralization.GetTetrahedronsCount()}", GUILayout.Height(40));
        }
        else
        {
            EditorGUILayout.LabelField($"Vertices Count: {0}\nTetrahedrons Count: {0}", GUILayout.Height(40));
        }
        EditorGUILayout.EndHorizontal();

        EditorGUILayout.LabelField("Output Tetrahedral Mesh", EditorStyles.boldLabel);
        DrawMeshPreview(m_tetrahedralMeshMesh, 2);
        if(null != m_tetrahedralMesh)
        {
            EditorGUILayout.LabelField($"Tetrahedrons Count: {m_tetrahedralMesh.GetTetrahedronsCount()}");
        }
        else
        {
            EditorGUILayout.LabelField($"Tetrahedrons Count: {0}");
        }

        if(!m_asyncTaskIsRunning && GUILayout.Button($"Reset Mesh Preview"))
        {
            ResetMeshPreview();
        }
    }

    private void DrawMeshPreview(Mesh mesh, int previewIndex)
    {
        if(null == mesh)
        {
            mesh = new Mesh(); // just draw something
        }
        if(null == m_meshPreviews[previewIndex])
        {
            m_meshPreviews[previewIndex] = new CustomMeshPreview(mesh);
        }
        if(m_meshPreviews[previewIndex].mesh != mesh)
        {
            m_meshPreviews[previewIndex].Dispose();
            m_meshPreviews[previewIndex] = new CustomMeshPreview(mesh);
        }

        m_meshPreviews[previewIndex].OnPreviewGUI(GUILayoutUtility.GetRect(200, 200, GUILayout.ExpandWidth(true)), GUIStyle.none);
    }

    private void ResetMeshPreview()
    {
        for(int i=0; i<m_meshPreviews.Count; i++)
        {
            if(null == m_meshPreviews[i])
            {
                continue;
            }
            Mesh oldMesh = m_meshPreviews[i].mesh;
            m_meshPreviews[i].Dispose();
            m_meshPreviews[i] = null;
            DrawMeshPreview(oldMesh, i);
        }
    }
}

#endif
