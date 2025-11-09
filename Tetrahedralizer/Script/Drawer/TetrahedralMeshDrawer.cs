using System;
using System.Collections.Generic;
using UnityEngine;

[ExecuteAlways]
public class TetrahedralMeshDrawer : MonoBehaviour
{
    [SerializeField] private TetrahedralMesh m_tetrahedralMesh;
    [SerializeField] private bool m_drawTetrahedronsAsIndividualGameObjects;
    [SerializeField] private Transform m_tetrahedronsParent;
    [SerializeField] private List<Material> m_materials;
    [Range(0f,1f)] [SerializeField] private float m_tetrahedronsScale = 1f;


    private void Update()
    {
        if(null == m_tetrahedronsParent)
        {
            return;
        }
        foreach(Transform t in m_tetrahedronsParent)
        {
            t.localScale = m_tetrahedronsScale * Vector3.one;
        }
    }

    [ContextMenu("Draw")]
    public void Draw()
    {
        Draw(m_tetrahedralMesh, m_materials);
    }

    public void Draw(TetrahedralMesh tetrahedralMesh, List<Material> materials)
    {
        Clear();
        if(null == tetrahedralMesh)
        {
            return;
        }

        if(m_drawTetrahedronsAsIndividualGameObjects)
        {
            foreach((Mesh mesh, Vector3 center) in tetrahedralMesh.ToMeshes())
            {
                TetrahedralizerUtility.CreateGameObject(mesh, materials, m_tetrahedronsParent, center);
            }
        }
        else
        {
            (Mesh mesh, Vector3 center) = tetrahedralMesh.ToMesh();
            TetrahedralizerUtility.CreateGameObject(mesh, materials, m_tetrahedronsParent, center);
        }
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
