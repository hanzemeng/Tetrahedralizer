using System;
using System.Linq;
using System.Collections.Generic;
using System.Threading.Tasks;
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
        TetrahedralizerUtility.RemoveDuplicateVertices(weldedVertices, weldedTriangles);
        Tetrahedralization tetrahedralization = ScriptableObject.CreateInstance<Tetrahedralization>();
        output.m_tetrahedralization = tetrahedralization;

        CreateInternal(tetrahedralization, weldedTriangles, TetrahedralizerUtility.UnpackVector3s(weldedVertices));
    }
    public Task CreateAsync(TetrahedralizedMeshCreationInput input, TetrahedralizedMeshCreationOutput output, IProgress<string> progress=null)
    {
        List<Vector3> weldedVertices = input.m_mesh.vertices.ToList();
        int[] weldedTriangles = input.m_mesh.triangles;
        TetrahedralizerUtility.RemoveDuplicateVertices(weldedVertices, weldedTriangles);
        Tetrahedralization tetrahedralization = ScriptableObject.CreateInstance<Tetrahedralization>();
        output.m_tetrahedralization = tetrahedralization;

        return Task.Run(() =>
        {
            CreateInternal(tetrahedralization, weldedTriangles, TetrahedralizerUtility.UnpackVector3s(weldedVertices), progress);
        });
    }

    private void CreateInternal(Tetrahedralization tetrahedralization, int[] weldedTriangles, List<double> weldedVerticesUnpack, IProgress<string> progress=null)
    {
        if(null != progress)
        {
            progress.Report("Tetrahedralize vertices.");
        }
        DelaunayTetrahedralization.DelaunayTetrahedralizationInput DTInput = new DelaunayTetrahedralization.DelaunayTetrahedralizationInput();
        DelaunayTetrahedralization.DelaunayTetrahedralizationOutput DTOutput = new DelaunayTetrahedralization.DelaunayTetrahedralizationOutput();
        {
            DelaunayTetrahedralization delaunayTetrahedralization = new DelaunayTetrahedralization();
            DTInput.m_explicitVertices = weldedVerticesUnpack;
            delaunayTetrahedralization.CalculateDelaunayTetrahedralization(DTInput, DTOutput);
        }

        if(null != progress)
        {
            progress.Report("Cut tetrahedrons with constraints.");
        }
        BinarySpacePartition.BinarySpacePartitionInput BSPInput = new BinarySpacePartition.BinarySpacePartitionInput();
        BinarySpacePartition.BinarySpacePartitionOutput BSPOutput = new BinarySpacePartition.BinarySpacePartitionOutput();
        {
            
            BinarySpacePartition binarySpacePartition = new BinarySpacePartition();
            BSPInput.m_explicitVertices = weldedVerticesUnpack;
            BSPInput.m_tetrahedrons = DTOutput.m_tetrahedrons;
            BSPInput.m_constraints = weldedTriangles;
            binarySpacePartition.CalculateBinarySpacePartition(BSPInput, BSPOutput);
        }

        if(null != progress)
        {
            progress.Report("Calculate winding numbers and labels.");
        }
        InteriorCharacterization.InteriorCharacterizationInput ICInput = new InteriorCharacterization.InteriorCharacterizationInput();
        InteriorCharacterization.InteriorCharacterizationOutput ICOutput = new InteriorCharacterization.InteriorCharacterizationOutput();
        {
            InteriorCharacterization interiorCharacterization = new InteriorCharacterization();
            ICInput.m_explicitVertices = weldedVerticesUnpack;
            ICInput.m_implicitVertices = BSPOutput.m_insertedVertices;
            ICInput.m_polyhedrons = BSPOutput.m_polyhedrons;
            ICInput.m_polyhedronsFacets = BSPOutput.m_polyhedronsFacets;
            ICInput.m_constraints = weldedTriangles;
            ICInput.m_polyhedronsWindingNumbers = null;
            ICInput.m_minCutNeighborMultiplier = 1d;
            interiorCharacterization.CalculateInteriorCharacterization(ICInput, ICOutput);
        }

        if(null != progress)
        {
            progress.Report("Convert polyhedrons to tetrahedrons.");
        }
        List<int> insidePolyhedrons = TetrahedralizerUtility.NestedListToFlatList(TetrahedralizerUtility.FlatIListToNestedList(BSPOutput.m_polyhedrons).Where((i,j)=>0!=ICOutput.m_polyhedronsInteriorLabels[j]).ToList());
        PolyhedralizationTetrahedralization.PolyhedralizationTetrahedralizationInput PTInput = new PolyhedralizationTetrahedralization.PolyhedralizationTetrahedralizationInput();
        PolyhedralizationTetrahedralization.PolyhedralizationTetrahedralizationOutput PTOutput = new PolyhedralizationTetrahedralization.PolyhedralizationTetrahedralizationOutput();
        {
            PolyhedralizationTetrahedralization polyhedralizationTetrahedralization = new PolyhedralizationTetrahedralization();
            PTInput.m_explicitVertices = weldedVerticesUnpack;
            PTInput.m_implicitVertices = BSPOutput.m_insertedVertices;
            PTInput.m_polyhedrons = insidePolyhedrons;
            PTInput.m_polyhedronsFacets = BSPOutput.m_polyhedronsFacets;
            polyhedralizationTetrahedralization.CalculatePolyhedralizationTetrahedralization(PTInput, PTOutput);
        }

        // produce output
        tetrahedralization.m_explicitVertices = weldedVerticesUnpack;
        tetrahedralization.m_implicitVertices = BSPOutput.m_insertedVertices;
        tetrahedralization.m_tetrahedrons = PTOutput.m_tetrahedrons;
    }
}
