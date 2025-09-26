using System.Linq;
using System.Collections.Generic;
using UnityEngine;
#if UNITY_EDITOR
using UnityEditor;
#endif

[CreateAssetMenu(fileName = SHORT_NAME, menuName = TetrahedralizerConstant.SCRIPTABLE_OBJECT_PATH + SHORT_NAME)]
[PreferBinarySerialization]
public class Polyhedralization : ScriptableObject
{
    public const string SHORT_NAME = "Polyhedralization_SO";

    public List<double> m_explicitVertices; // Every 3 doubles are x,y,z of a point. Assuming left hand coordinate.
    public List<int> m_implicitVertices; // 5/9 followed by indexes of m_explicitVertices.
    public List<int> m_polyhedrons; // # of polyhedron facets, followed by facets indexes.
    public List<int> m_polyhedronsFacets;  // # of facets vertices, followed by vertices indexes ordered in cw or ccw.


    public List<(Mesh mesh, Vector3 center)> ToMeshes()
    {
        if(null == m_explicitVertices || 0 == m_explicitVertices.Count)
        {
            return null;
        }

        List<Vector3> vertices = TetrahedralizerUtility.PackDoubles(GenericPointApproximation.CalculateGenericPointApproximation(m_explicitVertices, m_implicitVertices));
        List<List<int>> polyhedrons = TetrahedralizerUtility.FlatIListToNestedList(m_polyhedrons);
        List<List<int>> polyhedronsFacets = TetrahedralizerUtility.FlatIListToNestedList(m_polyhedronsFacets);

        List<(Mesh, Vector3)> res = new List<(Mesh, Vector3)>();

        List<Vector3> polyVertices = new List<Vector3>();
        List<int> polyTriangles = new List<int>();
        int triangleIndex;
        for(int i=0; i<polyhedrons.Count; i++)
        {
            polyVertices.Clear();
            polyTriangles.Clear();
            triangleIndex = 0;

            for(int j=0; j<polyhedrons[i].Count; j++)
            {
                int facet = polyhedrons[i][j];
                for(int k=0; k<polyhedronsFacets[facet].Count; k++)
                {
                    polyVertices.Add(vertices[polyhedronsFacets[facet][k]]);
                }
                for(int k=1; k<polyhedronsFacets[facet].Count-1; k++)
                {
                    polyTriangles.Add(triangleIndex);
                    polyTriangles.Add(triangleIndex+k);
                    polyTriangles.Add(triangleIndex+k+1);
                }
                triangleIndex += polyhedronsFacets[facet].Count;
                for(int k=0; k<polyhedronsFacets[facet].Count; k++)
                {
                    polyVertices.Add(vertices[polyhedronsFacets[facet][k]]);
                }
                for(int k=1; k<polyhedronsFacets[facet].Count-1; k++)
                {
                    polyTriangles.Add(triangleIndex);
                    polyTriangles.Add(triangleIndex+k+1);
                    polyTriangles.Add(triangleIndex+k);
                }
                triangleIndex += polyhedronsFacets[facet].Count;
            }

            Vector3 center = TetrahedralizerUtility.CenterVertices(polyVertices);
            Mesh mesh = new Mesh();
            mesh.vertices = polyVertices.ToArray();
            mesh.triangles = polyTriangles.ToArray();
            mesh.RecalculateBounds();
            mesh.RecalculateNormals();
            mesh.RecalculateTangents();

            res.Add((mesh,center));
        }

        return res;
    }
    public (Mesh mesh, Vector3 center) ToMesh(bool drawInteriorFacets=false)
    {
        if(null == m_explicitVertices || 0 == m_explicitVertices.Count)
        {
            return (null, Vector3.zero);
        }

        List<Vector3> vertices = TetrahedralizerUtility.PackDoubles(GenericPointApproximation.CalculateGenericPointApproximation(m_explicitVertices, m_implicitVertices));
        List<List<int>> polyhedronsFacets = TetrahedralizerUtility.FlatIListToNestedList(m_polyhedronsFacets);
        if(!drawInteriorFacets)
        {
            polyhedronsFacets = GetExteriorFacets().Select(i=>polyhedronsFacets[i]).ToList();
        }

        List<Vector3> meshVertices = new List<Vector3>();
        List<int> polyTriangles = new List<int>();
        int triangleIndex = 0;
        foreach(List<int> facet in polyhedronsFacets)
        {
            for(int i=0; i<facet.Count; i++)
            {
                meshVertices.Add(vertices[facet[i]]);
            }
            for(int i=1; i<facet.Count-1; i++)
            {
                polyTriangles.Add(triangleIndex);
                polyTriangles.Add(triangleIndex+i);
                polyTriangles.Add(triangleIndex+i+1);
            }
            triangleIndex += facet.Count;
            for(int i=0; i<facet.Count; i++)
            {
                meshVertices.Add(vertices[facet[i]]);
            }
            for(int i=1; i<facet.Count-1; i++)
            {
                polyTriangles.Add(triangleIndex);
                polyTriangles.Add(triangleIndex+i+1);
                polyTriangles.Add(triangleIndex+i);
            }
            triangleIndex += facet.Count;
        }


        Vector3 center = TetrahedralizerUtility.CenterVertices(meshVertices);
        Mesh mesh = new Mesh();
        mesh.indexFormat = UnityEngine.Rendering.IndexFormat.UInt32;
        mesh.vertices = meshVertices.ToArray();
        mesh.triangles = polyTriangles.ToArray();
        mesh.RecalculateBounds();
        mesh.RecalculateNormals();
        mesh.RecalculateTangents();

        return (mesh, center);
    }

    public List<int> GetExteriorFacets()
    {
        HashSet<int> facets = new HashSet<int>();
        List<List<int>> polyhedrons = TetrahedralizerUtility.FlatIListToNestedList(m_polyhedrons);
        foreach(List<int> ints in polyhedrons)
        {
            foreach(int i in ints)
            {
                if(facets.Contains(i))
                {
                    facets.Remove(i);
                }
                else
                {
                    facets.Add(i);
                }
            }
        }

        return facets.ToList();
    }

    public void Assign(Polyhedralization polyhedralization)
    {
        m_explicitVertices = polyhedralization.m_explicitVertices;
        m_implicitVertices = polyhedralization.m_implicitVertices;
        m_polyhedrons = polyhedralization.m_polyhedrons;
        m_polyhedronsFacets = polyhedralization.m_polyhedronsFacets;
    }

    public int GetVerticesCount()
    {
        return m_explicitVertices.Count/3 + TetrahedralizerUtility.CountFlatIListElements(m_implicitVertices);
    }
    public int GetPolyhedronsCount()
    {
        return m_polyhedrons.Count;
    }
}


#if UNITY_EDITOR

[CustomEditor(typeof(Polyhedralization))]
public class PolyhedralizationEditor : Editor
{
    private Polyhedralization m_so;


    private void OnEnable()
    {
        m_so = (Polyhedralization)target;
    }

    public override void OnInspectorGUI()
    {
        //base.OnInspectorGUI();

        if(null == m_so.m_explicitVertices)
        {
            EditorGUILayout.LabelField($"Vertices Count: {0}");
            EditorGUILayout.LabelField($"Polyhedrons Count: {0}");
            EditorGUILayout.LabelField($"Polyhedrons' Facets Count: {0}");
            return;
        }

        EditorGUILayout.LabelField($"Vertices Count: {m_so.m_explicitVertices.Count/3 + TetrahedralizerUtility.CountFlatIListElements(m_so.m_implicitVertices)}");
        EditorGUILayout.LabelField($"Polyhedrons Count: {m_so.m_polyhedrons.Count}");
        EditorGUILayout.LabelField($"Polyhedrons' Facets Count: {m_so.m_polyhedronsFacets.Count}");
    }
}

#endif
