using UnityEngine;

[ExecuteAlways]
public class TriangleDrawer : MonoBehaviour
{
    public MeshFilter m_meshFiler;
    public Transform p0,p1,p2;

    private void Update()
    {
        m_meshFiler.mesh.vertices = new Vector3[]{p0.position,p1.position,p2.position,p0.position,p2.position,p1.position};
        m_meshFiler.mesh.triangles = new int[]{0,1,2,3,4,5};
        m_meshFiler.mesh.RecalculateBounds();
        m_meshFiler.mesh.RecalculateNormals();
        m_meshFiler.mesh.RecalculateTangents();
    }
}
