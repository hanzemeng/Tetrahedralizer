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


    private GUIStyle m_centerBoldLabelStyleField;
    private GUIStyle m_centerBoldLabelStyle
    {
        get
        {
            if(null == m_centerBoldLabelStyleField)
            {
                m_centerBoldLabelStyleField = new GUIStyle(EditorStyles.boldLabel)
                {
                    alignment = TextAnchor.MiddleCenter
                };
            }
            return m_centerBoldLabelStyleField;
        }
    }

    private float MESH_PREVIEW_RECT_SIZE = 200f;
    private float MESH_DESCRIPTION_RECT_SIZE = 50f;
    private float GAP_SMALL_RECT_SIZE = 5f;

    private bool m_asyncTaskIsRunning;
    private Page m_currentPage;
    private List<MeshPreviewField> m_meshesPreviews;
    private const int MESH_PREVIEWS_COUNT = 3;
    private bool m_synchronizeMeshesPreviews;

    private GameObject m_gameObject;
    private Mesh m_mesh;
    private List<Material> m_meshMaterials;
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
        m_asyncTaskIsRunning = false;
        m_currentPage = Page.OVERVIEW;
        m_meshesPreviews = Enumerable.Range(0, MESH_PREVIEWS_COUNT).Select(i=>new MeshPreviewField()).ToList();
        m_synchronizeMeshesPreviews = true;
        m_meshMaterials = new List<Material>();
    }
    private void OnDisable()
    {
        m_meshesPreviews.ForEach(i=>i.Dispose());
    }

    private void OnGUI()
    {
        if(m_asyncTaskIsRunning)
        {
            EditorGUILayout.HelpBox("An async task is running.", MessageType.Warning);
        }
        m_currentPage = (Page)GUILayout.Toolbar((int)m_currentPage, Enum.GetNames(typeof(Page)).Select(i=>TetrahedralizerUtility.UpperSnakeCaseToCapitalizedWords(i)).ToArray());

        EditorGUILayout.Space();

        if(m_currentPage == Page.OVERVIEW)
        {
            DrawOverviewPage();
        }
        else if(m_currentPage == Page.TETRAHEDRALIZED_MESH)
        {
            if(DrawTargetGameObjectOnGUI())
            {
                _ = DrawTetrahedralizedMeshPage();
                DrawPreviewSettingsOnGUI();
            }
        }
        else if(m_currentPage == Page.TETRAHEDRAL_MESH)
        {
            if(DrawTargetGameObjectOnGUI())
            {
                _ = DrawTetrahedralMeshPage();
                DrawPreviewSettingsOnGUI();
            }
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

    private bool DrawTargetGameObjectOnGUI()
    {
        m_gameObject = (GameObject)EditorGUILayout.ObjectField("Target Game Object:", m_gameObject, typeof(GameObject), true);

        if(null == m_gameObject)
        {
            EditorGUILayout.HelpBox("The target game object must be assigned.", MessageType.Error);
            return false;
        }
        MeshFilter meshFilter;
        if(!m_gameObject.TryGetComponent<MeshFilter>(out meshFilter) || null == meshFilter.sharedMesh)
        {
            EditorGUILayout.HelpBox("The target game object has no Mesh Filter with a valid mesh.", MessageType.Error);
            return false;
        }
        MeshRenderer meshRenderer;
        if(!m_gameObject.TryGetComponent<MeshRenderer>(out meshRenderer))
        {
            EditorGUILayout.HelpBox("The target game object has no Mesh Renderer.", MessageType.Error);
            return false;
        }
        meshRenderer.GetSharedMaterials(m_meshMaterials);
        if(0 == m_meshMaterials.Count || m_meshMaterials.Where(i=>null==i).Any())
        {
            EditorGUILayout.HelpBox("The target game object's Mesh Renderer has no materials or invalid material(s).", MessageType.Error);
            return false;
        }

        m_mesh = meshFilter.sharedMesh;

        EditorGUI.BeginDisabledGroup(true);
        EditorGUILayout.ObjectField("Input Mesh: ", m_mesh, typeof(Mesh), false);
        for(int i=0; i<m_meshMaterials.Count; i++)
        {
            EditorGUILayout.ObjectField($"Input Material {i}: ", m_meshMaterials[i], typeof(Material), false);
        }
        EditorGUI.EndDisabledGroup();

        return true;
    }

    private void DrawPreviewSettingsOnGUI()
    {
        if(!m_asyncTaskIsRunning && GUILayout.Button($"Reset Meshes Previews"))
        {
            ResetMeshesPreviews();
        }

        if((m_synchronizeMeshesPreviews && GUILayout.Button($"Synchronize Meshes Previews")) || 
           (!m_synchronizeMeshesPreviews && GUILayout.Button($"Unsynchronize Meshes Previews")))
        {
            m_synchronizeMeshesPreviews = !m_synchronizeMeshesPreviews;
        }
    }

    private async Task DrawTetrahedralizedMeshPage()
    {
        EditorGUI.BeginChangeCheck();
        m_tetrahedralization = (Tetrahedralization)EditorGUILayout.ObjectField("Output Tetrahedralization", m_tetrahedralization, typeof(Tetrahedralization), false);
        if(EditorGUI.EndChangeCheck())
        {
            UpdateMesh(ref m_tetrahedralizationMesh, null == m_tetrahedralization ? null : m_tetrahedralization.ToMesh().mesh);
        }

        EditorGUILayout.Space();
        if(!m_asyncTaskIsRunning && null!=m_tetrahedralization && GUILayout.Button($"Create Tetrahedralized Mesh"))
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

            UpdateMesh(ref m_tetrahedralizationMesh, null == m_tetrahedralization ? null : m_tetrahedralization.ToMesh().mesh);
        }

        GUILayout.Space(GAP_SMALL_RECT_SIZE);
        EditorGUILayout.LabelField("Input and Output Previews", m_centerBoldLabelStyle);

        EditorGUILayout.BeginHorizontal();
        GUILayout.Space(GAP_SMALL_RECT_SIZE);
        DrawMeshPreview(m_mesh, 0, GUILayoutUtility.GetRect(MESH_PREVIEW_RECT_SIZE, MESH_PREVIEW_RECT_SIZE, GUILayout.ExpandWidth(false)));
        EditorGUI.DrawRect(GUILayoutUtility.GetRect(GAP_SMALL_RECT_SIZE, MESH_PREVIEW_RECT_SIZE, GUILayout.ExpandWidth(false)), Color.yellow);
        DrawMeshPreview(m_tetrahedralizationMesh, 1, GUILayoutUtility.GetRect(MESH_PREVIEW_RECT_SIZE, MESH_PREVIEW_RECT_SIZE, GUILayout.ExpandWidth(false)));
        EditorGUILayout.EndHorizontal();

        EditorGUILayout.BeginHorizontal();
        GUILayout.Space(GAP_SMALL_RECT_SIZE);
        EditorGUI.LabelField(GUILayoutUtility.GetRect(MESH_PREVIEW_RECT_SIZE, MESH_DESCRIPTION_RECT_SIZE, GUILayout.ExpandWidth(false)), 
        $"Input Mesh\nVertices Count: {(null==m_mesh ? 0:m_mesh.vertexCount)}\nTriangles Count: {(null==m_mesh ? 0:m_mesh.GetIndexCount()/3)}");
        GUILayout.Space(GAP_SMALL_RECT_SIZE);
        EditorGUI.LabelField(GUILayoutUtility.GetRect(MESH_PREVIEW_RECT_SIZE, MESH_DESCRIPTION_RECT_SIZE, GUILayout.ExpandWidth(false)), 
        $"Output Tetrahedralization\nVertices Count: {(null==m_tetrahedralization ? 0:m_tetrahedralization.GetVerticesCount())}\nTetrahedrons Count: {(null==m_tetrahedralization ? 0:m_tetrahedralization.GetTetrahedronsCount())}");
        EditorGUILayout.EndHorizontal();
    }

    private async Task DrawTetrahedralMeshPage()
    {
        EditorGUI.BeginChangeCheck();
        m_tetrahedralization = (Tetrahedralization)EditorGUILayout.ObjectField("Input Tetrahedralization", m_tetrahedralization, typeof(Tetrahedralization), false);
        if(EditorGUI.EndChangeCheck())
        {
            UpdateMesh(ref m_tetrahedralizationMesh, null == m_tetrahedralization ? null : m_tetrahedralization.ToMesh().mesh);
        }
        EditorGUI.BeginChangeCheck();
        m_tetrahedralMesh = (TetrahedralMesh)EditorGUILayout.ObjectField("Output Tetrahedral Mesh", m_tetrahedralMesh, typeof(TetrahedralMesh), false);
        if(EditorGUI.EndChangeCheck())
        {
            UpdateMesh(ref m_tetrahedralMeshMesh, null == m_tetrahedralMesh ? null : m_tetrahedralMesh.ToMesh().mesh);
        }

        EditorGUILayout.Space();
        if(!m_asyncTaskIsRunning && null!=m_tetrahedralization && null!=m_tetrahedralMesh && GUILayout.Button($"Create Tetrahedral Mesh"))
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

            UpdateMesh(ref m_tetrahedralMeshMesh, null == m_tetrahedralMesh ? null : m_tetrahedralMesh.ToMesh().mesh);
        }


        GUILayout.Space(GAP_SMALL_RECT_SIZE);
        EditorGUILayout.LabelField("Input and Output Previews", m_centerBoldLabelStyle);

        EditorGUILayout.BeginHorizontal();
        GUILayout.Space(GAP_SMALL_RECT_SIZE);
        DrawMeshPreview(m_tetrahedralizationMesh, 1, GUILayoutUtility.GetRect(MESH_PREVIEW_RECT_SIZE, MESH_PREVIEW_RECT_SIZE, GUILayout.ExpandWidth(false)));
        EditorGUI.DrawRect(GUILayoutUtility.GetRect(GAP_SMALL_RECT_SIZE, MESH_PREVIEW_RECT_SIZE, GUILayout.ExpandWidth(false)), Color.yellow);
        DrawMeshPreview(m_mesh, 0, GUILayoutUtility.GetRect(MESH_PREVIEW_RECT_SIZE, MESH_PREVIEW_RECT_SIZE, GUILayout.ExpandWidth(false)));
        EditorGUI.DrawRect(GUILayoutUtility.GetRect(GAP_SMALL_RECT_SIZE, MESH_PREVIEW_RECT_SIZE, GUILayout.ExpandWidth(false)), Color.yellow);
        DrawMeshPreview(m_tetrahedralMeshMesh, 2, GUILayoutUtility.GetRect(MESH_PREVIEW_RECT_SIZE, MESH_PREVIEW_RECT_SIZE, GUILayout.ExpandWidth(false)));
        EditorGUILayout.EndHorizontal();

        EditorGUILayout.BeginHorizontal();
        GUILayout.Space(GAP_SMALL_RECT_SIZE);
        EditorGUI.LabelField(GUILayoutUtility.GetRect(MESH_PREVIEW_RECT_SIZE, MESH_DESCRIPTION_RECT_SIZE, GUILayout.ExpandWidth(false)), 
        $"Input Tetrahedralization\nVertices Count: {(null==m_tetrahedralization ? 0:m_tetrahedralization.GetVerticesCount())}\nTetrahedrons Count: {(null==m_tetrahedralization ? 0:m_tetrahedralization.GetTetrahedronsCount())}");
        GUILayout.Space(GAP_SMALL_RECT_SIZE);
        EditorGUI.LabelField(GUILayoutUtility.GetRect(MESH_PREVIEW_RECT_SIZE, MESH_DESCRIPTION_RECT_SIZE, GUILayout.ExpandWidth(false)), 
        $"Input Mesh\nVertices Count: {(null==m_mesh ? 0:m_mesh.vertexCount)}\nTriangles Count: {(null==m_mesh ? 0:m_mesh.GetIndexCount()/3)}");
        GUILayout.Space(GAP_SMALL_RECT_SIZE);
        EditorGUI.LabelField(GUILayoutUtility.GetRect(MESH_PREVIEW_RECT_SIZE, MESH_DESCRIPTION_RECT_SIZE, GUILayout.ExpandWidth(false)), 
        $"Output Tetrahedral Mesh\nVertices Count: {(null==m_tetrahedralMesh ? 0:m_tetrahedralMesh.GetTetrahedronsCount()*12)}\nTetrahedrons Count: {(null==m_tetrahedralMesh ? 0:m_tetrahedralMesh.GetTetrahedronsCount())}");
        EditorGUILayout.EndHorizontal();
    }

    private void UpdateMesh(ref Mesh target, Mesh source)
    {
        target = source;
        if(null != target)
        {
            target.hideFlags = HideFlags.HideAndDontSave;
        }
    }

    private void DrawMeshPreview(Mesh mesh, int previewIndex, Rect rect)
    {
        if(null == mesh)
        {
            mesh = new Mesh(); // just draw something
        }
        m_meshesPreviews[previewIndex].AssignMesh(mesh);
        m_meshesPreviews[previewIndex].AssignMaterials(m_meshMaterials);

        EditorGUI.BeginChangeCheck();
        m_meshesPreviews[previewIndex].DrawOnGUI(rect);
        if(EditorGUI.EndChangeCheck() && m_synchronizeMeshesPreviews)
        {
            m_meshesPreviews.ForEach(i=>i.CopyPreviewParameters(m_meshesPreviews[previewIndex]));
        }
    }

    private void ResetMeshesPreviews()
    {
        m_meshesPreviews.ForEach(i=>i.ResetPreviewParameters());
    }
}

#endif
