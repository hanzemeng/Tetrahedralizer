

namespace Hanzzz.Tetrahedralizer
{
    using UnityEngine;
    
    [ExecuteAlways]
    public class TriangleDrawer : MonoBehaviour
    {
        public MeshFilter m_meshFiler;
        public Transform m_p0,m_p1,m_p2;
    
    
        private void Update()
        {
            Mesh mesh = new Mesh();
            mesh.vertices = new Vector3[]{m_p0.position,m_p1.position,m_p2.position,m_p0.position,m_p2.position,m_p1.position};
            mesh.triangles = new int[]{0,1,2,3,4,5}; 
            mesh.RecalculateBounds();
            mesh.RecalculateNormals();
            mesh.RecalculateTangents();
            m_meshFiler.mesh = mesh;
        }
    }
    
}