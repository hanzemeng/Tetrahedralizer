using System;
using System.Collections.Generic;
using UnityEngine;

[CreateAssetMenu(fileName = "Tetrahedralized_Mesh", menuName = "ScriptableObjects/Tetrahedralizer/Tetrahedralized_Mesh", order = 0)]
[PreferBinarySerialization]
public class TetrahedralizedMesh : ScriptableObject
{
    public Mesh originalMesh;

    public List<ListUInt32> originalVerticesMappings; // originalVerticesMappings[i] contains originalMesh's vertices that share the same position as the ith vertex
    public List<NineUInt32> newVerticesMappings;
    public List<UInt32> tetrahedrons;

    public TetrahedralizedMesh()
    {
        originalVerticesMappings = new List<ListUInt32>();
        newVerticesMappings = new List<NineUInt32>();
        tetrahedrons = new List<UInt32>();
    }
}
