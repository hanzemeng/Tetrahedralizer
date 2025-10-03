using System;
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
    public List<int> m_polyhedronsFacets;  // # of facets vertices, followed by vertices indexes ordered in cw or ccw. No collinear segments within the same facet.
    public List<bool> m_polyhedronsFacetsPointOut;  // only defined for exterior facets, true if the facet points out of its polyhedron


    public List<(Mesh mesh, Vector3 center)> ToMeshes()
    {
        if(null == m_explicitVertices || 0 == m_explicitVertices.Count)
        {
            return null;
        }

        using GenericPointPredicate genericPointPredicate = new GenericPointPredicate(m_explicitVertices,m_implicitVertices);
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
                if(FacetPointsOut(facet, i, polyhedronsFacets, polyhedrons, genericPointPredicate))
                {
                    for(int k=1; k<polyhedronsFacets[facet].Count-1; k++)
                    {
                        polyTriangles.Add(triangleIndex);
                        polyTriangles.Add(triangleIndex+k);
                        polyTriangles.Add(triangleIndex+k+1);
                    }
                }
                else
                {
                    for(int k=1; k<polyhedronsFacets[facet].Count-1; k++)
                    {
                        polyTriangles.Add(triangleIndex+k+1);
                        polyTriangles.Add(triangleIndex+k);
                        polyTriangles.Add(triangleIndex);
                    }
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
    public (Mesh mesh, Vector3 center) ToMesh()
    {
        if(null == m_explicitVertices || 0 == m_explicitVertices.Count)
        {
            return (null, Vector3.zero);
        }

        List<Vector3> vertices = TetrahedralizerUtility.PackDoubles(GenericPointApproximation.CalculateGenericPointApproximation(m_explicitVertices, m_implicitVertices));
        List<List<int>> polyhedrons = TetrahedralizerUtility.FlatIListToNestedList(m_polyhedrons);
        List<List<int>> polyhedronsFacets = TetrahedralizerUtility.FlatIListToNestedList(m_polyhedronsFacets);
        bool[] shouldDrawFacets = GetFacetsExteriorFlags();

        List<Vector3> meshVertices = new List<Vector3>();
        List<int> polyTriangles = new List<int>();
        int triangleIndex = 0;
        for(int i=0; i<polyhedrons.Count; i++)
        {
            for(int j=0; j<polyhedrons[i].Count; j++)
            {
                int facet = polyhedrons[i][j];
                if(!shouldDrawFacets[facet])
                {
                    continue;
                }

                for(int k=0; k<polyhedronsFacets[facet].Count; k++)
                {
                    meshVertices.Add(vertices[polyhedronsFacets[facet][k]]);
                }
                if(m_polyhedronsFacetsPointOut[facet])
                {
                    for(int k=1; k<polyhedronsFacets[facet].Count-1; k++)
                    {
                        polyTriangles.Add(triangleIndex);
                        polyTriangles.Add(triangleIndex+k);
                        polyTriangles.Add(triangleIndex+k+1);
                    }
                }
                else
                {
                    for(int k=1; k<polyhedronsFacets[facet].Count-1; k++)
                    {
                        polyTriangles.Add(triangleIndex+k+1);
                        polyTriangles.Add(triangleIndex+k);
                        polyTriangles.Add(triangleIndex);
                    }
                }
                triangleIndex += polyhedronsFacets[facet].Count;
            }
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
    public bool[] GetFacetsExteriorFlags()
    {
        bool[] res = Enumerable.Repeat(false, m_polyhedronsFacets.Count).ToArray();
        GetExteriorFacets().ForEach(i=>res[i]=true);
        return res;
    }

    public List<List<Vector2>> GetFacetsUVs()
    {
        #if UNITY_EDITOR
        List<List<Vector2>> res = new List<List<Vector2>>();
        bool[] isExteriorFacet = GetFacetsExteriorFlags();
        List<List<int>> facets = TetrahedralizerUtility.FlatIListToNestedList(m_polyhedronsFacets);
        Vector2[] meshUVs = Unwrapping.GeneratePerTriangleUV(ToMesh().mesh);

        int k = 0;
        for(int i=0; i<facets.Count; i++)
        {
            if(!isExteriorFacet[i])
            {
                res.Add(Enumerable.Repeat(Vector2.zero, facets[i].Count).ToList());
                continue;
            }

            List<Vector2> facetUVs = new List<Vector2>();
            facetUVs.Add(m_polyhedronsFacetsPointOut[i] ? meshUVs[k+0]:meshUVs[k+2]);
            for(int j=0; j<facets[i].Count-2; j++)
            {
                facetUVs.Add(meshUVs[k+3*j+1]);
            }
            k += 3*(facets[i].Count-2);
            facetUVs.Add(m_polyhedronsFacetsPointOut[i] ? meshUVs[k-1]:meshUVs[k-3]);
            res.Add(facetUVs);
        }
        return res;
        #else
        return null;
        #endif
    }

    public void CalculatePolyhedronsFacetsOrients()
    {
        using GenericPointPredicate genericPointPredicate = new GenericPointPredicate(m_explicitVertices, m_implicitVertices);
        List<List<int>> polyhedrons = TetrahedralizerUtility.FlatIListToNestedList(m_polyhedrons);
        List<List<int>> polyhedronsFacets = TetrahedralizerUtility.FlatIListToNestedList(m_polyhedronsFacets);
        m_polyhedronsFacetsPointOut = Enumerable.Repeat(false,polyhedronsFacets.Count).ToList();
        bool[] isExteriorFacets = GetFacetsExteriorFlags();

        for(int i=0; i<polyhedrons.Count; i++)
        {
            for(int j=0; j<polyhedrons[i].Count; j++)
            {
                int facet = polyhedrons[i][j];
                if(!isExteriorFacets[facet])
                {
                    continue;
                }
                m_polyhedronsFacetsPointOut[facet] = FacetPointsOut(facet, i, polyhedronsFacets, polyhedrons, genericPointPredicate);
            }
        }
    }

    public int GetVerticesCount()
    {
        return m_explicitVertices.Count/3 + TetrahedralizerUtility.CountFlatIListElements(m_implicitVertices);
    }
    public int GetPolyhedronsCount()
    {
        return m_polyhedrons.Count;
    }

    public void Assign(Polyhedralization polyhedralization)
    {
        m_explicitVertices = polyhedralization.m_explicitVertices;
        m_implicitVertices = polyhedralization.m_implicitVertices;
        m_polyhedrons = polyhedralization.m_polyhedrons;
        m_polyhedronsFacets = polyhedralization.m_polyhedronsFacets;
        m_polyhedronsFacetsPointOut = polyhedralization.m_polyhedronsFacetsPointOut;
    }

    private bool FacetPointsOut(int facet, int polyhedron, List<List<int>> polyhedronsFacets, List<List<int>> polyhedrons, GenericPointPredicate genericPointPredicate)
    {
        List<int> facetVertices = polyhedronsFacets[facet];
        int p0 = facetVertices[0];
        int p1 = facetVertices[1];
        int p2 = facetVertices[2];

        for(int i=0; i<polyhedrons[polyhedron].Count; i++)
        {
            if(facet == polyhedrons[polyhedron][i])
            {
                continue;
            }

            int f = polyhedrons[polyhedron][i];
            for(int j=0; j<polyhedronsFacets[f].Count; j++)
            {
                if(facetVertices.Contains(polyhedronsFacets[f][j]))
                {
                    continue;
                }

                int o = genericPointPredicate.Orient3d(p0,p1,p2,polyhedronsFacets[f][j]);
                if(0==o)
                {
                    continue;
                }
                return o<0;
            }
        }
        throw new Exception();
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
