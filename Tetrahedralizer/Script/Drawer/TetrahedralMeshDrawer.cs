using System;
using System.Linq;
using System.Collections.Generic;
using UnityEngine;

[ExecuteAlways]
public class TetrahedralMeshDrawer : MonoBehaviour
{
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

    public void Draw(TetrahedralMesh tetrahedralMesh, List<Material> materials=null)
    {
        Clear();
        if(null == tetrahedralMesh)
        {
            return;
        }
        if(null != materials)
        {
            m_materials = materials;
        }
        List<Int32> temp = new List<Int32>();
        List<Int32> triangleSubmeshes = new List<Int32>();
        List<Vector3> vertices = new List<Vector3>();
        MeshVertexDataMapper mvdm = new MeshVertexDataMapper();

        mvdm.AssignSourceTetrahedralMesh(tetrahedralMesh);
        Int32 submeshCount = tetrahedralMesh.GetSubmeshesCount();
        tetrahedralMesh.GetFacetsSubmeshes(triangleSubmeshes);

        if(m_drawTetrahedronsAsIndividualGameObjects)
        {
            for(Int32 i=0; i<triangleSubmeshes.Count; i+=4)
            {
                for(Int32 j=0; j<12; j++)
                {
                    mvdm.CopyVertexData(3*i+j);
                }
                Mesh mesh = mvdm.MakeMesh();
                mesh.subMeshCount = submeshCount;
                mvdm.ClearTarget();

                for(Int32 j=0; j<submeshCount; j++)
                {
                    temp.Clear();
                    for(Int32 k=0; k<4; k++)
                    {
                        if(j == triangleSubmeshes[i+k])
                        {
                            temp.Add(3*k+0);
                            temp.Add(3*k+1);
                            temp.Add(3*k+2);
                        }
                        
                    }
                    mesh.SetTriangles(temp,j);
                }

                mesh.GetVertices(vertices);
                Vector3 center = AdjustVerticesCenter(vertices);
                mesh.SetVertices(vertices);

                mesh.RecalculateBounds();
                mesh.RecalculateNormals();
                mesh.RecalculateTangents();

                CreateGameObject(mesh, center);
            }
        }
        else
        {
            for(Int32 i=0; i<3*triangleSubmeshes.Count; i++)
            {
                mvdm.CopyVertexData(i);
            }

            Mesh mesh = mvdm.MakeMesh();
            mesh.subMeshCount = submeshCount;
            for(Int32 i=0; i<submeshCount; i++)
            {
                temp.Clear();
                for(Int32 j=0; j<triangleSubmeshes.Count; j++)
                {
                    if(i == triangleSubmeshes[j])
                    {
                        temp.Add(3*j+0);
                        temp.Add(3*j+1);
                        temp.Add(3*j+2);
                    }
                        
                }
                mesh.SetTriangles(temp,i);
            }

            mesh.GetVertices(vertices);
            Vector3 center = AdjustVerticesCenter(vertices);
            mesh.SetVertices(vertices);

            mesh.RecalculateBounds();
            mesh.RecalculateNormals();
            mesh.RecalculateTangents();

            CreateGameObject(mesh, center);
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

    private Vector3 AdjustVerticesCenter(IList<Vector3> vertices)
    {
        Vector3 center = Vector3.zero;

        foreach(Vector3 v in vertices)
        {
            center += v;
        }
        center /= vertices.Count;
        for(Int32 i=0; i<vertices.Count; i++)
        {
            vertices[i] = vertices[i] - center;
        }

        return center;
    }

    private void CreateGameObject(Mesh mesh)
    {
        CreateGameObject(mesh, Vector3.zero);
    }
    private void CreateGameObject(Mesh mesh, Vector3 position)
    {
        GameObject g = new GameObject();
        (g.AddComponent<MeshFilter>()).mesh = mesh;
        (g.AddComponent<MeshRenderer>()).SetMaterials(m_materials);
        g.transform.SetParent(m_tetrahedronsParent, false);
        g.transform.localPosition = position;
    }
}
