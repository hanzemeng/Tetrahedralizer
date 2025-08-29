using System;
using System.Linq;
using System.Collections.Generic;
using UnityEngine;

public class TetrahedralizedMeshCreation
{
    public class TetrahedralizedMeshCreationInput
    {
        public Mesh m_mesh;
    }
    public class TetrahedralizedMeshCreationOutput
    {
        public Tetrahedralization m_tetrahedralization;
    }

    public void Create(TetrahedralizedMeshCreationInput input, TetrahedralizedMeshCreationOutput output)
    {
        List<Vector3> weldedVertices = input.m_mesh.vertices.ToList();
        int[] weldedTriangles = input.m_mesh.triangles;
        TetrahedralizerLibraryUtility.RemoveDuplicateVertices(weldedVertices, weldedTriangles);
        List<double> weldedVerticesUnpack = TetrahedralizerLibraryUtility.UnpackVector3s(weldedVertices);

        // tetrahedralize mesh vertices
        DelaunayTetrahedralization.DelaunayTetrahedralizationInput DTInput = new DelaunayTetrahedralization.DelaunayTetrahedralizationInput();
        DelaunayTetrahedralization.DelaunayTetrahedralizationOutput DTOutput = new DelaunayTetrahedralization.DelaunayTetrahedralizationOutput();
        {
            DelaunayTetrahedralization delaunayTetrahedralization = new DelaunayTetrahedralization();
            DTInput.m_explicitVertices = weldedVerticesUnpack;
            delaunayTetrahedralization.CalculateDelaunayTetrahedralization(DTInput, DTOutput);
        }

        // cut tetrahedrons with mesh constraints (triangles)
        BinarySpacePartition.BinarySpacePartitionInput BSPInput = new BinarySpacePartition.BinarySpacePartitionInput();
        BinarySpacePartition.BinarySpacePartitionOutput BSPOutput = new BinarySpacePartition.BinarySpacePartitionOutput();
        {
            
            BinarySpacePartition binarySpacePartition = new BinarySpacePartition();
            BSPInput.m_explicitVertices = weldedVerticesUnpack;
            BSPInput.m_tetrahedrons = DTOutput.m_tetrahedrons;
            BSPInput.m_constraints = weldedTriangles;
            binarySpacePartition.CalculateBinarySpacePartition(BSPInput, BSPOutput);
        }

        // convert polyhedrons to tetrahedrons
        PolyhedralizationTetrahedralization.PolyhedralizationTetrahedralizationInput PTInput = new PolyhedralizationTetrahedralization.PolyhedralizationTetrahedralizationInput();
        PolyhedralizationTetrahedralization.PolyhedralizationTetrahedralizationOutput PTOutput = new PolyhedralizationTetrahedralization.PolyhedralizationTetrahedralizationOutput();
        {
            
            PolyhedralizationTetrahedralization polyhedralizationTetrahedralization = new PolyhedralizationTetrahedralization();
            PTInput.m_explicitVertices = weldedVerticesUnpack;
            PTInput.m_implicitVertices = BSPOutput.m_insertedVertices;
            PTInput.m_polyhedrons = BSPOutput.m_polyhedrons;
            PTInput.m_polyhedronsFacets = BSPOutput.m_polyhedronsFacets;
            polyhedralizationTetrahedralization.CalculatePolyhedralizationTetrahedralization(PTInput, PTOutput);
        }

        // produce output
        Tetrahedralization tetrahedralization = ScriptableObject.CreateInstance<Tetrahedralization>();
        tetrahedralization.m_explicitVertices = weldedVerticesUnpack;
        tetrahedralization.m_implicitVertices = BSPOutput.m_insertedVertices;
        tetrahedralization.m_tetrahedrons = PTOutput.m_tetrahedrons;
        output.m_tetrahedralization = tetrahedralization;
    }
}
