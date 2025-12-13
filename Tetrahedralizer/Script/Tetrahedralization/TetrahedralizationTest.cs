using System.Collections.Generic;
using System.Linq;
using UnityEngine;
using Hanzzz.Tetrahedralizer;

public class TetrahedralizationTest : MonoBehaviour
{
    [SerializeField] private MeshFilter m_meshFilter;
    [SerializeField] private TetrahedralizationDrawer m_tetrahedralizationDrawer;


    [ContextMenu("test")]
    public void Test()
    {
        Mesh mesh = m_meshFilter.sharedMesh;
        List<Vector3> weldedVertices = mesh.vertices.ToList();

        
        int[] weldedTriangles = mesh.triangles;
        TetrahedralizerUtility.RemoveDuplicateVertices(weldedVertices, weldedTriangles);

        //foreach(Vector3 vector3 in weldedVertices)
        //{
        //    Transform t = GameObject.CreatePrimitive(PrimitiveType.Sphere).transform;
        //    t.SetParent(transform);
        //    t.position = vector3;
        //    t.localScale = 0.1f*Vector3.one;
        //}

        DelaunayTetrahedralization delaunayTetrahedralization = new DelaunayTetrahedralization();

        List<double> vertices = TetrahedralizerUtility.UnpackVector3s(weldedVertices);
        List<int> tetrahedrons = delaunayTetrahedralization.CalculateDelaunayTetrahedralization(vertices, null);

        Tetrahedralization tetrahedralization = ScriptableObject.CreateInstance<Tetrahedralization>();
        tetrahedralization.m_explicitVertices = vertices;
        tetrahedralization.m_implicitVertices = null;
        tetrahedralization.m_tetrahedrons = tetrahedrons;

        m_tetrahedralizationDrawer.Draw(tetrahedralization);
    }
}
