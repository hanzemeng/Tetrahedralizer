using System;
using System.Linq;
using System.Collections.Generic;
using UnityEngine;

[ExecuteAlways]
public class TetrahedralizationDrawer : MonoBehaviour
{
    [SerializeField] private bool m_drawTetrahedralizationAsIndividualGameObjects;
    [SerializeField] private Transform m_tetrahedronsParent;
    [SerializeField] private Material m_tetrahedronsMaterial;
    [SerializeField] [Range(0f,1f)] private float m_tetrahedronsScale;

    private readonly static int[] m_zeroToEleven = Enumerable.Range(0,12).ToArray();


    public void Update()
    {
        if(null == m_tetrahedronsParent)
        {
            return;
        }
        foreach(Transform tetrahedron in m_tetrahedronsParent)
        {
            tetrahedron.localScale = m_tetrahedronsScale * Vector3.one;
        }
    }

    public void Draw(Tetrahedralization tetrahedralization)
    {
        List<int> tetrahedrons = tetrahedralization.m_tetrahedrons;
        List<Vector3> vertices;
        if(null == tetrahedralization.m_implicitVertices || 0 == tetrahedralization.m_implicitVertices.Count)
        {
            vertices = TetrahedralizerLibraryUtility.PackDoubles(tetrahedralization.m_explicitVertices);
        }
        else
        {
            GenericPointApproximation.GenericPointApproximationInput input = new GenericPointApproximation.GenericPointApproximationInput();
            input.m_explicitVertices = tetrahedralization.m_explicitVertices;
            input.m_implicitVertices = tetrahedralization.m_implicitVertices;
            GenericPointApproximation.GenericPointApproximationOutput output = new GenericPointApproximation.GenericPointApproximationOutput();
            GenericPointApproximation genericPointApproximation = new GenericPointApproximation();
            genericPointApproximation.CalculateGenericPointApproximation(input, output);
            vertices = TetrahedralizerLibraryUtility.PackDoubles(output.m_approximatePositions);
        }

        Clear();

        if(m_drawTetrahedralizationAsIndividualGameObjects)
        {
            DrawAsIndividual(tetrahedrons, vertices);
        }
        else
        {
            DrawAsWhole(tetrahedrons, vertices);
        }
    }

    private void DrawAsIndividual(List<int> tetrahedrons, List<Vector3> vertices)
    {
        for(int i=0; i<tetrahedrons.Count; i+=4)
        {
            Vector3 v0 = vertices[tetrahedrons[i+0]];
            Vector3 v1 = vertices[tetrahedrons[i+1]];
            Vector3 v2 = vertices[tetrahedrons[i+2]];
            Vector3 v3 = vertices[tetrahedrons[i+3]];
            Vector3 average = (v0+v1+v2+v3) / 4f;
            v0 -= average;
            v1 -= average;
            v2 -= average;
            v3 -= average;

            GameObject newGameObject = new GameObject();
            newGameObject.name = $"{tetrahedrons[i+0]}_{tetrahedrons[i+1]}_{tetrahedrons[i+2]}_{tetrahedrons[i+3]}";
            newGameObject.transform.SetParent(m_tetrahedronsParent);
            newGameObject.transform.position = average;
            MeshFilter meshFilter = newGameObject.AddComponent<MeshFilter>();
            MeshRenderer meshRenderer = newGameObject.AddComponent<MeshRenderer>();

            Mesh mesh = new Mesh();
            mesh.vertices = new Vector3[]{v0,v2,v1, v0,v1,v3, v0,v3,v2, v1,v2,v3};
            mesh.triangles = m_zeroToEleven;
            mesh.RecalculateBounds();
            mesh.RecalculateNormals();
            mesh.RecalculateTangents();
            meshFilter.mesh = mesh;

            meshRenderer.material = m_tetrahedronsMaterial;
        }
    }
    private void DrawAsWhole(List<int> tetrahedrons, List<Vector3> vertices)
    {
        GameObject newGameObject = new GameObject();
        newGameObject.transform.SetParent(m_tetrahedronsParent);
        MeshFilter meshFilter = newGameObject.AddComponent<MeshFilter>();
        MeshRenderer meshRenderer = newGameObject.AddComponent<MeshRenderer>();
        
        List<Vector3> meshVertices = new List<Vector3>(3*tetrahedrons.Count);

        for(int i=0; i<tetrahedrons.Count; i+=4)
        {
            Vector3 v0 = vertices[tetrahedrons[i+0]];
            Vector3 v1 = vertices[tetrahedrons[i+1]];
            Vector3 v2 = vertices[tetrahedrons[i+2]];
            Vector3 v3 = vertices[tetrahedrons[i+3]];

            meshVertices.Add(v0,v2,v1, v0,v1,v3, v0,v3,v2, v1,v2,v3);
        }

        Mesh mesh = new Mesh();
        mesh.indexFormat = UnityEngine.Rendering.IndexFormat.UInt32;
        mesh.vertices = meshVertices.ToArray();
        mesh.triangles = Enumerable.Range(0,3*tetrahedrons.Count).ToArray();
        mesh.RecalculateBounds();
        mesh.RecalculateNormals();
        mesh.RecalculateTangents();
        meshFilter.mesh = mesh;
        meshRenderer.material = m_tetrahedronsMaterial;
    }

    [ContextMenu("Clear")]
    public void Clear()
    {
        while(0 != m_tetrahedronsParent.childCount)
        {
            DestroyImmediate(m_tetrahedronsParent.GetChild(0).gameObject);
        }
    }
}
