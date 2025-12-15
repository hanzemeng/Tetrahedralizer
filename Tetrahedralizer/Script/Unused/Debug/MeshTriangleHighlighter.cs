using System.Collections.Generic;
using UnityEngine;

[ExecuteInEditMode]
public class MeshTriangleHighlighter : MonoBehaviour
{
    [SerializeField] private MeshFilter m_meshFilter;
    [SerializeField] private int m_triangleIndex;
    [SerializeField] private Transform m_p0;
    [SerializeField] private Transform m_p1;
    [SerializeField] private Transform m_p2;

    private Vector3[] m_meshVertices;
    private int[] m_meshTriangles;

    [ContextMenu("Load Mesh")]
    public void Load()
    {
        Mesh mesh = m_meshFilter.sharedMesh;
        m_meshVertices = mesh.vertices;
        m_meshTriangles = mesh.triangles;
    }

    
    void Update()
    {
        m_p0.position = m_meshVertices[m_meshTriangles[3*m_triangleIndex]+0];
        m_p1.position = m_meshVertices[m_meshTriangles[3*m_triangleIndex]+1];
        m_p2.position = m_meshVertices[m_meshTriangles[3*m_triangleIndex]+02];
    }
}
