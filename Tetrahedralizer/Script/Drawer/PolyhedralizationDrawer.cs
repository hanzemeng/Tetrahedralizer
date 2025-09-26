using System;
using System.Text;
using System.Linq;
using System.Collections.Generic;
using UnityEngine;

[ExecuteAlways]
public class PolyhedralizationDrawer : MonoBehaviour
{
    [SerializeField] private Polyhedralization m_polyhedralization;
    [SerializeField] private bool m_drawPolyhedralizationAsIndividualGameObjects;
    [SerializeField] private bool m_drawPolyhedralizationInteriorFacets;
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

    [ContextMenu("Draw")]
    public void Draw()
    {
        Draw(m_polyhedralization);
    }
    public void Draw(Polyhedralization polyhedralization)
    {
        Clear();
        if(m_drawPolyhedralizationAsIndividualGameObjects)
        {
            DrawAsIndividual(polyhedralization);
        }
        else
        {
            DrawAsWhole(polyhedralization);
        }
    }
    private void DrawAsIndividual(Polyhedralization polyhedralization)
    {
        foreach((Mesh mesh, Vector3 center) in polyhedralization.ToMeshes())
        {
            GameObject newGameObject = new GameObject();
            newGameObject.transform.SetParent(m_polyhedronsParent);
            newGameObject.transform.position = center;

            MeshFilter meshFilter = newGameObject.AddComponent<MeshFilter>();
            MeshRenderer meshRenderer = newGameObject.AddComponent<MeshRenderer>();
            meshFilter.mesh = mesh;
            meshRenderer.material = m_polyhedronsMaterial;
        }
    }
    private void DrawAsWhole(Polyhedralization polyhedralization)
    {
        (Mesh mesh, Vector3 center) = polyhedralization.ToMesh(m_drawPolyhedralizationInteriorFacets);

        GameObject newGameObject = new GameObject();
        newGameObject.transform.SetParent(m_polyhedronsParent);
        newGameObject.transform.position = center;

        MeshFilter meshFilter = newGameObject.AddComponent<MeshFilter>();
        MeshRenderer meshRenderer = newGameObject.AddComponent<MeshRenderer>();
        meshFilter.mesh = mesh;
        meshRenderer.material = m_polyhedronsMaterial;
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
