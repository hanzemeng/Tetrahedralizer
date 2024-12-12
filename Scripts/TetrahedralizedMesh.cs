using System;
using System.Collections.Generic;
using UnityEngine;

[CreateAssetMenu(fileName = "Tetrahedralized_Mesh", menuName = "ScriptableObjects/Tetrahedralizer/Tetrahedralized_Mesh", order = 0)]
[PreferBinarySerialization]
public class TetrahedralizedMesh : ScriptableObject
{
    public Mesh originalMesh;

    public List<ListInt32> originalVerticesMappings; // originalVerticesMappings[i] contains originalMesh's vertices that share the same position as the ith vertex
    public List<NineInt32> newVerticesMappings;
    public List<Int32> tetrahedrons;

    public TetrahedralizedMesh()
    {
        originalVerticesMappings = new List<ListInt32>();
        newVerticesMappings = new List<NineInt32>();
        tetrahedrons = new List<Int32>();
    }
}
