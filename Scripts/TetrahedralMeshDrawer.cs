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

            for(UInt32 i=0; i<tetrahedralMesh.tetrahedrons.Count; i+=4)
            {
                UInt32 p0 = tetrahedralMesh.tetrahedrons[(int)i+0];
                UInt32 p1 = tetrahedralMesh.tetrahedrons[(int)i+1];
                UInt32 p2 = tetrahedralMesh.tetrahedrons[(int)i+2];
                UInt32 p3 = tetrahedralMesh.tetrahedrons[(int)i+3];
                triangles.Add((int)p0);
                triangles.Add((int)p1);
                triangles.Add((int)p3);

                triangles.Add((int)p1);
                triangles.Add((int)p2);
                triangles.Add((int)p3);

                triangles.Add((int)p2);
                triangles.Add((int)p0);
                triangles.Add((int)p3);

                triangles.Add((int)p0);
                triangles.Add((int)p2);
                triangles.Add((int)p1);
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
