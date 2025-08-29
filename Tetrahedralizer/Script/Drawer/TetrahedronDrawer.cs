using UnityEngine;
using System;
using System.Linq;

[ExecuteAlways]
public class TetrahedronDrawer : MonoBehaviour
{
    public MeshFilter m_meshFiler;
    public Transform p0,p1,p2,p3;

    private void Update()
    {
        Vector3 v0 = p0.position;
        Vector3 v1 = p1.position;
        Vector3 v2 = p2.position;
        Vector3 v3 = p3.position;
    
        m_meshFiler.mesh.vertices = new Vector3[]{v0,v2,v1, v0,v1,v3, v0,v3,v2, v1,v2,v3};
        m_meshFiler.mesh.triangles = Enumerable.Range(0,12).ToArray();
        m_meshFiler.mesh.RecalculateBounds();
        m_meshFiler.mesh.RecalculateNormals();
        m_meshFiler.mesh.RecalculateTangents();
    }
}
