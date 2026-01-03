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
            DelaunayTetrahedralization delaunayTetrahedralization = new DelaunayTetrahedralization();
            BinarySpacePartition binarySpacePartition = new BinarySpacePartition();
            ConvexHullPartition convexHullPartition = new ConvexHullPartition();
            InteriorCharacterization interiorCharacterization = new InteriorCharacterization();

            progress?.Report("Tetrahedralizing vertices.");
            List<int> tetrahedrons = delaunayTetrahedralization.CalculateDelaunayTetrahedralization(weldedVerticesUnpack, null);

            progress?.Report("Cutting tetrahedrons with constraints.");
            var bspRes = binarySpacePartition.CalculateBinarySpacePartition(weldedVerticesUnpack, tetrahedrons, weldedTriangles, aggressivelyAddVirtualConstraints);
            polyhedralization.m_polyhedrons = bspRes.polyhedrons;
            HashSet<int> convexHullFacets = polyhedralization.GetExteriorFacets().ToHashSet();

            //polyhedralization.m_explicitVertices = weldedVerticesUnpack;
            //polyhedralization.m_implicitVertices = bspRes.insertedVertices;
            //polyhedralization.m_polyhedrons = bspRes.polyhedrons;
            //polyhedralization.m_facets = bspRes.facets;
            //polyhedralization.m_segments = bspRes.segments;
            //polyhedralization.CalculateFacetsOrients();
            //return;

            progress?.Report("Removing outside polyhedrons.");
            var icRes = interiorCharacterization.CalculateInteriorCharacterization(weldedVerticesUnpack, bspRes.insertedVertices, bspRes.polyhedrons, bspRes.facets, bspRes.segments, bspRes.coplanarTriangles, weldedTriangles, polyhedronInMultiplier);
            polyhedralization.m_polyhedrons = TetrahedralizerUtility.NestedListToFlatList(TetrahedralizerUtility.FlatIListToNestedList(bspRes.polyhedrons).Where((i,j)=>0!=icRes.polyhedronsLabels[j]).ToList());
            List<Facet> constraintsFacets = polyhedralization.GetExteriorFacets().Where(i=>!convexHullFacets.Contains(i)).Select(i=>bspRes.facets[i]).ToList();
            
            //polyhedralization.m_explicitVertices = weldedVerticesUnpack;
            //polyhedralization.m_implicitVertices = bspRes.insertedVertices;
            //polyhedralization.m_facets = bspRes.facets;
            //polyhedralization.m_segments = bspRes.segments;
            //polyhedralization.CalculateFacetsOrients();
            //return;

            progress?.Report("Cutting convex hull with constraints.");
            var chpRes = convexHullPartition.CalculateConvexHullPartition(weldedVerticesUnpack, bspRes.insertedVertices, tetrahedrons, constraintsFacets, bspRes.segments, bspRes.coplanarTriangles);

            polyhedralization.m_explicitVertices = weldedVerticesUnpack;
            polyhedralization.m_implicitVertices = bspRes.insertedVertices;
            polyhedralization.m_implicitVertices.AddRange(chpRes.insertedVertices);
            progress?.Report("Removing outside polyhedrons.");
            var icRes2 = interiorCharacterization.CalculateInteriorCharacterization(polyhedralization.m_explicitVertices, polyhedralization.m_implicitVertices, chpRes.polyhedrons, chpRes.facets, chpRes.segments, bspRes.coplanarTriangles, weldedTriangles, polyhedronInMultiplier);
            polyhedralization.m_polyhedrons = TetrahedralizerUtility.NestedListToFlatList(TetrahedralizerUtility.FlatIListToNestedList(chpRes.polyhedrons).Where((i,j)=>0!=icRes2.polyhedronsLabels[j]).ToList());
            //polyhedralization.m_polyhedrons = chpRes.polyhedrons;
            polyhedralization.m_facets = chpRes.facets;
            polyhedralization.m_facetsCentroidsMapping = icRes2.facetsCentroidsMapping;
            polyhedralization.m_segments = chpRes.segments;

            polyhedralization.RemoveUnusedData();
            polyhedralization.CalculateFacetsOrients();
    
            progress?.Report("Finishing up.");
        }
    }
    
}