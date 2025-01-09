namespace Hanzzz.Tetrahedralizer
{

using System;
using System.Linq;
using System.Collections.Generic;
using UnityEngine;

[ExecuteInEditMode]
public class TetrahedralMeshDrawer : MonoBehaviour
{
    public TetrahedralMesh tetrahedralMesh;
    public Material[] materials;
    public bool asIndividualTetrahedron;
    [Range(0f,1f)] public float scale = 1f;
    
    private void OnValidate()
    {
        UpdateScale();
    }


    [ContextMenu("Update Mesh")]
    public void UpdateMesh()
    {
        Clear();
        if(null == tetrahedralMesh)
        {
            return;
        }
        if(0 != tetrahedralMesh.tetrahedrons.Count)
        {
            if(asIndividualTetrahedron)
            {
                Vector3[] vertices = new Vector3[12];
                Int32[] triangles = new Int32[12]{0,1,2,3,4,5,6,7,8,9,10,11};

                for(Int32 i=0; i<tetrahedralMesh.tetrahedrons.Count; i+=4)
                {
                    Int32 p0 = tetrahedralMesh.tetrahedrons[i+0];
                    Int32 p1 = tetrahedralMesh.tetrahedrons[i+1];
                    Int32 p2 = tetrahedralMesh.tetrahedrons[i+2];
                    Int32 p3 = tetrahedralMesh.tetrahedrons[i+3];

                    vertices[0] = tetrahedralMesh.vertices[p0];
                    vertices[1] = tetrahedralMesh.vertices[p1];
                    vertices[2] = tetrahedralMesh.vertices[p3];

                    vertices[3] = tetrahedralMesh.vertices[p1];
                    vertices[4] = tetrahedralMesh.vertices[p2];
                    vertices[5] = tetrahedralMesh.vertices[p3];

                    vertices[6] = tetrahedralMesh.vertices[p2];
                    vertices[7] = tetrahedralMesh.vertices[p0];
                    vertices[8] = tetrahedralMesh.vertices[p3];

                    vertices[9] = tetrahedralMesh.vertices[p0];
                    vertices[10] = tetrahedralMesh.vertices[p2];
                    vertices[11] = tetrahedralMesh.vertices[p1];

                    Mesh mesh = new Mesh();
                    Vector3 center = AdjustVerticesCenter(vertices);
                    mesh.vertices = vertices;
                    mesh.triangles = triangles;
                    mesh.RecalculateBounds();
                    mesh.RecalculateNormals();
                    mesh.RecalculateTangents();

                    CreateGameObject(mesh, center);
                }
            }
            else
            {
                Mesh mesh = new Mesh();
                List<Vector3> vertices = new List<Vector3>();
                mesh.indexFormat = UnityEngine.Rendering.IndexFormat.UInt32;

                for(Int32 i = 0; i<tetrahedralMesh.tetrahedrons.Count; i+=4)
                {
                    Int32 p0 = tetrahedralMesh.tetrahedrons[i+0];
                    Int32 p1 = tetrahedralMesh.tetrahedrons[i+1];
                    Int32 p2 = tetrahedralMesh.tetrahedrons[i+2];
                    Int32 p3 = tetrahedralMesh.tetrahedrons[i+3];

                    vertices.Add(tetrahedralMesh.vertices[p0]);
                    vertices.Add(tetrahedralMesh.vertices[p1]);
                    vertices.Add(tetrahedralMesh.vertices[p3]);

                    vertices.Add(tetrahedralMesh.vertices[p1]);
                    vertices.Add(tetrahedralMesh.vertices[p2]);
                    vertices.Add(tetrahedralMesh.vertices[p3]);

                    vertices.Add(tetrahedralMesh.vertices[p2]);
                    vertices.Add(tetrahedralMesh.vertices[p0]);
                    vertices.Add(tetrahedralMesh.vertices[p3]);

                    vertices.Add(tetrahedralMesh.vertices[p0]);
                    vertices.Add(tetrahedralMesh.vertices[p2]);
                    vertices.Add(tetrahedralMesh.vertices[p1]);
                }
                mesh.vertices = vertices.ToArray();
                mesh.triangles = Enumerable.Range(0,tetrahedralMesh.tetrahedrons.Count/4*12).Select(i => i).ToArray();
                mesh.RecalculateBounds();
                mesh.RecalculateNormals();
                mesh.RecalculateTangents();

                CreateGameObject(mesh);
            }
        }
        else
        {
            List<Int32> temp = new List<Int32>();
            List<Int32> triangleSubmeshes = new List<Int32>();
            List<Vector3> vertices = new List<Vector3>();
            MeshVertexDataMapper mvdm = new MeshVertexDataMapper();

            mvdm.AssignSourceTetrahedralMesh(tetrahedralMesh);
            Int32 submeshCount = tetrahedralMesh.GetSubmeshesCount();
            tetrahedralMesh.GetFacetsSubmeshes(triangleSubmeshes);

            if(asIndividualTetrahedron)
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
                    
                mesh.RecalculateBounds();
                mesh.RecalculateNormals();
                mesh.RecalculateTangents();

                mesh.GetVertices(vertices);
                Vector3 center = AdjustVerticesCenter(vertices);
                mesh.SetVertices(vertices);
                CreateGameObject(mesh, center);
            }
        }
    }

    //[ContextMenu("Update Scale")]
    public void UpdateScale()
    {
        foreach(Transform t in transform)
        {
            t.localScale = scale * Vector3.one;
        }
    }

    [ContextMenu("Clear")]
    public void Clear()
    {
        while(0 != transform.childCount)
        {
            DestroyImmediate(transform.GetChild(0).gameObject);
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
        (g.AddComponent<MeshRenderer>()).materials = materials;
        g.transform.SetParent(transform, false);
        g.transform.localPosition = position;
        g.transform.localScale = scale * Vector3.one;
    }
}

}
