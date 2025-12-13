using System;
using System.Linq;
using System.Collections.Generic;
using System.Threading.Tasks;
using UnityEngine;


namespace Hanzzz.Tetrahedralizer
{
    public class PolyhedralizedMeshCreation
    {
        /// <summary>
        /// Calculates a polyhedralization from a mesh.
        /// </summary>
        /// <param name="mesh">A mesh to be polyhedralized.</param>
        /// <param name="aggressivelyAddVirtualConstraints">If true, every mesh triangle can be represented as a union of polyhedrons facets. Otherwise, every polygon formed by coplanar mesh triangles can be represented as a union of polyhedrons facets.</param>
        /// <param name="polyhedronInMultiplier">Controls how many polyhedrons will be in the result. Smaller means more polyhedrons. Should be in (0,1].</param>
        /// <returns>
        /// A polyhedralization of the mesh.
        /// </returns>
        public Polyhedralization Create(Mesh mesh, bool aggressivelyAddVirtualConstraints, double polyhedronInMultiplier)
        {
            var syncRes = CreateInternalSync(mesh);
    
            CreateInternal(syncRes.polyhedralization, syncRes.weldedTriangles, TetrahedralizerUtility.UnpackVector3s(syncRes.weldedVertices), aggressivelyAddVirtualConstraints, polyhedronInMultiplier);
            return syncRes.polyhedralization;
        }
        /// <summary>
        /// Calculates a polyhedralization from a mesh.
        /// </summary>
        /// <param name="mesh">A mesh to be polyhedralized.</param>
        /// <param name="aggressivelyAddVirtualConstraints">If true, every mesh triangle can be represented as a union of polyhedrons facets. Otherwise, every polygon formed by coplanar mesh triangles can be represented as a union of polyhedrons facets.</param>
        /// <param name="polyhedronInMultiplier">Controls how many polyhedrons will be in the result. Smaller means more polyhedrons. Should be in (0,1].</param>
        /// <returns>
        /// A polyhedralization of the mesh.
        /// </returns>
        public Task<Polyhedralization> CreateAsync(Mesh mesh, bool aggressivelyAddVirtualConstraints, double polyhedronInMultiplier, IProgress<string> progress=null)
        {
            progress?.Report("Starting.");
            var syncRes = CreateInternalSync(mesh);
    
            return Task.Run(() =>
            {
                CreateInternal(syncRes.polyhedralization, syncRes.weldedTriangles, TetrahedralizerUtility.UnpackVector3s(syncRes.weldedVertices), aggressivelyAddVirtualConstraints, polyhedronInMultiplier, progress);
                return syncRes.polyhedralization;
            });
        }

        private (List<Vector3> weldedVertices, int[] weldedTriangles, Polyhedralization polyhedralization) CreateInternalSync(Mesh mesh)
        {
            List<Vector3> weldedVertices = mesh.vertices.ToList();
            int[] weldedTriangles = mesh.triangles;
            TetrahedralizerUtility.RemoveDuplicateVertices(weldedVertices, weldedTriangles);
            Polyhedralization polyhedralization = ScriptableObject.CreateInstance<Polyhedralization>();
            return (weldedVertices, weldedTriangles, polyhedralization);
        }
        private void CreateInternal(Polyhedralization polyhedralization, int[] weldedTriangles, List<double> weldedVerticesUnpack, bool aggressivelyAddVirtualConstraints, double polyhedronInMultiplier, IProgress<string> progress=null)
        {
            progress?.Report("Tetrahedralizing vertices.");
            DelaunayTetrahedralization delaunayTetrahedralization = new DelaunayTetrahedralization();
            List<int> tetrahedrons = delaunayTetrahedralization.CalculateDelaunayTetrahedralization(weldedVerticesUnpack, null);

            progress?.Report("Cutting tetrahedrons with constraints.");
            BinarySpacePartition binarySpacePartition = new BinarySpacePartition();
            var bspRes = binarySpacePartition.CalculateBinarySpacePartition(weldedVerticesUnpack, tetrahedrons, weldedTriangles, aggressivelyAddVirtualConstraints, false);

            progress?.Report("Removing outside polyhedrons.");
            InteriorCharacterization interiorCharacterization = new InteriorCharacterization();
            var icRes = interiorCharacterization.CalculateInteriorCharacterization(weldedVerticesUnpack, bspRes.insertedVertices, bspRes.polyhedrons, bspRes.facets, bspRes.facetsCentroids, bspRes.facetsCentroidsWeights, weldedTriangles, polyhedronInMultiplier);
    
            polyhedralization.m_explicitVertices = weldedVerticesUnpack;
            polyhedralization.m_implicitVertices = bspRes.insertedVertices;
            polyhedralization.m_polyhedrons = TetrahedralizerUtility.NestedListToFlatList(TetrahedralizerUtility.FlatIListToNestedList(bspRes.polyhedrons).Where((i,j)=>0!=icRes.polyhedronsLabels[j]).ToList());
            polyhedralization.m_facets = bspRes.facets;
            polyhedralization.m_facetsCentroids = bspRes.facetsCentroids;
            polyhedralization.m_facetsCentroidsWeights = bspRes.facetsCentroidsWeights;
            polyhedralization.m_facetsVerticesMapping = icRes.facetsVerticesMapping;
            polyhedralization.m_facetsCentroidsMapping = icRes.facetsCentroidsMapping;
    
            // remove unused facets
            List<List<int>> facets;
            {
                List<List<int>> newFacets = new List<List<int>>();
                List<int> newFacetsCentroids = new List<int>();
                List<double> newFacetsCentroidsWeights = new List<double>();
                List<List<List<int>>> newFacetsVerticesMapping = new List<List<List<int>>>();
                List<int> newFacetsCentroidsMapping = new List<int>();
                facets = TetrahedralizerUtility.FlatIListToNestedList(polyhedralization.m_facets);
                List<List<int>> polyhedrons = TetrahedralizerUtility.FlatIListToNestedList(polyhedralization.m_polyhedrons);
                
                List<List<List<int>>> facetsVerticesMapping = facets.Select(i=>i.Select(j=>new List<int>()).ToList()).ToList();
                {
                    int index=0;
                    for(int i=0; i<facetsVerticesMapping.Count; i++)
                    {
                        for(int j=0; j<facetsVerticesMapping[i].Count; j++)
                        {
                            int n = polyhedralization.m_facetsVerticesMapping[index++];
                            for(int k=0; k<n; k++)
                            {
                                facetsVerticesMapping[i][j].Add(polyhedralization.m_facetsVerticesMapping[index++]);
                            }
                        }
                    }
                }
                bool[] neededFacets = Enumerable.Repeat(false, facets.Count).ToArray();
                polyhedrons.ForEach(i=>i.ForEach(j=>neededFacets[j]=true));
                int[] mappings = new int[facets.Count];
                
                for(int i=0; i<neededFacets.Length; i++)
                {
                    if(!neededFacets[i])
                    {
                        continue;
                    }
                    mappings[i] = newFacets.Count();
                    
                    newFacets.Add(facets[i]);
                    newFacetsCentroids.Add(polyhedralization.m_facetsCentroids[3*i+0]);
                    newFacetsCentroids.Add(polyhedralization.m_facetsCentroids[3*i+1]);
                    newFacetsCentroids.Add(polyhedralization.m_facetsCentroids[3*i+2]);
                    newFacetsCentroidsWeights.Add(polyhedralization.m_facetsCentroidsWeights[2*i+0]);
                    newFacetsCentroidsWeights.Add(polyhedralization.m_facetsCentroidsWeights[2*i+1]);
                    newFacetsVerticesMapping.Add(facetsVerticesMapping[i]);
                    newFacetsCentroidsMapping.Add(polyhedralization.m_facetsCentroidsMapping[i]);
                }
                for(int i=0; i<polyhedrons.Count; i++)
                {
                    for(int j=0; j<polyhedrons[i].Count; j++)
                    {
                        polyhedrons[i][j] = mappings[polyhedrons[i][j]];
                    }
                }
                polyhedralization.m_polyhedrons = TetrahedralizerUtility.NestedListToFlatList(polyhedrons);
                facets = newFacets;
                polyhedralization.m_facetsCentroids = newFacetsCentroids;
                polyhedralization.m_facetsCentroidsWeights = newFacetsCentroidsWeights;
                List<int> temp = new List<int>();
                newFacetsVerticesMapping.ForEach(i=>i.ForEach(j=>{temp.Add(j.Count); j.ForEach(k=>temp.Add(k));}));
                polyhedralization.m_facetsVerticesMapping = temp;
                polyhedralization.m_facetsCentroidsMapping = newFacetsCentroidsMapping;
            }
            
            List<List<int>> implicitPoints;
            // remove unused implicit points
            {
                int explictPointsCount = polyhedralization.m_explicitVertices.Count/3;
                implicitPoints = TetrahedralizerUtility.FlatIListToNestedList(polyhedralization.m_implicitVertices);
                bool[] neededPoints = Enumerable.Repeat(false, implicitPoints.Count).ToArray();
                facets.ForEach(i=>i.ForEach(j=>{if(j>=explictPointsCount){neededPoints[j-explictPointsCount]=true;}}));
    
                int[] mappings = new int[implicitPoints.Count];
                List<List<int>> newImplicitPoints = new List<List<int>>();
                for(int i=0; i<implicitPoints.Count; i++)
                {
                    if(!neededPoints[i])
                    {
                        continue;
                    }
                    mappings[i] = newImplicitPoints.Count + explictPointsCount;
                    newImplicitPoints.Add(implicitPoints[i]);
                }
    
                for(int i=0; i<facets.Count; i++)
                {
                    for(int j=0; j<facets[i].Count; j++)
                    {
                        if(facets[i][j] >= explictPointsCount)
                        {
                            facets[i][j] = mappings[facets[i][j]-explictPointsCount];
                        }
                    }
                }
    
                implicitPoints = newImplicitPoints;
            }
    
            // remove unused explict points
            {
                int explictPointsCount = polyhedralization.m_explicitVertices.Count/3;
                bool[] neededPoints = Enumerable.Repeat(false, explictPointsCount).ToArray();
                implicitPoints.ForEach(i=>i.ForEach(j=>neededPoints[j]=true));
                facets.ForEach(i=>i.ForEach(j=>{if(j<explictPointsCount){neededPoints[j]=true;}}));
                polyhedralization.m_facetsCentroids.ForEach(i=>neededPoints[i]=true);
    
                int[] mappings = new int[explictPointsCount];
                List<double> newExplictPoints = new List<double>();
                for(int i=0; i<explictPointsCount; i++)
                {
                    if(!neededPoints[i])
                    {
                        continue;
                    }
                    mappings[i] = newExplictPoints.Count/3;
                    newExplictPoints.Add(polyhedralization.m_explicitVertices[3*i+0]);
                    newExplictPoints.Add(polyhedralization.m_explicitVertices[3*i+1]);
                    newExplictPoints.Add(polyhedralization.m_explicitVertices[3*i+2]);
                }
    
                int implicitPointsOffset = explictPointsCount - newExplictPoints.Count/3;
                for(int i=0; i<implicitPoints.Count; i++)
                {
                    for(int j=0; j<implicitPoints[i].Count; j++)
                    {
                        implicitPoints[i][j] = mappings[implicitPoints[i][j]];
                    }
                }
                for(int i=0; i<facets.Count; i++)
                {
                    for(int j=0; j<facets[i].Count; j++)
                    {
                        if(facets[i][j] < explictPointsCount)
                        {
                            facets[i][j] = mappings[facets[i][j]];
                        }
                        else
                        {
                            facets[i][j] = facets[i][j] - implicitPointsOffset;
                        }
                    }
                }
                for(int i=0; i<polyhedralization.m_facetsCentroids.Count; i++)
                {
                    polyhedralization.m_facetsCentroids[i] = mappings[polyhedralization.m_facetsCentroids[i]];
                }
                polyhedralization.m_explicitVertices = newExplictPoints;
                polyhedralization.m_implicitVertices = TetrahedralizerUtility.NestedListToFlatList(implicitPoints);
                polyhedralization.m_facets = TetrahedralizerUtility.NestedListToFlatList(facets);
            }
    
            polyhedralization.CalculateFacetsOrients();
    
            progress?.Report("Finishing up.");
        }
    }
    
}