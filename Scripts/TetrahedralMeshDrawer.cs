namespace Hanzzz.Tetrahedralizer
{

using System;
using System.Linq;
using System.Collections.Generic;
using UnityEngine;

[RequireComponent(typeof(MeshFilter))]
[RequireComponent(typeof(MeshRenderer))]
[ExecuteInEditMode]
public class TetrahedralMeshDrawer : MonoBehaviour
{
    public TetrahedralMesh tetrahedralMesh;

    private MeshFilter m_meshFilter;
    private bool m_changed;


    private void Awake()
    {
        m_meshFilter = GetComponent<MeshFilter>();
        m_changed = true;
    }
    private void Update()
    {
        if(m_changed)
        {
            m_changed = false;
            UpdateMesh();
        }
    }
    private void OnValidate()
    {
        m_changed = true;
    }


    [ContextMenu("Update Mesh")]
    public void UpdateMesh()
    {
        if(null == tetrahedralMesh)
        {
            m_meshFilter.mesh = null;
            return;
        }
        if(null == tetrahedralMesh.mesh)
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
            mesh.RecalculateNormals();
            mesh.RecalculateTangents();

            m_meshFilter.mesh = mesh;

            // Below is to make a game object for each tetrahedron, which I advise against for preformance reason.

            //Vector3[] vertices = new Vector3[12];
            //int[] triangles = new int[12]{0,1,2,3,4,5,6,7,8,9,10,11};
            //Material material = GetComponent<MeshRenderer>().sharedMaterial;
            //for(Int32 i = 0; i<tetrahedralMesh.tetrahedrons.Count; i+=4)
            //{
            //    Int32 p0 = tetrahedralMesh.tetrahedrons[i+0];
            //    Int32 p1 = tetrahedralMesh.tetrahedrons[i+1];
            //    Int32 p2 = tetrahedralMesh.tetrahedrons[i+2];
            //    Int32 p3 = tetrahedralMesh.tetrahedrons[i+3];

            //    vertices[0] = tetrahedralMesh.vertices[p0];
            //    vertices[1] = tetrahedralMesh.vertices[p1];
            //    vertices[2] = tetrahedralMesh.vertices[p3];

            //    vertices[3] = tetrahedralMesh.vertices[p1];
            //    vertices[4] = tetrahedralMesh.vertices[p2];
            //    vertices[5] = tetrahedralMesh.vertices[p3];

            //    vertices[6] = tetrahedralMesh.vertices[p2];
            //    vertices[7] = tetrahedralMesh.vertices[p0];
            //    vertices[8] = tetrahedralMesh.vertices[p3];

            //    vertices[9] = tetrahedralMesh.vertices[p0];
            //    vertices[10] = tetrahedralMesh.vertices[p2];
            //    vertices[11] = tetrahedralMesh.vertices[p1];

            //    Mesh mesh = new Mesh();
            //    mesh.vertices = vertices;
            //    mesh.triangles = triangles;
            //    mesh.RecalculateNormals();
            //    mesh.RecalculateTangents();
            //    GameObject g = new GameObject();
            //    (g.AddComponent<MeshFilter>()).mesh = mesh;
            //    g.AddComponent<MeshRenderer>().material = material;
            //}
        }
        else
        {
            GetComponent<MeshFilter>().mesh = tetrahedralMesh.mesh;
        }
    }
}

}
