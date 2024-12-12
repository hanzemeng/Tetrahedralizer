using System;
using System.Linq;
using System.Collections;
using System.Collections.Generic;
using UnityEngine;

[RequireComponent(typeof(MeshFilter))]
[RequireComponent(typeof(MeshRenderer))]
public class TetrahedralMeshDrawer : MonoBehaviour
{
    public TetrahedralMesh tetrahedralMesh;
    public bool autoUpdate;

    [ContextMenu("Update Mesh")]
    public void UpdateMesh()
    {
        if(null == tetrahedralMesh.mesh)
        {
            Mesh mesh = new Mesh();

            mesh.vertices = tetrahedralMesh.vertices.ToArray();
            List<int> triangles = new List<int>();

            for(Int32 i=0; i<tetrahedralMesh.tetrahedrons.Count; i+=4)
            {
                Int32 p0 = tetrahedralMesh.tetrahedrons[i+0];
                Int32 p1 = tetrahedralMesh.tetrahedrons[i+1];
                Int32 p2 = tetrahedralMesh.tetrahedrons[i+2];
                Int32 p3 = tetrahedralMesh.tetrahedrons[i+3];
                triangles.Add(p0);
                triangles.Add(p1);
                triangles.Add(p3);

                triangles.Add(p1);
                triangles.Add(p2);
                triangles.Add(p3);

                triangles.Add(p2);
                triangles.Add(p0);
                triangles.Add(p3);

                triangles.Add(p0);
                triangles.Add(p2);
                triangles.Add(p1);
            }
            mesh.triangles = triangles.ToArray();
            mesh.RecalculateNormals();
            mesh.RecalculateTangents();

            GetComponent<MeshFilter>().mesh = mesh;
        }
        else
        {
            GetComponent<MeshFilter>().mesh = tetrahedralMesh.mesh;
        }
    }

    private void OnValidate()
    {
        if(!autoUpdate)
        {
            return;
        }

        UpdateMesh();
    }

    

}
