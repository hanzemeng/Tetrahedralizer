using System;
using System.Linq;
using System.Collections.Generic;
using UnityEngine;

[ExecuteAlways]
public class TetrahedralizationDrawer : MonoBehaviour
{
    [SerializeField] private Tetrahedralization m_tetrahedralization;
    [SerializeField] private bool m_drawTetrahedralizationAsIndividualGameObjects;
    [SerializeField] private Transform m_tetrahedronsParent;
    [SerializeField] private Material m_tetrahedronsMaterial;
    [SerializeField] [Range(0f,1f)] private float m_tetrahedronsScale;


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

    [ContextMenu("Draw")]
    public void Draw()
    {
        Draw(m_tetrahedralization);
    }
    public void Draw(Tetrahedralization tetrahedralization)
    {
        Clear();
        if(m_drawTetrahedralizationAsIndividualGameObjects)
        {
            DrawAsIndividual(tetrahedralization);
        }
        else
        {
            DrawAsWhole(tetrahedralization);
        }
    }
    private void DrawAsIndividual(Tetrahedralization tetrahedralization)
    {
        foreach((Mesh mesh, Vector3 center) in tetrahedralization.ToMeshes())
        {
            GameObject newGameObject = new GameObject();
            newGameObject.transform.SetParent(m_tetrahedronsParent);
            newGameObject.transform.position = center;

            MeshFilter meshFilter = newGameObject.AddComponent<MeshFilter>();
            MeshRenderer meshRenderer = newGameObject.AddComponent<MeshRenderer>();
            meshFilter.mesh = mesh;
            meshRenderer.material = m_tetrahedronsMaterial;
        }
    }
    private void DrawAsWhole(Tetrahedralization tetrahedralization)
    {
        (Mesh mesh, Vector3 center) = tetrahedralization.ToMesh();

        GameObject newGameObject = new GameObject();
        newGameObject.transform.SetParent(m_tetrahedronsParent);
        newGameObject.transform.position = center;

        MeshFilter meshFilter = newGameObject.AddComponent<MeshFilter>();
        MeshRenderer meshRenderer = newGameObject.AddComponent<MeshRenderer>();
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
