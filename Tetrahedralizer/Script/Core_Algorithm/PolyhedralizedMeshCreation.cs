using System;
using System.Linq;
using System.Collections.Generic;
using System.Threading.Tasks;
using UnityEngine;

public class PolyhedralizedMeshCreation
{
    public class PolyhedralizedMeshCreationInput
    {
        public Mesh m_mesh;
    }
    public class PolyhedralizedMeshCreationOutput
    {
        public Polyhedralization m_polyhedralization;
    }

    public void Create(PolyhedralizedMeshCreationInput input, PolyhedralizedMeshCreationOutput output)
    {
        List<Vector3> weldedVertices = input.m_mesh.vertices.ToList();
        int[] weldedTriangles = input.m_mesh.triangles;
        TetrahedralizerUtility.RemoveDuplicateVertices(weldedVertices, weldedTriangles);
        Polyhedralization polyhedralization = ScriptableObject.CreateInstance<Polyhedralization>();
        output.m_polyhedralization = polyhedralization;

        CreateInternal(polyhedralization, weldedTriangles, TetrahedralizerUtility.UnpackVector3s(weldedVertices));
    }
    public Task CreateAsync(PolyhedralizedMeshCreationInput input, PolyhedralizedMeshCreationOutput output, IProgress<string> progress=null)
    {
        List<Vector3> weldedVertices = input.m_mesh.vertices.ToList();
        int[] weldedTriangles = input.m_mesh.triangles;
        TetrahedralizerUtility.RemoveDuplicateVertices(weldedVertices, weldedTriangles);
        Polyhedralization polyhedralization = ScriptableObject.CreateInstance<Polyhedralization>();
        output.m_polyhedralization = polyhedralization;

        return Task.Run(() =>
        {
            CreateInternal(polyhedralization, weldedTriangles, TetrahedralizerUtility.UnpackVector3s(weldedVertices), progress);
        });
    }

    private void CreateInternal(Polyhedralization polyhedralization, int[] weldedTriangles, List<double> weldedVerticesUnpack, IProgress<string> progress=null)
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
            progress.Report("Removing outside polyhedrons.");
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

        polyhedralization.m_explicitVertices = weldedVerticesUnpack;
        polyhedralization.m_implicitVertices = BSPOutput.m_insertedVertices;
        polyhedralization.m_polyhedrons = TetrahedralizerUtility.NestedListToFlatList(TetrahedralizerUtility.FlatIListToNestedList(BSPOutput.m_polyhedrons).Where((i,j)=>0!=ICOutput.m_polyhedronsInteriorLabels[j]).ToList());
        polyhedralization.m_polyhedronsFacets = BSPOutput.m_polyhedronsFacets;
        
        // remove unused polyhedrons facets
        {
            List<List<int>> polyhedrons = TetrahedralizerUtility.FlatIListToNestedList(polyhedralization.m_polyhedrons);
            List<List<int>> polyhedronsFacets = TetrahedralizerUtility.FlatIListToNestedList(polyhedralization.m_polyhedronsFacets);
            bool[] neededFacets = Enumerable.Repeat(false, polyhedronsFacets.Count).ToArray();
            polyhedrons.ForEach(i=>i.ForEach(j=>neededFacets[j]=true));

            int[] mappings = new int[polyhedronsFacets.Count];
            List<List<int>> newPolyhedronsFacets = new List<List<int>>();
            for(int i=0; i<neededFacets.Length; i++)
            {
                if(!neededFacets[i])
                {
                    continue;
                }
                mappings[i] = newPolyhedronsFacets.Count();
                newPolyhedronsFacets.Add(polyhedronsFacets[i]);
            }

            for(int i=0; i<polyhedrons.Count; i++)
            {
                for(int j=0; j<polyhedrons[i].Count; j++)
                {
                    polyhedrons[i][j] = mappings[polyhedrons[i][j]];
                }
            }

            polyhedralization.m_polyhedrons = TetrahedralizerUtility.NestedListToFlatList(polyhedrons);
            polyhedralization.m_polyhedronsFacets = TetrahedralizerUtility.NestedListToFlatList(newPolyhedronsFacets);
        }
    }
}
