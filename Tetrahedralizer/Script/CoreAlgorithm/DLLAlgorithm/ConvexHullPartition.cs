using System;
using System.Linq;
using System.Collections.Generic;
using System.Runtime.InteropServices;


namespace Hanzzz.Tetrahedralizer
{
    public class ConvexHullPartition
    {
        // todo
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
        public (List<int> insertedVertices, List<int> polyhedrons, List<Facet> facets, List<Segment> segments)
        CalculateConvexHullPartition(IReadOnlyList<double> explicitVertices, IReadOnlyList<int> implicitVertices, IReadOnlyList<Facet> convexHullFacets, IReadOnlyList<Facet> constraintsFacets, IReadOnlyList<Segment> segments, IReadOnlyList<int> coplanarTriangles)
        {
            [DllImport(TetrahedralizerConstant.TETRAHEDRALIZER_LIBRARY_NAME)]
            static extern IntPtr CreateConvexHullPartitionHandle();
            [DllImport(TetrahedralizerConstant.TETRAHEDRALIZER_LIBRARY_NAME)]
            static extern void DisposeConvexHullPartitionHandle(IntPtr handle);
            [DllImport(TetrahedralizerConstant.TETRAHEDRALIZER_LIBRARY_NAME)]
            static extern void AddConvexHullPartitionInput(IntPtr handle, 
            int explicit_count, double[] explicit_values, int implicit_count, int[] implicit_values, 
            int convex_hull_facets_count, FacetInteropData[] convex_hull_facets, int constraints_facets_count, FacetInteropData[] constraints_facets, 
            int segments_count, SegmentInteropData[] segments, int coplanar_triangles_count, int[] coplanar_triangles);
            [DllImport(TetrahedralizerConstant.TETRAHEDRALIZER_LIBRARY_NAME)]
            static extern void CalculateConvexHullPartition(IntPtr handle);
            [DllImport(TetrahedralizerConstant.TETRAHEDRALIZER_LIBRARY_NAME)]
            static extern int GetConvexHullPartitionInsertedVerticesCount(IntPtr handle);
            [DllImport(TetrahedralizerConstant.TETRAHEDRALIZER_LIBRARY_NAME)]
            static extern void GetConvexHullPartitionInsertedVertices(IntPtr handle, [Out] int[] out_array);
            [DllImport(TetrahedralizerConstant.TETRAHEDRALIZER_LIBRARY_NAME)]
            static extern int GetConvexHullPartitionPolyhedronsCount(IntPtr handle);
            [DllImport(TetrahedralizerConstant.TETRAHEDRALIZER_LIBRARY_NAME)]
            static extern void GetConvexHullPartitionPolyhedrons(IntPtr handle, [Out] int[] out_array);
            [DllImport(TetrahedralizerConstant.TETRAHEDRALIZER_LIBRARY_NAME)]
            static extern int GetConvexHullPartitionFacetsCount(IntPtr handle);
            [DllImport(TetrahedralizerConstant.TETRAHEDRALIZER_LIBRARY_NAME)]
            static extern void GetConvexHullPartitionFacets(IntPtr handle, [Out] FacetInteropData[] facetInteropData);
            [DllImport(TetrahedralizerConstant.TETRAHEDRALIZER_LIBRARY_NAME)]
            static extern int GetConvexHullPartitionSegmentsCount(IntPtr handle);
            [DllImport(TetrahedralizerConstant.TETRAHEDRALIZER_LIBRARY_NAME)]
            static extern void GetConvexHullPartitionSegments(IntPtr handle, [Out] SegmentInteropData[] segmentInteropData);
    
            double[] explicitVerticesArray = explicitVertices.ToArray();
            TetrahedralizerUtility.SwapElementsByInterval(explicitVerticesArray, 3);
            int[] implicitVerticesArray = null == implicitVertices ? null : implicitVertices.ToArray();
            int implicitCount = null == implicitVertices ? 0 : TetrahedralizerUtility.CountFlatIListElements(implicitVerticesArray);
    
            FacetInteropData[] convexHullFacetsInteropData = new FacetInteropData[convexHullFacets.Count];
            GCHandle[] convexHullFacetsInteropDataHandles = new GCHandle[convexHullFacets.Count];
            for(int i=0; i<convexHullFacets.Count; i++)
            {
                convexHullFacetsInteropData[i] = new FacetInteropData(convexHullFacets[i], out convexHullFacetsInteropDataHandles[i]);
            }
            FacetInteropData[] constraintsFacetsInteropData = new FacetInteropData[constraintsFacets.Count];
            GCHandle[] constraintsFacetsInteropDataHandles = new GCHandle[constraintsFacets.Count];
            for(int i=0; i<constraintsFacets.Count; i++)
            {
                constraintsFacetsInteropData[i] = new FacetInteropData(constraintsFacets[i], out constraintsFacetsInteropDataHandles[i]);
            }
            SegmentInteropData[] segmentInteropData = segments.Select(i=>new SegmentInteropData(i)).ToArray();


            IntPtr handle = CreateConvexHullPartitionHandle();
            AddConvexHullPartitionInput(handle, 
            explicitVerticesArray.Length/3, explicitVerticesArray, implicitCount, implicitVerticesArray, 
            convexHullFacets.Count, convexHullFacetsInteropData, constraintsFacets.Count, constraintsFacetsInteropData, 
            segments.Count, segmentInteropData, TetrahedralizerUtility.CountFlatIListElements(coplanarTriangles), coplanarTriangles.ToArray());
            CalculateConvexHullPartition(handle);
    
            List<int> insertedVertices = InteropUtility.GetList<int>(handle, GetConvexHullPartitionInsertedVerticesCount, GetConvexHullPartitionInsertedVertices);
            List<int> polyhedrons = InteropUtility.GetList<int>(handle, GetConvexHullPartitionPolyhedronsCount, GetConvexHullPartitionPolyhedrons);
            List<Facet> facets = InteropUtility.GetList<FacetInteropData>(handle, GetConvexHullPartitionFacetsCount, GetConvexHullPartitionFacets).Select(i=>new Facet(i)).ToList();
            List<Segment> newSegments = InteropUtility.GetList<SegmentInteropData>(handle, GetConvexHullPartitionSegmentsCount, GetConvexHullPartitionSegments).Select(i=>new Segment(i)).ToList();

            DisposeConvexHullPartitionHandle(handle);
            for(int i=0; i<convexHullFacets.Count; i++)
            {
                convexHullFacetsInteropDataHandles[i].Free();
            }
            for(int i=0; i<constraintsFacets.Count; i++)
            {
                constraintsFacetsInteropDataHandles[i].Free();
            }
            return (insertedVertices, polyhedrons, facets, newSegments);
        }
    }
    
}