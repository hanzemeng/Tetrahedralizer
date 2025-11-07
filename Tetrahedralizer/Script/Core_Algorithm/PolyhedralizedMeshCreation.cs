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
        public bool m_aggressivelyAddVirtualConstraints; // If true, every constraint can be represented as a union of polyhedrons facets. Otherwise, every polygon formed by coplanar constraints can be represented as a union of polyhedrons facets.
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

        CreateInternal(polyhedralization, weldedTriangles, TetrahedralizerUtility.UnpackVector3s(weldedVertices), input.m_aggressivelyAddVirtualConstraints);
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
            CreateInternal(polyhedralization, weldedTriangles, TetrahedralizerUtility.UnpackVector3s(weldedVertices), input.m_aggressivelyAddVirtualConstraints, progress);
        });
    }

    private void CreateInternal(Polyhedralization polyhedralization, int[] weldedTriangles, List<double> weldedVerticesUnpack, bool aggressivelyAddVirtualConstraints, IProgress<string> progress=null)
    {
        if(null != progress)
        {
            progress.Report("Tetrahedralizing vertices.");
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
            progress.Report("Cutting tetrahedrons with constraints.");
        }
        BinarySpacePartition.BinarySpacePartitionInput BSPInput = new BinarySpacePartition.BinarySpacePartitionInput();
        BinarySpacePartition.BinarySpacePartitionOutput BSPOutput = new BinarySpacePartition.BinarySpacePartitionOutput();
        {
            
            BinarySpacePartition binarySpacePartition = new BinarySpacePartition();
            BSPInput.m_explicitVertices = weldedVerticesUnpack;
            BSPInput.m_tetrahedrons = DTOutput.m_tetrahedrons;
            BSPInput.m_constraints = weldedTriangles;
            BSPInput.m_aggressivelyAddVirtualConstraints = aggressivelyAddVirtualConstraints;
            BSPInput.m_removeCollinearSegments = true;
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
        
        // remove unused polyhedrons facets, implicit vertices, and explicit vertices
        List<List<int>> newPolyhedronsFacets = new List<List<int>>();
        {
            List<List<int>> polyhedrons = TetrahedralizerUtility.FlatIListToNestedList(polyhedralization.m_polyhedrons);
            List<List<int>> polyhedronsFacets = TetrahedralizerUtility.FlatIListToNestedList(polyhedralization.m_polyhedronsFacets);
            bool[] neededFacets = Enumerable.Repeat(false, polyhedronsFacets.Count).ToArray();
            polyhedrons.ForEach(i=>i.ForEach(j=>neededFacets[j]=true));
            int[] mappings = new int[polyhedronsFacets.Count];
            
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
        }

        {
            int explictPointsCount = polyhedralization.m_explicitVertices.Count/3;
            List<List<int>> implicitPoints = TetrahedralizerUtility.FlatIListToNestedList(polyhedralization.m_implicitVertices);
            bool[] neededPoints = Enumerable.Repeat(false, implicitPoints.Count).ToArray();
            newPolyhedronsFacets.ForEach(i=>i.ForEach(j=>{if(j>=explictPointsCount){neededPoints[j-explictPointsCount]=true;}}));

            int[] implicitMappings = new int[neededPoints.Count()];
            List<List<int>> newImplicitPoints = new List<List<int>>();
            for(int i=0; i<implicitPoints.Count; i++)
            {
                if(!neededPoints[i])
                {
                    continue;
                }
                implicitMappings[i] = newImplicitPoints.Count;
                newImplicitPoints.Add(implicitPoints[i]);
            }

            neededPoints = Enumerable.Repeat(false, explictPointsCount).ToArray();
            newPolyhedronsFacets.ForEach(i=>i.ForEach(j=>{if(j<explictPointsCount){neededPoints[j]=true;}}));
            newImplicitPoints.ForEach(i=>i.ForEach(j=>neededPoints[j]=true));

            int[] explicitMappings = new int[neededPoints.Count()];
            List<double> newExplictPoints = new List<double>();
            for(int i=0; i<explictPointsCount; i++)
            {
                if(!neededPoints[i])
                {
                    continue;
                }
                explicitMappings[i] = newExplictPoints.Count/3;
                newExplictPoints.Add(polyhedralization.m_explicitVertices[3*i+0]);
                newExplictPoints.Add(polyhedralization.m_explicitVertices[3*i+1]);
                newExplictPoints.Add(polyhedralization.m_explicitVertices[3*i+2]);
            }

            int newExplictPointsCount = newExplictPoints.Count/3;
            for(int i=0; i<newImplicitPoints.Count; i++)
            {
                for(int j=0; j<newImplicitPoints[i].Count; j++)
                {
                    newImplicitPoints[i][j] = explicitMappings[newImplicitPoints[i][j]];
                }
            }
            for(int i=0; i<newPolyhedronsFacets.Count; i++)
            {
                for(int j=0; j<newPolyhedronsFacets[i].Count; j++)
                {
                    if(newPolyhedronsFacets[i][j] < explictPointsCount)
                    {
                        newPolyhedronsFacets[i][j] = explicitMappings[newPolyhedronsFacets[i][j]];
                    }
                    else
                    {
                        newPolyhedronsFacets[i][j] = implicitMappings[newPolyhedronsFacets[i][j]-explictPointsCount]+newExplictPointsCount;
                    }
                }
            }

            polyhedralization.m_explicitVertices = newExplictPoints;
            polyhedralization.m_implicitVertices = TetrahedralizerUtility.NestedListToFlatList(newImplicitPoints);
            polyhedralization.m_polyhedronsFacets = TetrahedralizerUtility.NestedListToFlatList(newPolyhedronsFacets);
            polyhedralization.CalculatePolyhedronsFacetsOrients();
        }
    }
}
