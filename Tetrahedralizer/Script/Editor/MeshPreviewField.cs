namespace Hanzzz.Tetrahedralizer
{
    #if UNITY_EDITOR

    using System.Collections.Generic;
    using UnityEngine;
    using UnityEngine.SceneManagement;
    using UnityEditor;
    using UnityEditor.SceneManagement;
    
    public class MeshPreviewField
    {
        private Scene m_previewScene;
        private Transform m_previewParent;
    
        private Camera m_camera;
        private Light m_directionalLight0;
        private Light m_directionalLight1;
        private RenderTexture m_renderTexture;
        private MeshFilter m_meshFilter;
        private MeshRenderer m_meshRenderer;
    
        private float m_zoomFactor;
        private Vector2 m_previewDirection;
        private Vector2 m_lightDirection;
        private Vector3 m_pivotPositionOffset;
        private Vector3 m_orthoPosition;
    
        public MeshPreviewField()
        {
            m_previewScene = EditorSceneManager.NewPreviewScene();
    
            m_previewParent = new GameObject().transform;
            m_previewParent.gameObject.hideFlags = HideFlags.HideAndDontSave;
            SceneManager.MoveGameObjectToScene(m_previewParent.gameObject, m_previewScene);
    
            m_renderTexture = new RenderTexture(1024,1024,32);
            m_renderTexture.hideFlags = HideFlags.HideAndDontSave;
    
            m_camera = new GameObject().AddComponent<Camera>();
            m_camera.targetTexture = m_renderTexture;
            m_camera.overrideSceneCullingMask = EditorSceneManager.GetSceneCullingMask(m_previewScene);
            m_camera.nearClipPlane = 0.0001f;
            m_camera.farClipPlane = 1000f;
            m_camera.orthographic = false;
            m_camera.clearFlags = CameraClearFlags.SolidColor;
            m_camera.backgroundColor = new Color(49f/255f,49f/255f,49f/255f);
    
            m_directionalLight0 = new GameObject().AddComponent<Light>();
            m_directionalLight0.type = LightType.Directional;
            m_directionalLight0.intensity = 1.0f;
    
            m_directionalLight1 = new GameObject().AddComponent<Light>();
            m_directionalLight1.type = LightType.Directional;
            m_directionalLight1.intensity = 1.0f;
    
            m_meshFilter = new GameObject().AddComponent<MeshFilter>();
            m_meshRenderer = m_meshFilter.gameObject.AddComponent<MeshRenderer>();
    
            ResetPreviewParameters();
    
            m_camera.transform.SetParent(m_previewParent);
            m_directionalLight0.transform.SetParent(m_previewParent);
            m_directionalLight1.transform.SetParent(m_previewParent);
            m_meshFilter.transform.SetParent(m_previewParent);
        }
    
        public void Dispose()
        {
            if(null != m_previewParent)
            {
                Object.DestroyImmediate(m_previewParent.gameObject);
            }
            if(null != m_renderTexture)
            {
                m_renderTexture.Release();
                Object.DestroyImmediate(m_renderTexture);
                m_renderTexture = null;
            }
            EditorSceneManager.ClosePreviewScene(m_previewScene);
        }
    
        public void ResetPreviewParameters()
        {
            m_zoomFactor = 0.75f;
            m_previewDirection = new Vector2(130f,0f);
            m_lightDirection = new Vector2(-40f,-40f);
            m_pivotPositionOffset = Vector3.zero;
            m_orthoPosition = new Vector3(0.5f, 0.5f, -1f);
            GUI.changed = true;
        }
        public void CopyPreviewParameters(MeshPreviewField meshPreviewField)
        {
            m_zoomFactor = meshPreviewField.m_zoomFactor;
            m_previewDirection = meshPreviewField.m_previewDirection;
            m_lightDirection = meshPreviewField.m_lightDirection;
            m_pivotPositionOffset = meshPreviewField.m_pivotPositionOffset;
            m_orthoPosition = meshPreviewField.m_orthoPosition;
            GUI.changed = true;
        }
    
        public void AssignMesh(Mesh mesh)
        {
            if(null == mesh)
            {
                mesh = MeshUtility.GetEmptyMesh();
            }
            m_meshFilter.sharedMesh = mesh;
        }
        public void AssignMaterials(List<Material> materials)
        {
            m_meshRenderer.SetSharedMaterials(materials);
        }
    
        public void DrawOnGUI(Rect rect)
        {
            Event current = Event.current;
            if(current.type == EventType.Repaint)
            {
                ChangeTransforms();
                if(!m_renderTexture.IsCreated())
                {
                    m_renderTexture.Create();
                }
                m_camera.Render();
                GUI.DrawTexture(rect, m_renderTexture, ScaleMode.ScaleToFit);
            }
    
            if(current.type == EventType.ScrollWheel && rect.Contains(current.mousePosition))
            {
                ChangeCameraZoom(rect, current);
                GUI.changed = true;
                current.Use();
            }
    
            int controlID = GUIUtility.GetControlID(FocusType.Passive, rect);
            if(current.type == EventType.MouseDown && rect.Contains(current.mousePosition))
            {
                GUIUtility.hotControl = controlID;
            }
            if(GUIUtility.hotControl == controlID)
            {
                if(current.type == EventType.MouseUp)
                {
                    GUIUtility.hotControl = 0;
                }
                else if(current.type == EventType.MouseDrag)
                {
                    if(current.button == 0)
                    {
                        m_previewDirection = Drag2D(m_previewDirection, rect, current);
                    }
                    else if(current.button == 1)
                    {
                        m_lightDirection = Drag2D(m_lightDirection, rect, current);
                    }
                    else if(current.button == 2)
                    {
                        ChangeCameraPivot(rect, current);
                    }
                    GUI.changed = true;
                    current.Use();
                }
            }
        }
    
        private void ChangeTransforms()
        {
            Bounds bounds = m_meshFilter.sharedMesh.bounds;
    
            float magnitude = bounds.extents.magnitude;
            float num = 4f * magnitude;
            Vector3 position = Quaternion.identity * Vector3.forward * (-num * m_zoomFactor) + m_pivotPositionOffset;
            m_camera.transform.position = position;
            m_camera.transform.rotation = Quaternion.identity;
    
            Quaternion quaternion = Quaternion.Euler(m_previewDirection.y, 0f, 0f) * Quaternion.Euler(0f, m_previewDirection.x, 0f);
    
            m_meshFilter.transform.position = quaternion * -bounds.center;
            m_meshFilter.transform.rotation = quaternion;
    
            m_directionalLight0.transform.rotation = Quaternion.Euler(-m_lightDirection.y, -m_lightDirection.x, 0f);
            m_directionalLight1.transform.rotation = Quaternion.Euler(m_lightDirection.y, m_lightDirection.x, 0f);
            //previewUtility.ambientColor = new Color(0.1f, 0.1f, 0.1f, 0f);
        }
        private Vector2 Drag2D(Vector2 scrollPosition, Rect position, Event current)
        {
            return scrollPosition - current.delta * ((!current.shift) ? 1 : 3) / Mathf.Min(position.width, position.height) * 140f;;
        }
        private void ChangeCameraZoom(Rect rect, Event evt)
        {
            float num = (0f - HandleUtility.niceMouseDeltaZoom * 0.5f) * 0.05f;
            float value = m_zoomFactor + m_zoomFactor * num;
            value = Mathf.Clamp(value, 0.1f, 10f);
            Vector2 vector = new Vector2(evt.mousePosition.x / rect.width, 1f - evt.mousePosition.y / rect.height);
            Vector3 vector2 = m_camera.ViewportToWorldPoint(vector);
            Vector3 vector3 = m_orthoPosition - vector2;
            Vector3 vector4 = vector2 + vector3 * (value / m_zoomFactor);
            m_orthoPosition = new Vector3(vector4.x, vector4.y, m_orthoPosition.z);
            m_zoomFactor = value;
        }
        private void ChangeCameraPivot(Rect rect, Event evt)
        {
            Vector3 vector = new Vector3((0f - evt.delta.x) * (float)m_camera.pixelWidth / rect.width, evt.delta.y * (float)m_camera.pixelHeight / rect.height, 0f);
            Vector3 position = m_camera.WorldToScreenPoint(m_pivotPositionOffset);
            position += vector;
            Vector3 vector2 = m_camera.ScreenToWorldPoint(position) - m_pivotPositionOffset;
            m_pivotPositionOffset += vector2;
        }
    }
    
    #endif
    
}