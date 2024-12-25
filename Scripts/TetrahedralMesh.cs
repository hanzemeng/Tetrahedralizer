using System;
using System.Collections.Generic;
using UnityEngine;

[CreateAssetMenu(fileName = "Tetrahedral_Mesh", menuName = "ScriptableObjects/Tetrahedralizer/Tetrahedral_Mesh", order = 1)]
[PreferBinarySerialization]
public class TetrahedralMesh : ScriptableObject
{
    public List<Int32> tetrahedrons;
    public List<Vector3> vertices;

    public Mesh mesh;

    public TetrahedralMesh()
    {
        tetrahedrons = new List<Int32>();
        vertices = new List<Vector3>();
    }
}
