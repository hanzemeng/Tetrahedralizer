using System;
using System.Linq;
using System.Collections.Generic;
using System.Runtime.InteropServices;


namespace Hanzzz.Tetrahedralizer
{
    public class BinarySpacePartition
    {
        /// <summary>
        /// Slices the tetrahedrons with intersecting constraints. Resulting in multiple polyhedrons.
        /// </summary>
        /// <param name="explicitVertices">A list of explicit vertices (x,y,z). x is right, y is up, z is forward.</param>
        /// <param name="tetrahedrons">A list of tetrahedrons (t0,t1,t2,t3) in left-hand orientation.</param>
        /// <param name="constraints">A list of constraints (c0,c1,c2). Orientation does not matter.</param>
        /// <param name="aggressivelyAddVirtualConstraints">If true, the common edges between coplanar constraints will be used to slice the tetrahedrons.</param>
        /// <param name="removeCollinearSegments">If true, collinear segments in the resulting facets will be removed.</param>
        /// <returns>
        /// A tuple containing:
        /// <para><c>insertedVertices</c>(n, i1...in). n is 5 or 9, i1...in are indexes of explicit vertices.</para>
        /// <para><c>polyhedrons</c>(n, f1...fn). n is the number of facets, f1...fn are indexes to facets.</para>
        /// <para><c>facets</c>(n, v1...vn). n is the number of vertices, v1...vn are indexes to vertices. Vertices are either in cw or ccw.</para>
        /// <para><c>facetsCentroids</c>(v0,v1,v2). v0,v1,v2 are indexes to 3 explicit vertices.</para>
        /// <para><c>facetsCentroidsWeights</c>(w0,w1). w0,w1,1-w0-w1 are weights of the 3 explicit vertices defined above.</para>
        /// </returns>
        public (List<int> insertedVertices, List<int> polyhedrons, List<int> facets, List<int> facetsCentroids, List<double> facetsCentroidsWeights)
        CalculateBinarySpacePartition(IReadOnlyList<double> explicitVertices, IReadOnlyList<int> tetrahedrons, IReadOnlyList<int> constraints, bool aggressivelyAddVirtualConstraints, bool removeCollinearSegments)
        {
            [DllImport(TetrahedralizerConstant.TETRAHEDRALIZER_LIBRARY_NAME)]
            static extern IntPtr CreateBinarySpacePartitionHandle();
            [DllImport(TetrahedralizerConstant.TETRAHEDRALIZER_LIBRARY_NAME)]
            static extern void DisposeBinarySpacePartitionHandle(IntPtr handle);
            [DllImport(TetrahedralizerConstant.TETRAHEDRALIZER_LIBRARY_NAME)]
            static extern void AddBinarySpacePartitionInput(IntPtr handle, int explicit_count, double[] explicit_values, int tetrahedron_count, int[] tetrahedrons, int constraints_count, int[] constraints, bool aggressivelyAddVirtualConstraints);
            [DllImport(TetrahedralizerConstant.TETRAHEDRALIZER_LIBRARY_NAME)]
            static extern void CalculateBinarySpacePartition(IntPtr handle);
            [DllImport(TetrahedralizerConstant.TETRAHEDRALIZER_LIBRARY_NAME)]
            static extern int GetBinarySpacePartitionInsertedVerticesCount(IntPtr handle);
            [DllImport(TetrahedralizerConstant.TETRAHEDRALIZER_LIBRARY_NAME)]
            static extern IntPtr GetBinarySpacePartitionInsertedVertices(IntPtr handle);
            [DllImport(TetrahedralizerConstant.TETRAHEDRALIZER_LIBRARY_NAME)]
            static extern int GetBinarySpacePartitionPolyhedronsCount(IntPtr handle);
            [DllImport(TetrahedralizerConstant.TETRAHEDRALIZER_LIBRARY_NAME)]
            static extern IntPtr GetBinarySpacePartitionPolyhedrons(IntPtr handle);
            [DllImport(TetrahedralizerConstant.TETRAHEDRALIZER_LIBRARY_NAME)]
            static extern int GetBinarySpacePartitionFacetsCount(IntPtr handle);
            [DllImport(TetrahedralizerConstant.TETRAHEDRALIZER_LIBRARY_NAME)]
            static extern IntPtr GetBinarySpacePartitionFacets(IntPtr handle);
            [DllImport(TetrahedralizerConstant.TETRAHEDRALIZER_LIBRARY_NAME)]
            static extern IntPtr GetBinarySpacePartitionFacetsCentroids(IntPtr handle);
            [DllImport(TetrahedralizerConstant.TETRAHEDRALIZER_LIBRARY_NAME)]
            static extern IntPtr GetBinarySpacePartitionFacetsCentroidsWeights(IntPtr handle);
    
            double[] explicitVerticesArray = explicitVertices.ToArray();
            TetrahedralizerUtility.SwapElementsByInterval(explicitVerticesArray, 3);
            int[] tetrahedronsArray = tetrahedrons.ToArray();
            TetrahedralizerUtility.SwapElementsByInterval(tetrahedronsArray, 4);
    
            IntPtr handle = CreateBinarySpacePartitionHandle();
            AddBinarySpacePartitionInput(handle, explicitVerticesArray.Length/3, explicitVerticesArray, tetrahedronsArray.Length/4, tetrahedronsArray, constraints.Count/3, constraints.ToArray(), aggressivelyAddVirtualConstraints);
            CalculateBinarySpacePartition(handle);
    
            IntPtr ptr = GetBinarySpacePartitionInsertedVertices(handle);
            List<int> insertedVertices = ptr.ReadInt32NestedRepeat(GetBinarySpacePartitionInsertedVerticesCount(handle));
            ptr = GetBinarySpacePartitionPolyhedrons(handle);
            List<int> polyhedrons = ptr.ReadInt32NestedRepeat(GetBinarySpacePartitionPolyhedronsCount(handle));
            ptr = GetBinarySpacePartitionFacets(handle);
            List<int> facets = ptr.ReadInt32NestedRepeat(GetBinarySpacePartitionFacetsCount(handle));
            ptr = GetBinarySpacePartitionFacetsCentroids(handle);
            List<int> facetsCentroids = ptr.ReadInt32Repeat(3*GetBinarySpacePartitionFacetsCount(handle));
            ptr = GetBinarySpacePartitionFacetsCentroidsWeights(handle);
            List<double> facetsCentroidsWeights = ptr.ReadDoubleRepeat(2*GetBinarySpacePartitionFacetsCount(handle));
            
            if(removeCollinearSegments)
            {
                List<List<int>> facetsNested = TetrahedralizerUtility.FlatIListToNestedList(facets);
                List<List<int>> newFacetsNested = new List<List<int>>();
                using GenericPointPredicate genericPointPredicate = new GenericPointPredicate(explicitVertices, insertedVertices);
    
                foreach(List<int> facet in facetsNested)
                {
                    List<int> newFacet = new List<int>();
                    for(int i=0; i<facet.Count; i++)
                    {
                        int p = 0==i ? facet.Count-1:i-1;
                        int n = facet.Count-1==i ? 0:i+1;
    
                        if(!genericPointPredicate.IsCollinear(facet[p],facet[i],facet[n]))
                        {
                            newFacet.Add(facet[i]);
                        }
                    }
                    newFacetsNested.Add(newFacet);
                }
    
                facets = TetrahedralizerUtility.NestedListToFlatList(newFacetsNested);
            }
    
            DisposeBinarySpacePartitionHandle(handle);
            return (insertedVertices, polyhedrons, facets, facetsCentroids, facetsCentroidsWeights);
    
            // sanity checks: every facet should not be a line, and every polyhedron should not be a plane
            //{
            //    using GenericPointPredicate genericPointPredicate = new GenericPointPredicate(input.m_explicitVertices, output.m_insertedVertices);
            //    List<List<int>> polyhedrons = TetrahedralizerUtility.FlatIListToNestedList(output.m_polyhedrons);
            //    List<List<int>> facets = TetrahedralizerUtility.FlatIListToNestedList(output.m_polyhedronsFacets);
    
            //    foreach(List<int> facet in facets)
            //    {
            //        int p0 = facet[0];
            //        int p1 = facet[1];
            //        bool facetIsDegenerate = true;
            //        for(int i=2; i<facet.Count; i++)
            //        {
            //            if(!genericPointPredicate.IsCollinear(p0,p1,facet[i]))
            //            {
            //                facetIsDegenerate = false;
            //                break;
            //            }
            //        }
            //        if(facetIsDegenerate)
            //        {
            //            throw new Exception();
            //        }
            //    }
    
            //    foreach(List<int> polyhedron in polyhedrons)
            //    {
            //        int p0,p1,p2;
            //        p0=p1=p2=-1; // find non collinear points
            //        List<int> facetVertices = facets[polyhedron[0]];
            //        for(int i=0; i<facetVertices.Count; i++)
            //        {
            //            int p = 0==i ? facetVertices.Count-1:i-1;
            //            int n = facetVertices.Count-1==i ? 0:i+1;
            //            if(!genericPointPredicate.IsCollinear(facetVertices[p],facetVertices[i],facetVertices[n]))
            //            {
            //                p0=facetVertices[p];
            //                p1=facetVertices[i];
            //                p2=facetVertices[n];
            //                break;
            //            }
            //        }
    
            //        bool polyhedronIsDegenerate = true;
            //        for(int i=1; i<polyhedron.Count; i++)
            //        {
            //            facetVertices = facets[polyhedron[i]];
            //            for(int j=0; j<facetVertices.Count; j++)
            //            {
            //                if(0 != genericPointPredicate.Orient3d(p0,p1,p2,facetVertices[j]))
            //                {
            //                    polyhedronIsDegenerate = false;
            //                    break;
            //                }
            //            }
            //        }
    
            //        if(polyhedronIsDegenerate)
            //        {
            //            throw new Exception();
            //        }
            //    }
            //}
        }
    }
    
}