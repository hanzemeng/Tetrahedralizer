//using System.Linq;
//using System.Collections.Generic;
//using UnityEngine;
//#if UNITY_EDITOR
//using UnityEditor;
//#endif

//[CreateAssetMenu(fileName = SHORT_NAME, menuName = TetrahedralizerConstant.SCRIPTABLE_OBJECT_PATH + SHORT_NAME)]
//[PreferBinarySerialization]
//public class Tetrahedralization : ScriptableObject
//{
//    public const string SHORT_NAME = "Tetrahedralization_SO";

//    public List<double> m_explicitVertices; // Every 3 doubles are x,y,z of a point. Assuming left hand coordinate.
//    public List<int> m_implicitVertices; // 5/9 followed by indexes of m_explicitVertices
//    public List<int> m_tetrahedrons; // Every 4 ints is a tetrahedron. Curl around the first 3 points and your thumb points toward the 4th point. Assuming left hand coordinate.


//    // each element is a tetrahedron, and the center of the tetrahedron in world space
//    public List<(Mesh mesh, Vector3 center)> ToMeshes()
//    {
//        List<(Mesh, Vector3)> res = new List<(Mesh, Vector3)>();
//        int[] zeroToEleven = Enumerable.Range(0,12).ToArray();
//        List<Vector3> vertices = TetrahedralizerUtility.PackVector3s(GenericPointApproximation.CalculateGenericPointApproximation(m_explicitVertices, m_implicitVertices));
//        for(int i=0; i<m_tetrahedrons.Count; i+=4)
//        {
//            Vector3 v0 = vertices[m_tetrahedrons[i+0]];
//            Vector3 v1 = vertices[m_tetrahedrons[i+1]];
//            Vector3 v2 = vertices[m_tetrahedrons[i+2]];
//            Vector3 v3 = vertices[m_tetrahedrons[i+3]];
//            Vector3[] meshVertices = new Vector3[]{v0,v2,v1, v0,v1,v3, v0,v3,v2, v1,v2,v3};
//            Vector3 center = TetrahedralizerUtility.CenterVertices(meshVertices);

//            Mesh mesh = new Mesh();
//            mesh.vertices = meshVertices;
//            mesh.triangles = zeroToEleven;
//            mesh.RecalculateBounds();
//            mesh.RecalculateNormals();
//            mesh.RecalculateTangents();

//            res.Add((mesh, center));
//        }

//        return res;
//    }
//    public (Mesh mesh, Vector3 center) ToMesh()
//    {
//        List<Vector3> vertices = TetrahedralizerUtility.PackVector3s(GenericPointApproximation.CalculateGenericPointApproximation(m_explicitVertices, m_implicitVertices));
//        List<Vector3> meshVertices = new List<Vector3>(3*m_tetrahedrons.Count);

//        for(int i=0; i<m_tetrahedrons.Count; i+=4)
//        {
//            Vector3 v0 = vertices[m_tetrahedrons[i+0]];
//            Vector3 v1 = vertices[m_tetrahedrons[i+1]];
//            Vector3 v2 = vertices[m_tetrahedrons[i+2]];
//            Vector3 v3 = vertices[m_tetrahedrons[i+3]];

//            meshVertices.Add(v0,v2,v1, v0,v1,v3, v0,v3,v2, v1,v2,v3);
//        }

//        Vector3 center = TetrahedralizerUtility.CenterVertices(meshVertices);
//        Mesh mesh = new Mesh();
//        mesh.indexFormat = UnityEngine.Rendering.IndexFormat.UInt32;
//        mesh.vertices = meshVertices.ToArray();
//        mesh.triangles = Enumerable.Range(0,meshVertices.Count).ToArray();
//        mesh.RecalculateBounds();
//        mesh.RecalculateNormals();
//        mesh.RecalculateTangents();

//        return (mesh, center);
//    }

//    public void Assign(Tetrahedralization tetrahedralization)
//    {
//        m_explicitVertices = tetrahedralization.m_explicitVertices;
//        m_implicitVertices = tetrahedralization.m_implicitVertices;
//        m_tetrahedrons = tetrahedralization.m_tetrahedrons;
//    }

//    public int GetVerticesCount()
//    {
//        return m_explicitVertices.Count/3 + TetrahedralizerUtility.CountFlatIListElements(m_implicitVertices);
//    }
//    public int GetTetrahedronsCount()
//    {
//        return m_tetrahedrons.Count / 4;
//    }
//}


//#if UNITY_EDITOR

//[CustomEditor(typeof(Tetrahedralization))]
//public class TetrahedralizationEditor : Editor
//{
//    private Tetrahedralization m_so;


//    private void OnEnable()
//    {
//        m_so = (Tetrahedralization)target;
//    }

//    public override void OnInspectorGUI()
//    {
//        //base.OnInspectorGUI();

//        if(null == m_so.m_explicitVertices)
//        {
//            EditorGUILayout.LabelField($"Vertices Count: {0}");
//            EditorGUILayout.LabelField($"Tetrahedrons Count: {0}");
//            return;
//        }

//        EditorGUILayout.LabelField($"Vertices Count: {m_so.GetVerticesCount()}");
//        EditorGUILayout.LabelField($"Tetrahedrons Count: {m_so.GetTetrahedronsCount()}");
//    }
//}

//#endif
