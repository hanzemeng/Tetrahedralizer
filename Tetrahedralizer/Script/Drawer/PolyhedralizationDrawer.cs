using System;
using System.Text;
using System.Linq;
using System.Collections.Generic;
using UnityEngine;

[ExecuteAlways]
public class PolyhedralizationDrawer : MonoBehaviour
{
    [SerializeField] private Transform m_polyhedronsParent;
    [SerializeField] private Material m_polyhedronsMaterial;
    [SerializeField] [Range(0f,1f)] private float m_polyhedronsScale;


    public void Update()
    {
        if(null == m_polyhedronsParent)
        {
            return;
        }
        foreach(Transform polyhedron in m_polyhedronsParent)
        {
            polyhedron.localScale = m_polyhedronsScale * Vector3.one;
        }
    }

    public void Draw(Polyhedralization polyhedralization)
    {
        Clear();

        List<Vector3> vertices;
        if(null == polyhedralization.m_implicitVertices || 0 == polyhedralization.m_implicitVertices.Count)
        {
            vertices = TetrahedralizerLibraryUtility.PackDoubles(polyhedralization.m_explicitVertices);
        }
        else
        {
            GenericPointApproximation.GenericPointApproximationInput input = new GenericPointApproximation.GenericPointApproximationInput();
            input.m_explicitVertices = polyhedralization.m_explicitVertices;
            input.m_implicitVertices = polyhedralization.m_implicitVertices;
            GenericPointApproximation.GenericPointApproximationOutput output = new GenericPointApproximation.GenericPointApproximationOutput();
            GenericPointApproximation genericPointApproximation = new GenericPointApproximation();
            genericPointApproximation.CalculateGenericPointApproximation(input, output);
            vertices = TetrahedralizerLibraryUtility.PackDoubles(output.m_approximatePositions);
        }

        List<List<int>> polyhedrons = TetrahedralizerLibraryUtility.FlatIListToNestedList(polyhedralization.m_polyhedrons);
        List<List<int>> polyhedronsFacets = TetrahedralizerLibraryUtility.FlatIListToNestedList(polyhedralization.m_polyhedronsFacets);

        StringBuilder stringBuilder = new StringBuilder();
        HashSet<int> uniqueVertices = new HashSet<int>();
        List<Vector3> polyVertices = new List<Vector3>();
        List<int> polyTriangles = new List<int>();
        int triangleIndex = 0;
        for(int i=0; i<polyhedrons.Count; i++)
        {
            stringBuilder.Clear();
            uniqueVertices.Clear();
            polyVertices.Clear();
            polyTriangles.Clear();
            triangleIndex = 0;

            for(int j=0; j<polyhedrons[i].Count; j++)
            {
                int facet = polyhedrons[i][j];
                stringBuilder.Append($"{facet}_");
                // draw one side first
                for(int k=0; k<polyhedronsFacets[facet].Count; k++)
                {
                    uniqueVertices.Add(polyhedronsFacets[facet][k]);
                    polyVertices.Add(vertices[polyhedronsFacets[facet][k]]);
                }
                for(int k=1; k<polyhedronsFacets[facet].Count-1; k++)
                {
                    polyTriangles.Add(triangleIndex);
                    polyTriangles.Add(triangleIndex+k);
                    polyTriangles.Add(triangleIndex+k+1);
                }
                triangleIndex += polyhedronsFacets[facet].Count;
                // draw the other side
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

            Vector3 avg = uniqueVertices.Aggregate(Vector3.zero, (i,j)=>i+vertices[j]) / uniqueVertices.Count;
            for(int j=0; j<polyVertices.Count; j++)
            {
                polyVertices[j] -= avg;
            }

            stringBuilder.Remove(stringBuilder.Length-1, 1);
            GameObject newGameObject = new GameObject();
            newGameObject.name = stringBuilder.ToString();
            newGameObject.transform.SetParent(m_polyhedronsParent);
            newGameObject.transform.position = avg;
            MeshFilter meshFilter = newGameObject.AddComponent<MeshFilter>();
            MeshRenderer meshRenderer = newGameObject.AddComponent<MeshRenderer>();

            Mesh mesh = new Mesh();
            mesh.vertices = polyVertices.ToArray();
            mesh.triangles = polyTriangles.ToArray();
            mesh.RecalculateBounds();
            mesh.RecalculateNormals();
            mesh.RecalculateTangents();
            meshFilter.mesh = mesh;

            meshRenderer.material = m_polyhedronsMaterial;
        }
    }

    [ContextMenu("Clear")]
    public void Clear()
    {
        while(0 != m_polyhedronsParent.childCount)
        {
            DestroyImmediate(m_polyhedronsParent.GetChild(0).gameObject);
        }
    }
}
