// CustomMeshPreview is a modified version of MeshPreview that allows multiple interactive mesh previews

using System;
using System.Collections.Generic;
using System.Linq;
using UnityEngine;
using UnityEngine.Rendering;
using UnityEngine.Rendering.Universal;
using UnityEngine.Rendering.HighDefinition;
using UnityEditor;

//
// Summary:
//     Use this class to render an interactive preview of a mesh.
public class CustomMeshPreview : IDisposable
{
    internal static class Styles
    {
        public static readonly GUIContent wireframeToggle = EditorGUIUtility.TrTextContent("Wireframe", "Show wireframe");

        public static GUIContent displayModeDropdown = EditorGUIUtility.TrTextContent("", "Change display mode");

        public static GUIContent uvChannelDropdown = EditorGUIUtility.TrTextContent("", "Change active UV channel");

        public static GUIStyle preSlider = "preSlider";

        public static GUIStyle preSliderThumb = "preSliderThumb";
    }

    private static int sliderHash = "Slider".GetHashCode();

    internal class Settings : IDisposable
    {
        private DisplayMode m_DisplayMode = DisplayMode.Shaded;

        private int m_ActiveUVChannel = 0;

        private int m_ActiveBlendshape = 0;

        private bool m_DrawWire = true;

        private Vector3 m_OrthoPosition = new Vector3(0f, 0f, 0f);

        private Vector2 m_PreviewDir = new Vector2(0f, 0f);

        private Vector2 m_LightDir = new Vector2(0f, 0f);

        private Vector3 m_PivotPositionOffset = Vector3.zero;

        private float m_ZoomFactor = 1f;

        private int m_CheckerTextureMultiplier = 10;

        private Material m_ShadedPreviewMaterial;

        private Material m_ActiveMaterial;

        private Material m_MeshMultiPreviewMaterial;

        private Material m_WireMaterial;

        private Material m_LineMaterial;

        private Texture2D m_CheckeredTexture;

        private bool[] m_AvailableDisplayModes = Enumerable.Repeat(element: true, 7).ToArray();

        private bool[] m_AvailableUVChannels = Enumerable.Repeat(element: true, 8).ToArray();

        public DisplayMode displayMode
        {
            get
            {
                return m_DisplayMode;
            }
            set
            {
                SetValue(ref m_DisplayMode, value);
            }
        }

        public int activeUVChannel
        {
            get
            {
                return m_ActiveUVChannel;
            }
            set
            {
                SetValue(ref m_ActiveUVChannel, value);
            }
        }

        public int activeBlendshape
        {
            get
            {
                return m_ActiveBlendshape;
            }
            set
            {
                SetValue(ref m_ActiveBlendshape, value);
            }
        }

        public bool drawWire
        {
            get
            {
                return m_DrawWire;
            }
            set
            {
                SetValue(ref m_DrawWire, value);
            }
        }

        public Vector3 orthoPosition
        {
            get
            {
                return m_OrthoPosition;
            }
            set
            {
                SetValue(ref m_OrthoPosition, value);
            }
        }

        public Vector2 previewDir
        {
            get
            {
                return m_PreviewDir;
            }
            set
            {
                SetValue(ref m_PreviewDir, value);
            }
        }

        public Vector2 lightDir
        {
            get
            {
                return m_LightDir;
            }
            set
            {
                SetValue(ref m_LightDir, value);
            }
        }

        public Vector3 pivotPositionOffset
        {
            get
            {
                return m_PivotPositionOffset;
            }
            set
            {
                SetValue(ref m_PivotPositionOffset, value);
            }
        }

        public float zoomFactor
        {
            get
            {
                return m_ZoomFactor;
            }
            set
            {
                SetValue(ref m_ZoomFactor, value);
            }
        }

        public int checkerTextureMultiplier
        {
            get
            {
                return m_CheckerTextureMultiplier;
            }
            set
            {
                SetValue(ref m_CheckerTextureMultiplier, value);
            }
        }

        public Material shadedPreviewMaterial
        {
            get
            {
                return m_ShadedPreviewMaterial;
            }
            set
            {
                SetValue(ref m_ShadedPreviewMaterial, value);
            }
        }

        public Material activeMaterial
        {
            get
            {
                return m_ActiveMaterial;
            }
            set
            {
                SetValue(ref m_ActiveMaterial, value);
            }
        }

        public Material meshMultiPreviewMaterial
        {
            get
            {
                return m_MeshMultiPreviewMaterial;
            }
            set
            {
                SetValue(ref m_MeshMultiPreviewMaterial, value);
            }
        }

        public Material wireMaterial
        {
            get
            {
                return m_WireMaterial;
            }
            set
            {
                SetValue(ref m_WireMaterial, value);
            }
        }

        public Material lineMaterial
        {
            get
            {
                return m_LineMaterial;
            }
            set
            {
                SetValue(ref m_LineMaterial, value);
            }
        }

        public Texture2D checkeredTexture
        {
            get
            {
                return m_CheckeredTexture;
            }
            set
            {
                SetValue(ref m_CheckeredTexture, value);
            }
        }

        public bool[] availableDisplayModes
        {
            get
            {
                return m_AvailableDisplayModes;
            }
            set
            {
                SetValue(ref m_AvailableDisplayModes, value);
            }
        }

        public bool[] availableUVChannels
        {
            get
            {
                return m_AvailableUVChannels;
            }
            set
            {
                SetValue(ref m_AvailableUVChannels, value);
            }
        }

        public event Action changed;

        public Settings()
        {
            shadedPreviewMaterial = new Material(Shader.Find("Standard"));
            wireMaterial = CreateWireframeMaterial();
            meshMultiPreviewMaterial = CreateMeshMultiPreviewMaterial();
            lineMaterial = CreateLineMaterial();
            checkeredTexture = EditorGUIUtility.LoadRequired("Previews/Textures/textureChecker.png") as Texture2D;
            activeMaterial = shadedPreviewMaterial;
            orthoPosition = new Vector3(0.5f, 0.5f, -1f);
            previewDir = new Vector2(130f, 0f);
            lightDir = new Vector2(-40f, -40f);
            zoomFactor = 1f;
        }

        public void Dispose()
        {
            if (shadedPreviewMaterial != null)
            {
                UnityEngine.Object.DestroyImmediate(shadedPreviewMaterial);
            }

            if (wireMaterial != null)
            {
                UnityEngine.Object.DestroyImmediate(wireMaterial);
            }

            if (meshMultiPreviewMaterial != null)
            {
                UnityEngine.Object.DestroyImmediate(meshMultiPreviewMaterial);
            }

            if (lineMaterial != null)
            {
                UnityEngine.Object.DestroyImmediate(lineMaterial);
            }
        }

        private void SetValue<T>(ref T setting, T newValue)
        {
            if (setting != null)
            {
                object obj = newValue;
                if (setting.Equals(obj))
                {
                    return;
                }
            }

            setting = newValue;
            this.changed?.Invoke();
        }

        public void Copy(Settings other)
        {
            displayMode = other.displayMode;
            activeUVChannel = other.activeUVChannel;
            activeBlendshape = other.activeBlendshape;
            drawWire = other.drawWire;
            orthoPosition = other.orthoPosition;
            previewDir = other.previewDir;
            lightDir = other.lightDir;
            pivotPositionOffset = other.pivotPositionOffset;
            zoomFactor = other.zoomFactor;
            checkerTextureMultiplier = other.checkerTextureMultiplier;
            shadedPreviewMaterial = other.shadedPreviewMaterial;
            activeMaterial = other.activeMaterial;
            meshMultiPreviewMaterial = other.meshMultiPreviewMaterial;
            wireMaterial = other.wireMaterial;
            lineMaterial = other.lineMaterial;
            checkeredTexture = other.checkeredTexture;
            availableDisplayModes = new bool[other.availableDisplayModes.Length];
            Array.Copy(other.availableDisplayModes, availableDisplayModes, other.availableDisplayModes.Length);
            availableUVChannels = new bool[other.availableUVChannels.Length];
            Array.Copy(other.availableUVChannels, availableUVChannels, other.availableUVChannels.Length);
        }
    }

    internal enum DisplayMode
    {
        Shaded,
        UVChecker,
        UVLayout,
        VertexColor,
        Normals,
        Tangent,
        Blendshapes
    }

    private static string[] m_DisplayModes = new string[7] { "Shaded", "UV Checker", "UV Layout", "Vertex Color", "Normals", "Tangents", "Blendshapes" };

    private static string[] m_UVChannels = new string[8] { "Channel 0", "Channel 1", "Channel 2", "Channel 3", "Channel 4", "Channel 5", "Channel 6", "Channel 7" };

    private Mesh m_Target;

    private PreviewRenderUtility m_PreviewUtility;

    private Settings m_Settings;

    private Mesh m_BakedSkinnedMesh;

    private List<string> m_BlendShapes;

    //
    // Summary:
    //     The Mesh to display in the preview space.
    public Mesh mesh
    {
        get
        {
            return m_Target;
        }
        set
        {
            m_Target = value;
        }
    }

    internal event Action<CustomMeshPreview> settingsChanged;

    //
    // Summary:
    //     Creates a new MeshPreview instance with a Mesh target.
    //
    // Parameters:
    //   target:
    //     The Mesh to be displayed.
    public CustomMeshPreview(Mesh target)
    {
        m_Target = target;
        m_PreviewUtility = new PreviewRenderUtility();
        m_PreviewUtility.camera.fieldOfView = 30f;
        m_PreviewUtility.camera.transform.position = new Vector3(5f, 5f, 0f);
        m_Settings = new Settings();
        ModelImporter modelImporter = AssetImporter.GetAtPath(AssetDatabase.GetAssetPath(target)) as ModelImporter;
        if ((bool)modelImporter && modelImporter.bakeAxisConversion)
        {
            m_Settings.previewDir += new Vector2(180f, 0f);
        }

        m_Settings.changed += OnSettingsChanged;
        m_BlendShapes = new List<string>();
        CheckAvailableAttributes();
    }

    //
    // Summary:
    //     Releases allocated resources associated with this object.
    public void Dispose()
    {
        DestroyBakedSkinnedMesh();
        m_PreviewUtility.Cleanup();
        m_Settings.changed -= OnSettingsChanged;
        m_Settings.Dispose();
    }

    private void OnSettingsChanged()
    {
        this.settingsChanged?.Invoke(this);
    }

    private static Material GetDefaultMaterial()
    {
        Material material = new Material(Shader.Find("Standard"));
        //#if UNITY_RENDER_PIPELINE_UNIVERSAL
        //if(GraphicsSettings.currentRenderPipeline is UniversalRenderPipelineAsset urpAsset)
        //{
        //    material = urpAsset.defaultMaterial;
        //}
        //#endif
        //#if UNITY_RENDER_PIPELINE_HIGH_DEFINITION
        //if(GraphicsSettings.currentRenderPipeline is HDRenderPipelineAsset hdrpAsset)
        //{
        //    material = hdrpAsset.defaultMaterial;
        //}
        //#endif
        //if(null == material)
        //{
        //    material = new Material(Shader.Find("Standard"));
        //}
        return material;
    }

    private static Material CreateWireframeMaterial()
    {
        Material material = GetDefaultMaterial();
        material.hideFlags = HideFlags.HideAndDontSave;
        material.SetColor("_Color", new Color(0f, 0f, 0f, 0.3f));
        material.SetFloat("_ZWrite", 0f);
        material.SetFloat("_ZBias", -1f);
        return material;
    }

    private static Material CreateMeshMultiPreviewMaterial()
    {
        Shader shader = EditorGUIUtility.LoadRequired("Previews/MeshPreviewShader.shader") as Shader;
        if (!shader)
        {
            Debug.LogWarning("Could not find the built in Mesh preview shader");
            return null;
        }

        Material material = new Material(shader);
        material.hideFlags = HideFlags.HideAndDontSave;
        return material;
    }

    private static Material CreateLineMaterial()
    {
        Material material = GetDefaultMaterial();
        material.hideFlags = HideFlags.HideAndDontSave;
        material.SetFloat("_SrcBlend", 5f);
        material.SetFloat("_DstBlend", 10f);
        material.SetFloat("_Cull", 0f);
        material.SetFloat("_ZWrite", 0f);
        return material;
    }

    private void ResetView()
    {
        m_Settings.zoomFactor = 1f;
        m_Settings.orthoPosition = new Vector3(0.5f, 0.5f, -1f);
        m_Settings.pivotPositionOffset = Vector3.zero;
        m_Settings.activeUVChannel = 0;
        m_Settings.meshMultiPreviewMaterial.SetFloat("_UVChannel", m_Settings.activeUVChannel);
        m_Settings.meshMultiPreviewMaterial.SetTexture("_MainTex", null);
        m_Settings.activeBlendshape = 0;
    }

    private void FrameObject()
    {
        m_Settings.zoomFactor = 1f;
        m_Settings.orthoPosition = new Vector3(0.5f, 0.5f, -1f);
        m_Settings.pivotPositionOffset = Vector3.zero;
    }

    private void CheckAvailableAttributes()
    {
        if (!mesh.HasVertexAttribute(VertexAttribute.Color))
        {
            m_Settings.availableDisplayModes[3] = false;
        }

        if (!mesh.HasVertexAttribute(VertexAttribute.Normal))
        {
            m_Settings.availableDisplayModes[4] = false;
        }

        if (!mesh.HasVertexAttribute(VertexAttribute.Tangent))
        {
            m_Settings.availableDisplayModes[5] = false;
        }

        int num = 0;
        for (int i = 4; i < 12; i++)
        {
            if (!mesh.HasVertexAttribute((VertexAttribute)i))
            {
                m_Settings.availableUVChannels[num] = false;
            }

            num++;
        }

        int blendShapeCount = mesh.blendShapeCount;
        if (blendShapeCount > 0)
        {
            for (int j = 0; j < blendShapeCount; j++)
            {
                m_BlendShapes.Add(mesh.GetBlendShapeName(j));
            }
        }
        else
        {
            m_Settings.availableDisplayModes[6] = false;
        }
    }

    private void DoPopup(Rect popupRect, string[] elements, int selectedIndex, GenericMenu.MenuFunction2 func, bool[] disabledItems)
    {
        GenericMenu genericMenu = new GenericMenu();
        for (int i = 0; i < elements.Length; i++)
        {
            string text = elements[i];
            if (!(text == m_DisplayModes[6]) || Selection.count <= 1)
            {
                if (disabledItems == null || disabledItems[i])
                {
                    genericMenu.AddItem(new GUIContent(text), i == selectedIndex, func, i);
                }
                else
                {
                    genericMenu.AddDisabledItem(new GUIContent(text));
                }
            }
        }

        genericMenu.DropDown(popupRect);
    }

    private void SetUVChannel(object data)
    {
        int num = (int)data;
        if (num >= 0 && num < m_Settings.availableUVChannels.Length)
        {
            m_Settings.activeUVChannel = num;
            if (m_Settings.displayMode == DisplayMode.UVLayout || m_Settings.displayMode == DisplayMode.UVChecker)
            {
                m_Settings.activeMaterial.SetFloat("_UVChannel", num);
            }
        }
    }

    private void DestroyBakedSkinnedMesh()
    {
        if ((bool)m_BakedSkinnedMesh)
        {
            UnityEngine.Object.DestroyImmediate(m_BakedSkinnedMesh);
        }
    }

    private void SetDisplayMode(object data)
    {
        int num = (int)data;
        if (num >= 0 && num < m_DisplayModes.Length)
        {
            m_Settings.displayMode = (DisplayMode)num;
            DestroyBakedSkinnedMesh();
            switch (m_Settings.displayMode)
            {
                case DisplayMode.Shaded:
                    OnDropDownAction(m_Settings.shadedPreviewMaterial, 0, flatUVs: false);
                    break;
                case DisplayMode.UVChecker:
                    OnDropDownAction(m_Settings.meshMultiPreviewMaterial, 4, flatUVs: false);
                    m_Settings.meshMultiPreviewMaterial.SetTexture("_MainTex", m_Settings.checkeredTexture);
                    m_Settings.meshMultiPreviewMaterial.mainTextureScale = new Vector2(m_Settings.checkerTextureMultiplier, m_Settings.checkerTextureMultiplier);
                    break;
                case DisplayMode.UVLayout:
                    OnDropDownAction(m_Settings.meshMultiPreviewMaterial, 0, flatUVs: true);
                    break;
                case DisplayMode.VertexColor:
                    OnDropDownAction(m_Settings.meshMultiPreviewMaterial, 1, flatUVs: false);
                    break;
                case DisplayMode.Normals:
                    OnDropDownAction(m_Settings.meshMultiPreviewMaterial, 2, flatUVs: false);
                    break;
                case DisplayMode.Tangent:
                    OnDropDownAction(m_Settings.meshMultiPreviewMaterial, 3, flatUVs: false);
                    break;
                case DisplayMode.Blendshapes:
                    OnDropDownAction(m_Settings.shadedPreviewMaterial, 0, flatUVs: false);
                    BakeSkinnedMesh();
                    break;
            }
        }
    }

    private void SetBlendshape(object data)
    {
        int num = (int)data;
        if (num >= 0 && num < m_BlendShapes.Count)
        {
            m_Settings.activeBlendshape = num;
            DestroyBakedSkinnedMesh();
            BakeSkinnedMesh();
        }
    }

    internal void CopySettings(CustomMeshPreview other)
    {
        m_Settings.Copy(other.m_Settings);
    }

    internal static void RenderMeshPreview(Mesh mesh, PreviewRenderUtility previewUtility, Settings settings, int meshSubset)
    {
        if (!(mesh == null) && previewUtility != null)
        {
            Bounds bounds = mesh.bounds;
            Transform component = previewUtility.camera.GetComponent<Transform>();
            previewUtility.camera.nearClipPlane = 0.0001f;
            previewUtility.camera.farClipPlane = 1000f;
            if (settings.displayMode == DisplayMode.UVLayout)
            {
                previewUtility.camera.orthographic = true;
                previewUtility.camera.orthographicSize = settings.zoomFactor;
                component.position = settings.orthoPosition;
                component.rotation = Quaternion.identity;
                DrawUVLayout(mesh, previewUtility, settings);
                return;
            }

            float magnitude = bounds.extents.magnitude;
            float num = 4f * magnitude;
            previewUtility.camera.orthographic = false;
            Quaternion identity = Quaternion.identity;
            Vector3 position = identity * Vector3.forward * ((0f - num) * settings.zoomFactor) + settings.pivotPositionOffset;
            component.position = position;
            component.rotation = identity;
            previewUtility.lights[0].intensity = 1.1f;
            previewUtility.lights[0].transform.rotation = Quaternion.Euler(0f - settings.lightDir.y, 0f - settings.lightDir.x, 0f);
            previewUtility.lights[1].intensity = 1.1f;
            previewUtility.lights[1].transform.rotation = Quaternion.Euler(settings.lightDir.y, settings.lightDir.x, 0f);
            previewUtility.ambientColor = new Color(0.1f, 0.1f, 0.1f, 0f);
            RenderMeshPreviewSkipCameraAndLighting(mesh, bounds, previewUtility, settings, null, meshSubset);
        }
    }

    private static void DrawUVLayout(Mesh mesh, PreviewRenderUtility previewUtility, Settings settings)
    {
        GL.PushMatrix();
        settings.lineMaterial.SetPass(0);
        GL.LoadProjectionMatrix(previewUtility.camera.projectionMatrix);
        GL.MultMatrix(previewUtility.camera.worldToCameraMatrix);
        GL.Begin(1);
        for (float num = -2f; num <= 3f; num += 0.125f)
        {
            if (Mathf.Abs(num - Mathf.Round(num)) < 0.01f)
            {
                GL.Color(new Color(0.6f, 0.6f, 0.7f, 1f));
                GL.Vertex3(-2f, num, 0f);
                GL.Vertex3(3f, num, 0f);
                GL.Vertex3(num, -2f, 0f);
                GL.Vertex3(num, 3f, 0f);
            }
            else if (num >= 0f && num <= 1f)
            {
                GL.Color(new Color(0.6f, 0.6f, 0.7f, 0.5f));
                GL.Vertex3(0f, num, 0f);
                GL.Vertex3(1f, num, 0f);
                GL.Vertex3(num, 0f, 0f);
                GL.Vertex3(num, 1f, 0f);
            }
        }

        GL.End();
        GL.LoadIdentity();
        settings.meshMultiPreviewMaterial.SetPass(0);
        GL.wireframe = true;
        Graphics.DrawMeshNow(mesh, previewUtility.camera.worldToCameraMatrix);
        GL.wireframe = false;
        GL.PopMatrix();
    }

    internal static Color GetSubMeshTint(int index)
    {
        float h = Mathf.Repeat((float)index * 0.618f, 1f);
        float s = ((index == 0) ? 0f : 0.3f);
        float v = 1f;
        return Color.HSVToRGB(h, s, v);
    }

    internal static void RenderMeshPreviewSkipCameraAndLighting(Mesh mesh, Bounds bounds, PreviewRenderUtility previewUtility, Settings settings, MaterialPropertyBlock customProperties, int meshSubset)
    {
        if (mesh == null || previewUtility == null)
        {
            return;
        }

        Quaternion quaternion = Quaternion.Euler(settings.previewDir.y, 0f, 0f) * Quaternion.Euler(0f, settings.previewDir.x, 0f);
        Vector3 pos = quaternion * -bounds.center;
        bool fog = RenderSettings.fog;
        Unsupported.SetRenderSettingsUseFogNoDirty(fog: false);
        int subMeshCount = mesh.subMeshCount;
        bool flag = false;
        int nameID = 0;
        if (subMeshCount > 1 && settings.displayMode == DisplayMode.Shaded && customProperties == null && meshSubset == -1)
        {
            flag = true;
            customProperties = new MaterialPropertyBlock();
            nameID = Shader.PropertyToID("_Color");
        }

        if (settings.activeMaterial != null)
        {
            previewUtility.camera.clearFlags = CameraClearFlags.Nothing;
            if (meshSubset < 0 || meshSubset >= subMeshCount)
            {
                for (int i = 0; i < subMeshCount; i++)
                {
                    if (flag)
                    {
                        customProperties.SetColor(nameID, GetSubMeshTint(i));
                    }

                    previewUtility.DrawMesh(mesh, pos, quaternion, settings.activeMaterial, i, customProperties);
                }
            }
            else
            {
                previewUtility.DrawMesh(mesh, pos, quaternion, settings.activeMaterial, meshSubset, customProperties);
            }

            previewUtility.Render();
        }

        if (settings.wireMaterial != null && settings.drawWire)
        {
            previewUtility.camera.clearFlags = CameraClearFlags.Nothing;
            GL.wireframe = true;
            if (flag)
            {
                customProperties.SetColor(nameID, settings.wireMaterial.color);
            }

            if (meshSubset < 0 || meshSubset >= subMeshCount)
            {
                for (int j = 0; j < subMeshCount; j++)
                {
                    MeshTopology topology = mesh.GetTopology(j);
                    if (topology != MeshTopology.Lines && topology != MeshTopology.LineStrip && topology != MeshTopology.Points)
                    {
                        previewUtility.DrawMesh(mesh, pos, quaternion, settings.wireMaterial, j, customProperties);
                    }
                }
            }
            else
            {
                previewUtility.DrawMesh(mesh, pos, quaternion, settings.wireMaterial, meshSubset, customProperties);
            }

            previewUtility.Render();
            GL.wireframe = false;
        }

        Unsupported.SetRenderSettingsUseFogNoDirty(fog);
    }

    private static void SetTransformMatrix(Transform tr, Matrix4x4 mat)
    {
        Vector3 localPosition = new Vector3(mat.m03, mat.m13, mat.m23);
        Vector3 lossyScale = mat.lossyScale;
        Vector3 vector = new Vector3(1f / lossyScale.x, 1f / lossyScale.y, 1f / lossyScale.z);
        mat.m00 *= vector.x;
        mat.m10 *= vector.x;
        mat.m20 *= vector.x;
        mat.m01 *= vector.y;
        mat.m11 *= vector.y;
        mat.m21 *= vector.y;
        mat.m02 *= vector.z;
        mat.m12 *= vector.z;
        mat.m22 *= vector.z;
        Quaternion rotation = mat.rotation;
        tr.localPosition = localPosition;
        tr.localRotation = rotation;
        tr.localScale = lossyScale;
    }

    private void BakeSkinnedMesh()
    {
        if (mesh == null)
        {
            return;
        }

        GameObject gameObject = new GameObject
        {
            hideFlags = HideFlags.HideAndDontSave
        };
        SkinnedMeshRenderer skinnedMeshRenderer = gameObject.AddComponent<SkinnedMeshRenderer>();
        skinnedMeshRenderer.hideFlags = HideFlags.HideAndDontSave;
        m_BakedSkinnedMesh = new Mesh
        {
            hideFlags = HideFlags.HideAndDontSave
        };
        bool flag = mesh.blendShapeCount > 0 && mesh.bindposes.Length == 0;
        Transform[] array = new Transform[mesh.bindposes.Length];
        if (!flag)
        {
            for (int i = 0; i < array.Length; i++)
            {
                Matrix4x4 inverse = mesh.bindposes[i].inverse;
                array[i] = new GameObject().transform;
                array[i].gameObject.hideFlags = HideFlags.HideAndDontSave;
                SetTransformMatrix(array[i], inverse);
            }

            skinnedMeshRenderer.bones = array;
        }

        skinnedMeshRenderer.sharedMesh = mesh;
        skinnedMeshRenderer.SetBlendShapeWeight(m_Settings.activeBlendshape, 100f);
        skinnedMeshRenderer.BakeMesh(m_BakedSkinnedMesh);
        if (flag)
        {
            m_BakedSkinnedMesh.RecalculateBounds();
        }

        skinnedMeshRenderer.sharedMesh = null;
        UnityEngine.Object.DestroyImmediate(skinnedMeshRenderer);
        UnityEngine.Object.DestroyImmediate(gameObject);
        if (!flag)
        {
            for (int j = 0; j < array.Length; j++)
            {
                UnityEngine.Object.DestroyImmediate(array[j].gameObject);
            }
        }
    }

    //
    // Summary:
    //     Creates a texture preview to override Editor.RenderStaticPreview. The current
    //     mesh will be drawn.
    //
    // Parameters:
    //   width:
    //     The width to render the texture.
    //
    //   height:
    //     The height to render the texture.
    //
    // Returns:
    //     Returns a rendered texture of the current mesh with default settings.
    public Texture2D RenderStaticPreview(int width, int height)
    {
        if (!ShaderUtil.hardwareSupportsRectRenderTexture)
        {
            return null;
        }

        m_PreviewUtility.BeginStaticPreview(new Rect(0f, 0f, width, height));
        DoRenderPreview();
        return m_PreviewUtility.EndStaticPreview();
    }

    private void DoRenderPreview()
    {
        if (m_Settings.displayMode == DisplayMode.Blendshapes)
        {
            RenderMeshPreview(m_BakedSkinnedMesh, m_PreviewUtility, m_Settings, -1);
        }
        else
        {
            RenderMeshPreview(mesh, m_PreviewUtility, m_Settings, -1);
        }
    }

    //
    // Summary:
    //     Call this from an Editor.OnPreviewGUI or ObjectPreview.OnPreviewGUI to draw a
    //     mesh preview.
    //
    // Parameters:
    //   rect:
    //     Rectangle in which to draw the preview.
    //
    //   background:
    //     The background style.
    public void OnPreviewGUI(Rect rect, GUIStyle background)
    {
        Event current = Event.current;
        if (!ShaderUtil.hardwareSupportsRectRenderTexture)
        {
            if (current.type == EventType.Repaint)
            {
                EditorGUI.DropShadowLabel(new Rect(rect.x, rect.y, rect.width, 40f), "Mesh preview requires\nrender texture support");
            }

            return;
        }

        Vector2 Drag2D(Vector2 scrollPosition, Rect position)
        {
            int controlID = GUIUtility.GetControlID(sliderHash, FocusType.Passive);
            Event current = Event.current;
            switch (current.GetTypeForControl(controlID))
            {
                case EventType.MouseDown:
                    if (position.Contains(current.mousePosition) && position.width > 50f)
                    {
                        GUIUtility.hotControl = controlID;
                        //current.Use();
                        EditorGUIUtility.SetWantsMouseJumping(1);
                    }

                    break;
                case EventType.MouseDrag:
                    if (GUIUtility.hotControl == controlID)
                    {
                        scrollPosition -= current.delta * ((!current.shift) ? 1 : 3) / Mathf.Min(position.width, position.height) * 140f;
                        //current.Use();
                        GUI.changed = true;
                    }

                    break;
                case EventType.MouseUp:
                    if (GUIUtility.hotControl == controlID)
                    {
                        GUIUtility.hotControl = 0;
                    }

                    EditorGUIUtility.SetWantsMouseJumping(0);
                    break;
            }

            return scrollPosition;
        }

        if ((current.type == EventType.ValidateCommand || current.type == EventType.ExecuteCommand) && current.commandName == "FrameSelected")
        {
            FrameObject();
            //current.Use();
        }

        if (current.button <= 0 && m_Settings.displayMode != DisplayMode.UVLayout)
        {
            m_Settings.previewDir = Drag2D(m_Settings.previewDir, rect);
        }

        if (current.button == 1 && m_Settings.displayMode != DisplayMode.UVLayout)
        {
            m_Settings.lightDir = Drag2D(m_Settings.lightDir, rect);
        }

        if (current.type == EventType.ScrollWheel && rect.Contains(current.mousePosition))
        {
            MeshPreviewZoom(rect, current);
            GUI.changed = true;
        }

        if (current.type == EventType.MouseDrag && (m_Settings.displayMode == DisplayMode.UVLayout || current.button == 2)  && rect.Contains(current.mousePosition))
        {
            MeshPreviewPan(rect, current);
            GUI.changed = true;
        }

        if (current.type == EventType.Repaint)
        {
            m_PreviewUtility.BeginPreview(rect, background);
            DoRenderPreview();
            m_PreviewUtility.EndAndDrawPreview(rect);
        }
    }

    //
    // Summary:
    //     Call this from Editor.OnPreviewSettings or ObjectPreview.OnPreviewSettings to
    //     draw additional settings related to the mesh preview.
    public void OnPreviewSettings()
    {
        if (!ShaderUtil.hardwareSupportsRectRenderTexture)
        {
            return;
        }

        GUI.enabled = true;
        if (m_Settings.displayMode == DisplayMode.UVChecker)
        {
            int checkerTextureMultiplier = m_Settings.checkerTextureMultiplier;
            float x = EditorStyles.label.CalcSize(new GUIContent("--------")).x;
            Rect controlRect = EditorGUILayout.GetControlRect(GUILayout.Width(x));
            controlRect.x += 3f;
            m_Settings.checkerTextureMultiplier = (int)GUI.HorizontalSlider(controlRect, m_Settings.checkerTextureMultiplier, 30f, 1f, Styles.preSlider, Styles.preSliderThumb);
            if (checkerTextureMultiplier != m_Settings.checkerTextureMultiplier)
            {
                m_Settings.activeMaterial.mainTextureScale = new Vector2(m_Settings.checkerTextureMultiplier, m_Settings.checkerTextureMultiplier);
            }
        }

        if (m_Settings.displayMode == DisplayMode.UVLayout || m_Settings.displayMode == DisplayMode.UVChecker)
        {
            float x2 = EditorStyles.toolbarDropDown.CalcSize(new GUIContent("Channel 6")).x;
            Rect controlRect2 = EditorGUILayout.GetControlRect(GUILayout.Width(x2));
            controlRect2.y -= 1f;
            controlRect2.x += 5f;
            GUIContent content = new GUIContent("Channel " + m_Settings.activeUVChannel, Styles.uvChannelDropdown.tooltip);
            if (EditorGUI.DropdownButton(controlRect2, content, FocusType.Passive, EditorStyles.toolbarDropDown))
            {
                DoPopup(controlRect2, m_UVChannels, m_Settings.activeUVChannel, SetUVChannel, m_Settings.availableUVChannels);
            }
        }

        if (m_Settings.displayMode == DisplayMode.Blendshapes)
        {
            float x3 = EditorStyles.toolbarDropDown.CalcSize(new GUIContent("Blendshapes")).x;
            Rect controlRect3 = EditorGUILayout.GetControlRect(GUILayout.Width(x3));
            controlRect3.y -= 1f;
            controlRect3.x += 5f;
            GUIContent content2 = new GUIContent(m_BlendShapes[m_Settings.activeBlendshape], Styles.uvChannelDropdown.tooltip);
            if (EditorGUI.DropdownButton(controlRect3, content2, FocusType.Passive, EditorStyles.toolbarDropDown))
            {
                DoPopup(controlRect3, m_BlendShapes.ToArray(), m_Settings.activeBlendshape, SetBlendshape, null);
            }
        }

        float x4 = EditorStyles.toolbarDropDown.CalcSize(new GUIContent(m_DisplayModes[3])).x;
        Rect controlRect4 = EditorGUILayout.GetControlRect(GUILayout.Width(x4));
        controlRect4.y -= 1f;
        controlRect4.x += 2f;
        GUIContent content3 = new GUIContent(m_DisplayModes[(int)m_Settings.displayMode], Styles.displayModeDropdown.tooltip);
        if (EditorGUI.DropdownButton(controlRect4, content3, FocusType.Passive, EditorStyles.toolbarDropDown))
        {
            DoPopup(controlRect4, m_DisplayModes, (int)m_Settings.displayMode, SetDisplayMode, m_Settings.availableDisplayModes);
        }

        using (new EditorGUI.DisabledScope(m_Settings.displayMode == DisplayMode.UVLayout))
        {
            m_Settings.drawWire = GUILayout.Toggle(m_Settings.drawWire, Styles.wireframeToggle, EditorStyles.toolbarButton);
        }
    }

    private void OnDropDownAction(Material mat, int mode, bool flatUVs)
    {
        ResetView();
        m_Settings.activeMaterial = mat;
        m_Settings.activeMaterial.SetFloat("_Mode", mode);
        m_Settings.activeMaterial.SetFloat("_UVChannel", 0f);
        m_Settings.activeMaterial.SetFloat("_Cull", flatUVs ? 0f : 2f);
    }

    private void MeshPreviewZoom(Rect rect, Event evt)
    {
        float num = (0f - HandleUtility.niceMouseDeltaZoom * 0.5f) * 0.05f;
        float value = m_Settings.zoomFactor + m_Settings.zoomFactor * num;
        value = Mathf.Clamp(value, 0.1f, 10f);
        Vector2 vector = new Vector2(evt.mousePosition.x / rect.width, 1f - evt.mousePosition.y / rect.height);
        Vector3 vector2 = m_PreviewUtility.camera.ViewportToWorldPoint(vector);
        Vector3 vector3 = m_Settings.orthoPosition - vector2;
        Vector3 vector4 = vector2 + vector3 * (value / m_Settings.zoomFactor);
        if (m_Settings.displayMode != DisplayMode.UVLayout)
        {
            m_PreviewUtility.camera.transform.position = new Vector3(vector4.x, vector4.y, vector4.z);
        }
        else
        {
            m_Settings.orthoPosition = new Vector3(vector4.x, vector4.y, m_Settings.orthoPosition.z);
        }

        m_Settings.zoomFactor = value;
        //evt.Use();
    }

    private void MeshPreviewPan(Rect rect, Event evt)
    {
        Camera camera = m_PreviewUtility.camera;
        Vector3 vector = new Vector3((0f - evt.delta.x) * (float)camera.pixelWidth / rect.width, evt.delta.y * (float)camera.pixelHeight / rect.height, 0f);
        if (m_Settings.displayMode == DisplayMode.UVLayout)
        {
            Vector3 position = camera.WorldToScreenPoint(m_Settings.orthoPosition);
            position += vector;
            Vector3 vector2 = camera.ScreenToWorldPoint(position);
            m_Settings.orthoPosition = new Vector3(vector2.x, vector2.y, m_Settings.orthoPosition.z);
        }
        else
        {
            Vector3 position = camera.WorldToScreenPoint(m_Settings.pivotPositionOffset);
            position += vector;
            Vector3 vector2 = camera.ScreenToWorldPoint(position) - m_Settings.pivotPositionOffset;
            m_Settings.pivotPositionOffset += vector2;
        }

        //evt.Use();
    }
}
