#if UNITY_EDITOR

using System;
using System.IO;
using System.Linq;
using System.Collections.Generic;
using UnityEngine;
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
    private class TetrahedralizerEditorWindowSettings : ScriptableObject
    {
        private static TetrahedralizerEditorWindowSettings m_instance;
        public static TetrahedralizerEditorWindowSettings instance
        {
            get
            {
                if(null == m_instance)
                {
                    m_instance = CreateInstance<TetrahedralizerEditorWindowSettings>();
                    if(EditorPrefs.HasKey(EDITOR_PREFS_KEY))
                    {
                        m_instance.Load(EditorPrefs.GetString(EDITOR_PREFS_KEY));
                    }
                }
                return m_instance;
            }
        }

        public const string EDITOR_PREFS_KEY = nameof(TetrahedralizerEditorWindowSettings);

        public const string TETRAHEDRALIZER_LINK_DOCUMENTATION = "https://docs.google.com/document/d/1TmDsCK4SiXGq7Y7xVC6l-rAW5G5WxgNKjIlzR9Algpc/edit?usp=sharing";
        public const string TETRAHEDRALIZER_LINK_UNITY_ASSET_STORE = "https://assetstore.unity.com/packages/slug/306196";
        public const string TETRAHEDRALIZER_LINK_GITHUB = "https://github.com/hanzemeng/Tetrahedralizer";

        public const float MESH_PREVIEW_RECT_SIZE = 200f;
        public const float MESH_DESCRIPTION_RECT_SIZE = 50f;
        public const float GAP_SMALL_RECT_SIZE = 5f;

        public const int MESH_PREVIEWS_COUNT = 3;


        private GUIStyle m_centerBoldLabelStyleField;
        public GUIStyle m_centerBoldLabelStyle
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

        public Page m_currentPage;
        public bool m_synchronizeMeshesPreviews;

        public GameObject m_gameObject;
        public Tetrahedralization m_tetrahedralization;
        public TetrahedralMesh m_tetrahedralMesh;
        public Material m_material;


        public void Load(string data)
        {
            T LoadFromGUID<T>(string guid) where T : UnityEngine.Object
            {
                if(string.IsNullOrEmpty(guid))
                {
                    return null;
                }
                return (T)AssetDatabase.LoadAssetByGUID(new GUID(guid), typeof(T));
            }

            MemoryStream memoryStream = new MemoryStream(Convert.FromBase64String(data));
            BinaryReader binaryReader = new BinaryReader(memoryStream);

            m_currentPage = (Page)binaryReader.ReadInt32();
            m_synchronizeMeshesPreviews = binaryReader.ReadBoolean();
            m_gameObject = LoadFromGUID<GameObject>(binaryReader.ReadString());
            m_tetrahedralization = LoadFromGUID<Tetrahedralization>(binaryReader.ReadString());
            m_tetrahedralMesh = LoadFromGUID<TetrahedralMesh>(binaryReader.ReadString());
            m_material = LoadFromGUID<Material>(binaryReader.ReadString());

            binaryReader.Dispose();
            memoryStream.Dispose();
        }
        public void Save()
        {
            string GetGUID(UnityEngine.Object obj)
            {
                if(null == obj)
                {
                    return string.Empty;
                }
                return obj.GetGUID();
            }

            MemoryStream memoryStream = new MemoryStream();
            BinaryWriter binaryWriter = new BinaryWriter(memoryStream);

            binaryWriter.Write((int)m_currentPage);
            binaryWriter.Write(m_synchronizeMeshesPreviews);
            binaryWriter.Write(GetGUID(m_gameObject));
            binaryWriter.Write(GetGUID(m_tetrahedralization));
            binaryWriter.Write(GetGUID(m_tetrahedralMesh));
            binaryWriter.Write(GetGUID(m_material));

            EditorPrefs.SetString(EDITOR_PREFS_KEY, Convert.ToBase64String(memoryStream.ToArray())) ;

            binaryWriter.Dispose();
            memoryStream.Dispose();
        }
    }


    private bool m_asyncTaskIsRunning;
    private List<MeshPreviewField> m_meshesPreviews;

    private TetrahedralizerEditorWindowSettings m_settings;

    private Mesh m_mesh;
    private List<Material> m_meshMaterials;
    private Mesh m_tetrahedralizationMesh;
    private Mesh m_tetrahedralMeshMesh; // genius name, I know

    [MenuItem(TetrahedralizerConstant.TETRAHEDRALIZER_WINDOW_PATH + TetrahedralizerConstant.TETRAHEDRALIZER_NAME)]
    public static void Open()
    {
        GetWindow<TetrahedralizerEditorWindow>().titleContent = new GUIContent(TetrahedralizerConstant.TETRAHEDRALIZER_NAME);
    }

    private void OnEnable()
    {
        m_settings = TetrahedralizerEditorWindowSettings.instance;
        if(null != m_settings.m_tetrahedralization)
        {
            UpdateMesh(ref m_tetrahedralizationMesh, m_settings.m_tetrahedralization.ToMesh().mesh);
        }
        if(null != m_settings.m_tetrahedralMesh)
        {
            UpdateMesh(ref m_tetrahedralMeshMesh, m_settings.m_tetrahedralMesh.ToMesh().mesh);
        }

        m_asyncTaskIsRunning = false;
        m_meshesPreviews = Enumerable.Range(0, TetrahedralizerEditorWindowSettings.MESH_PREVIEWS_COUNT).Select(i=>new MeshPreviewField()).ToList();
        m_meshMaterials = new List<Material>();
    }
    private void OnDisable()
    {
        m_meshesPreviews.ForEach(i=>i.Dispose());
        m_settings.Save();
    }

    private void OnGUI()
    {
        if(m_asyncTaskIsRunning)
        {
            EditorGUILayout.HelpBox("An async task is running.", MessageType.Warning);
        }
        m_settings = TetrahedralizerEditorWindowSettings.instance;
        m_settings.m_currentPage = (Page)GUILayout.Toolbar((int)m_settings.m_currentPage, Enum.GetNames(typeof(Page)).Select(i=>TetrahedralizerUtility.UpperSnakeCaseToCapitalizedWords(i)).ToArray());

        GUILayout.Space(TetrahedralizerEditorWindowSettings.GAP_SMALL_RECT_SIZE);

        if(m_settings.m_currentPage == Page.OVERVIEW)
        {
            DrawOverviewPage();
        }
        else if(m_settings.m_currentPage == Page.TETRAHEDRALIZED_MESH)
        {
            if(DrawTargetGameObjectOnGUI())
            {
                _ = DrawTetrahedralizedMeshPage();
                DrawPreviewSettingsOnGUI();
            }
        }
        else if(m_settings.m_currentPage == Page.TETRAHEDRAL_MESH)
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
            Application.OpenURL(TetrahedralizerEditorWindowSettings.TETRAHEDRALIZER_LINK_DOCUMENTATION);
        }
        if(GUILayout.Button($"Open {TetrahedralizerConstant.TETRAHEDRALIZER_NAME} on Unity Asset Store"))
        {
            Application.OpenURL(TetrahedralizerEditorWindowSettings.TETRAHEDRALIZER_LINK_UNITY_ASSET_STORE);
        }
        if(GUILayout.Button($"Open {TetrahedralizerConstant.TETRAHEDRALIZER_NAME} on GitHub"))
        {
            Application.OpenURL(TetrahedralizerEditorWindowSettings.TETRAHEDRALIZER_LINK_GITHUB);
        }
    }

    private bool DrawTargetGameObjectOnGUI()
    {
        m_settings.m_gameObject = (GameObject)EditorGUILayout.ObjectField(new GUIContent("Target Game Object:", "The GameObject to be tetrahedralized. We only care about its Mesh in MeshFilter and Materials in MeshRenderer."), m_settings.m_gameObject, typeof(GameObject), true);

        if(null == m_settings.m_gameObject)
        {
            EditorGUILayout.HelpBox("The target GameObject must be assigned.", MessageType.Error);
            return false;
        }
        MeshFilter meshFilter;
        if(!m_settings.m_gameObject.TryGetComponent<MeshFilter>(out meshFilter) || null == meshFilter.sharedMesh)
        {
            EditorGUILayout.HelpBox("The target GameObject has no MeshFilter with a valid Mesh.", MessageType.Error);
            return false;
        }
        MeshRenderer meshRenderer;
        if(!m_settings.m_gameObject.TryGetComponent<MeshRenderer>(out meshRenderer))
        {
            EditorGUILayout.HelpBox("The target GameObject has no MeshRenderer.", MessageType.Error);
            return false;
        }
        meshRenderer.GetSharedMaterials(m_meshMaterials);
        if(0 == m_meshMaterials.Count || m_meshMaterials.Where(i=>null==i).Any())
        {
            EditorGUILayout.HelpBox("The target GameObject's MeshRenderer has no Materials or has invalid Material(s).", MessageType.Error);
            return false;
        }

        m_mesh = meshFilter.sharedMesh;

        EditorGUI.BeginDisabledGroup(true);
        EditorGUILayout.ObjectField(new GUIContent("Input Mesh:", "The Mesh in the MeshFilter of the target GameObject."), m_mesh, typeof(Mesh), false);
        for(int i=0; i<m_meshMaterials.Count; i++)
        {
            EditorGUILayout.ObjectField(new GUIContent($"Input Material {i}:", $"The {i}th Material in the MeshRenderer of the target GameObject."), m_meshMaterials[i], typeof(Material), false);
        }
        EditorGUI.EndDisabledGroup();

        return true;
    }

    private void DrawPreviewSettingsOnGUI()
    {
        m_settings.m_material = (Material)EditorGUILayout.ObjectField(new GUIContent("Interior Material:", "The TetrahedralMesh contains an additional submesh than the input Mesh. This material is used to render that submesh."), m_settings.m_material, typeof(Material), false);

        if(!m_asyncTaskIsRunning && GUILayout.Button($"Reset Meshes Previews"))
        {
            ResetMeshesPreviews();
        }

        if((m_settings.m_synchronizeMeshesPreviews && GUILayout.Button($"Synchronize Meshes Previews")) || 
           (!m_settings.m_synchronizeMeshesPreviews && GUILayout.Button($"Unsynchronize Meshes Previews")))
        {
            m_settings.m_synchronizeMeshesPreviews = !m_settings.m_synchronizeMeshesPreviews;
        }

        EditorGUILayout.LabelField("Mesh Preview Controls", EditorStyles.boldLabel);
        EditorGUILayout.LabelField("Left Drag: Rotate Mesh");
        EditorGUILayout.LabelField("Right Drag: Rotate Lights");
        EditorGUILayout.LabelField("Scroll Wheel Drag: Pan Camera");
        EditorGUILayout.LabelField("Scroll Wheel Scroll: Zoom Camera");
    }

    private async Task DrawTetrahedralizedMeshPage()
    {
        EditorGUI.BeginChangeCheck();
        m_settings.m_tetrahedralization = (Tetrahedralization)EditorGUILayout.ObjectField(new GUIContent("Output Tetrahedralization:", "The resulting Tetrahedralization. It represents the convex hull of the target GameObject using tetrahedrons. It only knows the vertices’ positions."), m_settings.m_tetrahedralization, typeof(Tetrahedralization), false);
        if(EditorGUI.EndChangeCheck())
        {
            UpdateMesh(ref m_tetrahedralizationMesh, null == m_settings.m_tetrahedralization ? null : m_settings.m_tetrahedralization.ToMesh().mesh);
        }

        GUILayout.Space(TetrahedralizerEditorWindowSettings.GAP_SMALL_RECT_SIZE);
        if(!m_asyncTaskIsRunning && null!=m_settings.m_tetrahedralization && GUILayout.Button($"Create Tetrahedralized Mesh"))
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
            m_settings.m_tetrahedralization.Assign(output.m_tetrahedralization);
            EditorUtility.SetDirty(m_settings.m_tetrahedralization);

            UpdateMesh(ref m_tetrahedralizationMesh, null == m_settings.m_tetrahedralization ? null : m_settings.m_tetrahedralization.ToMesh().mesh);
        }

        GUILayout.Space(TetrahedralizerEditorWindowSettings.GAP_SMALL_RECT_SIZE);
        EditorGUILayout.LabelField("Input and Output Previews", m_settings.m_centerBoldLabelStyle);

        EditorGUILayout.BeginHorizontal();
        GUILayout.Space(TetrahedralizerEditorWindowSettings.GAP_SMALL_RECT_SIZE);
        DrawMeshPreview(m_mesh, 0, GUILayoutUtility.GetRect(TetrahedralizerEditorWindowSettings.MESH_PREVIEW_RECT_SIZE, TetrahedralizerEditorWindowSettings.MESH_PREVIEW_RECT_SIZE, GUILayout.ExpandWidth(false)), false);
        EditorGUI.DrawRect(GUILayoutUtility.GetRect(TetrahedralizerEditorWindowSettings.GAP_SMALL_RECT_SIZE, TetrahedralizerEditorWindowSettings.MESH_PREVIEW_RECT_SIZE, GUILayout.ExpandWidth(false)), Color.yellow);
        DrawMeshPreview(m_tetrahedralizationMesh, 1, GUILayoutUtility.GetRect(TetrahedralizerEditorWindowSettings.MESH_PREVIEW_RECT_SIZE, TetrahedralizerEditorWindowSettings.MESH_PREVIEW_RECT_SIZE, GUILayout.ExpandWidth(false)), false);
        EditorGUILayout.EndHorizontal();

        EditorGUILayout.BeginHorizontal();
        GUILayout.Space(TetrahedralizerEditorWindowSettings.GAP_SMALL_RECT_SIZE);
        EditorGUI.LabelField(GUILayoutUtility.GetRect(TetrahedralizerEditorWindowSettings.MESH_PREVIEW_RECT_SIZE, TetrahedralizerEditorWindowSettings.MESH_DESCRIPTION_RECT_SIZE, GUILayout.ExpandWidth(false)), 
        $"Input Mesh:\nVertices Count: {(null==m_mesh ? 0:m_mesh.vertexCount)}\nTriangles Count: {(null==m_mesh ? 0:m_mesh.GetIndexCount()/3)}");
        GUILayout.Space(TetrahedralizerEditorWindowSettings.GAP_SMALL_RECT_SIZE);
        EditorGUI.LabelField(GUILayoutUtility.GetRect(TetrahedralizerEditorWindowSettings.MESH_PREVIEW_RECT_SIZE, TetrahedralizerEditorWindowSettings.MESH_DESCRIPTION_RECT_SIZE, GUILayout.ExpandWidth(false)), 
        $"Output Tetrahedralization:\nVertices Count: {(null==m_settings.m_tetrahedralization ? 0:m_settings.m_tetrahedralization.GetVerticesCount())}\nTetrahedrons Count: {(null==m_settings.m_tetrahedralization ? 0:m_settings.m_tetrahedralization.GetTetrahedronsCount())}");
        EditorGUILayout.EndHorizontal();
    }

    private async Task DrawTetrahedralMeshPage()
    {
        EditorGUI.BeginChangeCheck();
        m_settings.m_tetrahedralization = (Tetrahedralization)EditorGUILayout.ObjectField(new GUIContent("Input Tetrahedralization:", "The Tetrahedralization that represents the convex hull of the target GameObject."), m_settings.m_tetrahedralization, typeof(Tetrahedralization), false);
        if(EditorGUI.EndChangeCheck())
        {
            UpdateMesh(ref m_tetrahedralizationMesh, null == m_settings.m_tetrahedralization ? null : m_settings.m_tetrahedralization.ToMesh().mesh);
        }
        EditorGUI.BeginChangeCheck();
        m_settings.m_tetrahedralMesh = (TetrahedralMesh)EditorGUILayout.ObjectField(new GUIContent("Output Tetrahedral Mesh:", "The resulting TetrahedralMesh. It represents the input Mesh using tetrahedrons. It contains every vertex attribute in the input Mesh. It contains one more submesh that is used to store tetrahedron facets that are not defined in the input Mesh."), m_settings.m_tetrahedralMesh, typeof(TetrahedralMesh), false);
        if(EditorGUI.EndChangeCheck())
        {
            UpdateMesh(ref m_tetrahedralMeshMesh, null == m_settings.m_tetrahedralMesh ? null : m_settings.m_tetrahedralMesh.ToMesh().mesh);
        }

        GUILayout.Space(TetrahedralizerEditorWindowSettings.GAP_SMALL_RECT_SIZE);
        if(!m_asyncTaskIsRunning && null!=m_settings.m_tetrahedralization && null!=m_settings.m_tetrahedralMesh && GUILayout.Button($"Create Tetrahedral Mesh"))
        {
            TetrahedralMeshCreation tetrahedralMeshCreation = new TetrahedralMeshCreation();
            TetrahedralMeshCreation.TetrahedralMeshCreationInput input = new TetrahedralMeshCreation.TetrahedralMeshCreationInput();
            TetrahedralMeshCreation.TetrahedralMeshCreationOutput output = new TetrahedralMeshCreation.TetrahedralMeshCreationOutput();
            input.m_mesh = m_mesh;
            input.m_tetrahedralization = m_settings.m_tetrahedralization;

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
            m_settings.m_tetrahedralMesh.Assign(output.m_tetrahedralMesh);
            EditorUtility.SetDirty(m_settings.m_tetrahedralMesh);

            UpdateMesh(ref m_tetrahedralMeshMesh, null == m_settings.m_tetrahedralMesh ? null : m_settings.m_tetrahedralMesh.ToMesh().mesh);
        }


        GUILayout.Space(TetrahedralizerEditorWindowSettings.GAP_SMALL_RECT_SIZE);
        EditorGUILayout.LabelField("Input and Output Previews", m_settings.m_centerBoldLabelStyle);

        EditorGUILayout.BeginHorizontal();
        GUILayout.Space(TetrahedralizerEditorWindowSettings.GAP_SMALL_RECT_SIZE);
        DrawMeshPreview(m_tetrahedralizationMesh, 1, GUILayoutUtility.GetRect(TetrahedralizerEditorWindowSettings.MESH_PREVIEW_RECT_SIZE, TetrahedralizerEditorWindowSettings.MESH_PREVIEW_RECT_SIZE, GUILayout.ExpandWidth(false)), false);
        EditorGUI.DrawRect(GUILayoutUtility.GetRect(TetrahedralizerEditorWindowSettings.GAP_SMALL_RECT_SIZE, TetrahedralizerEditorWindowSettings.MESH_PREVIEW_RECT_SIZE, GUILayout.ExpandWidth(false)), Color.yellow);
        DrawMeshPreview(m_mesh, 0, GUILayoutUtility.GetRect(TetrahedralizerEditorWindowSettings.MESH_PREVIEW_RECT_SIZE, TetrahedralizerEditorWindowSettings.MESH_PREVIEW_RECT_SIZE, GUILayout.ExpandWidth(false)), false);
        EditorGUI.DrawRect(GUILayoutUtility.GetRect(TetrahedralizerEditorWindowSettings.GAP_SMALL_RECT_SIZE, TetrahedralizerEditorWindowSettings.MESH_PREVIEW_RECT_SIZE, GUILayout.ExpandWidth(false)), Color.yellow);
        DrawMeshPreview(m_tetrahedralMeshMesh, 2, GUILayoutUtility.GetRect(TetrahedralizerEditorWindowSettings.MESH_PREVIEW_RECT_SIZE, TetrahedralizerEditorWindowSettings.MESH_PREVIEW_RECT_SIZE, GUILayout.ExpandWidth(false)), true);
        EditorGUILayout.EndHorizontal();

        EditorGUILayout.BeginHorizontal();
        GUILayout.Space(TetrahedralizerEditorWindowSettings.GAP_SMALL_RECT_SIZE);
        EditorGUI.LabelField(GUILayoutUtility.GetRect(TetrahedralizerEditorWindowSettings.MESH_PREVIEW_RECT_SIZE, TetrahedralizerEditorWindowSettings.MESH_DESCRIPTION_RECT_SIZE, GUILayout.ExpandWidth(false)), 
        $"Input Tetrahedralization:\nVertices Count: {(null==m_settings.m_tetrahedralization ? 0:m_settings.m_tetrahedralization.GetVerticesCount())}\nTetrahedrons Count: {(null==m_settings.m_tetrahedralization ? 0:m_settings.m_tetrahedralization.GetTetrahedronsCount())}");
        GUILayout.Space(TetrahedralizerEditorWindowSettings.GAP_SMALL_RECT_SIZE);
        EditorGUI.LabelField(GUILayoutUtility.GetRect(TetrahedralizerEditorWindowSettings.MESH_PREVIEW_RECT_SIZE, TetrahedralizerEditorWindowSettings.MESH_DESCRIPTION_RECT_SIZE, GUILayout.ExpandWidth(false)), 
        $"Input Mesh:\nVertices Count: {(null==m_mesh ? 0:m_mesh.vertexCount)}\nTriangles Count: {(null==m_mesh ? 0:m_mesh.GetIndexCount()/3)}");
        GUILayout.Space(TetrahedralizerEditorWindowSettings.GAP_SMALL_RECT_SIZE);
        EditorGUI.LabelField(GUILayoutUtility.GetRect(TetrahedralizerEditorWindowSettings.MESH_PREVIEW_RECT_SIZE, TetrahedralizerEditorWindowSettings.MESH_DESCRIPTION_RECT_SIZE, GUILayout.ExpandWidth(false)), 
        $"Output Tetrahedral Mesh:\nVertices Count: {(null==m_settings.m_tetrahedralMesh ? 0:m_settings.m_tetrahedralMesh.GetTetrahedronsCount()*12)}\nTetrahedrons Count: {(null==m_settings.m_tetrahedralMesh ? 0:m_settings.m_tetrahedralMesh.GetTetrahedronsCount())}");
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

    private void DrawMeshPreview(Mesh mesh, int previewIndex, Rect rect, bool useInteriorMaterial)
    {
        if(null == mesh)
        {
            mesh = new Mesh(); // just draw something
        }
        m_meshesPreviews[previewIndex].AssignMesh(mesh);
        if(useInteriorMaterial)
        {
            List<Material> temp = m_meshMaterials.Select(i=>i).ToList();
            temp.Add(m_settings.m_material);
            m_meshesPreviews[previewIndex].AssignMaterials(temp);
        }
        else
        {
            m_meshesPreviews[previewIndex].AssignMaterials(m_meshMaterials);
        }

        EditorGUI.BeginChangeCheck();
        m_meshesPreviews[previewIndex].DrawOnGUI(rect);
        if(EditorGUI.EndChangeCheck() && m_settings.m_synchronizeMeshesPreviews)
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
