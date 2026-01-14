

namespace Hanzzz.Tetrahedralizer
{
    //#if UNITY_EDITOR
    
    //using UnityEngine;
    //using UnityEditor;
    //using System;
    //using System.Text;
    
    //[CustomEditor(typeof(MeshFilter))]
    //public class MeshTriangleInspector : Editor
    //{
    //    private void OnSceneGUI()
    //    {
    //        Event e = Event.current;
    //        if (e.type != EventType.MouseDown || e.button != 0)
    //        {
    //            return;
    //        }
    
    //        Ray ray = HandleUtility.GUIPointToWorldRay(e.mousePosition);
    //        MeshFilter meshFilter = (MeshFilter)target;
    
    //        if(!Physics.Raycast(ray, out RaycastHit hit) || null == hit.collider || hit.collider.gameObject != meshFilter.gameObject)
    //        {
    //            return;
    //        }
    
    //        Mesh mesh = meshFilter.sharedMesh;
    //        int[] triangles = mesh.triangles;
    //        Vector3[] vertices = mesh.vertices;
    //        Vector2[] uv0s = mesh.uv;
    
    //        int triIndex = hit.triangleIndex;
    
    //        // Each triangle is 3 consecutive indices in mesh.triangles
    //        int i0 = triangles[triIndex * 3 + 0];
    //        int i1 = triangles[triIndex * 3 + 1];
    //        int i2 = triangles[triIndex * 3 + 2];
    
    
    //        string GetVector3Bits(Vector3 vec)
    //        {
    //            return $"{BitConverter.SingleToInt32Bits(vec.x):X8} {BitConverter.SingleToInt32Bits(vec.y):X8} {BitConverter.SingleToInt32Bits(vec.z):X8}";
    //        }
    //        string GetVector3Exact(Vector3 vec) // exact as in keeping the minimal number of digits such that the binary representation stays the same
    //        {
    //            return $"{vec.x:R} {vec.y:R} {vec.z:R}";
    //        }
    //        string GetVector2Exact(Vector2 vec) // exact as in keeping the minimal number of digits such that the binary representation stays the same
    //        {
    //            return $"{vec.x:R} {vec.y:R}";
    //        }
    //        void CreateInfo(int id, int index)
    //        {
    //            GameObject gameObject = GameObject.Find(nameof(MeshTriangleInspector) + id.ToString());
    //            if(null == gameObject)
    //            {
    //                gameObject = new GameObject();
    //                gameObject.name = nameof(MeshTriangleInspector) + id.ToString();
    //            }
    
    //            gameObject.transform.position = vertices[index];
    
    //            StringBuilder stringBuilder = new StringBuilder();
    //            stringBuilder.AppendLine($"Vertex index: {index}");
    //            stringBuilder.AppendLine($"Vertex Position: {GetVector3Bits(vertices[index])}");
    //            stringBuilder.AppendLine($"Vertex Position: {GetVector3Exact(vertices[index])}");
    //            //stringBuilder.AppendLine($"UV0: {GetVector2Exact(uv0s[index])}");
    
    
    //            StringDisplay stringDisplay = gameObject.GetComponent<StringDisplay>();
    //            if(null == stringDisplay)
    //            {
    //                stringDisplay = gameObject.AddComponent<StringDisplay>();
    //            }
    //            stringDisplay.m_stringData = stringBuilder.ToString();
    //        }
    
    //        //CreateInfo(0, i0);
    //        //CreateInfo(1, i1);
    //        //CreateInfo(2, i2);
    
    //        for(int i=0; i<triangles.Length; i++)
    //        {
    //            CreateInfo(i,triangles[i]);
    //        }
    //    }
    //}
    
    //#endif
}