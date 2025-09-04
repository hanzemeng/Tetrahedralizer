using System.Linq;
using System.Collections.Generic;
using UnityEngine;

public class DelaunayTetrahedralizationTest : MonoBehaviour
{
    [SerializeField] private Transform m_verticesParent;
    [SerializeField] private TetrahedralizationDrawer m_tetrahedralizationDrawer;


    [ContextMenu("Test")]
    public void Test()
    {
        List<Vector3> vertices = new List<Vector3>();
        foreach(Transform t in m_verticesParent)
        {
            vertices.Add(t.position);
        }

        DelaunayTetrahedralization.DelaunayTetrahedralizationInput input = new DelaunayTetrahedralization.DelaunayTetrahedralizationInput();
        input.m_explicitVertices = TetrahedralizerUtility.UnpackVector3s(vertices);
        DelaunayTetrahedralization.DelaunayTetrahedralizationOutput output = new DelaunayTetrahedralization.DelaunayTetrahedralizationOutput();
        DelaunayTetrahedralization delaunayTetrahedralization = new DelaunayTetrahedralization();

        delaunayTetrahedralization.CalculateDelaunayTetrahedralization(input, output);


        Tetrahedralization tetrahedralization = ScriptableObject.CreateInstance<Tetrahedralization>();
        tetrahedralization.m_explicitVertices = input.m_explicitVertices as List<double>;
        tetrahedralization.m_implicitVertices = null;
        tetrahedralization.m_tetrahedrons = output.m_tetrahedrons;
        m_tetrahedralizationDrawer.Draw(tetrahedralization);
    }

    [ContextMenu("Clear")]
    public void Clear()
    {
        m_tetrahedralizationDrawer.Clear();
    }
}
