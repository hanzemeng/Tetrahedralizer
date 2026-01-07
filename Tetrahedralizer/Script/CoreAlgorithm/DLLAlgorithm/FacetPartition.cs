using System;
using System.Linq;
using System.Collections.Generic;
using System.Runtime.InteropServices;


namespace Hanzzz.Tetrahedralizer
{
    public class FacetPartition
    {
        // todo
        /// <summary>
        /// Slices the tetrahedrons with intersecting constraints. Resulting in multiple polyhedrons.
        /// </summary>
        /// <param name="explicitVertices">A list of explicit vertices (x,y,z). x is right, y is up, z is forward.</param>
        /// <param name="tetrahedrons">A list of tetrahedrons (t0,t1,t2,t3) in left-hand orientation.</param>
        /// <param name="constraints">A list of constraints (c0,c1,c2). Orientation does not matter.</param>
        /// <param name="aggressivelyAddVirtualConstraints">If true, the common edges between coplanar constraints will be used to slice the tetrahedrons.</param>
        /// <returns>
        /// A tuple containing:
        /// <para><c>insertedVertices</c>(n, i1...in). n is 5 or 9, i1...in are indexes of explicit vertices.</para>
        /// <para><c>polyhedrons</c>(n, f1...fn). n is the number of facets, f1...fn are indexes to facets.</para>
        /// <para><c>facets</c>(n, v1...vn). n is the number of vertices, v1...vn are indexes to vertices. Vertices are either in cw or ccw.</para>
        /// <para><c>facetsCentroids</c>(v0,v1,v2). v0,v1,v2 are indexes to 3 explicit vertices.</para>
        /// <para><c>facetsCentroidsWeights</c>(w0,w1). w0,w1,1-w0-w1 are weights of the 3 explicit vertices defined above.</para>
        /// </returns>
        public (List<int> insertedVertices, List<int> polyhedrons, List<Facet> facets, List<int> facetsCentroidsMapping, List<Segment> segments)
        CalculateFacetPartition(IReadOnlyList<double> explicitVertices, IReadOnlyList<int> implicitVertices,
        IReadOnlyList<int> polyhedrons, IReadOnlyList<Facet> facets, IReadOnlyList<int> facetsCentroidsMapping, IReadOnlyList<Segment> segments,
        IReadOnlyList<int> coplanarTriangles, IReadOnlyList<int> constraints)
        {
            [DllImport(TetrahedralizerConstant.TETRAHEDRALIZER_LIBRARY_NAME)]
            static extern IntPtr CreateFacetPartitionHandle();
            [DllImport(TetrahedralizerConstant.TETRAHEDRALIZER_LIBRARY_NAME)]
            static extern void DisposeFacetPartitionHandle(IntPtr handle);
            [DllImport(TetrahedralizerConstant.TETRAHEDRALIZER_LIBRARY_NAME)]
            static extern void AddFacetPartitionInput(IntPtr handle, 
            int explicit_count, double[] explicit_values, int implicit_count, int[] implicit_values, 
            int polyhedrons_count, int[] polyhedrons, int facets_count, FacetInteropData[] facets, int[] facetsCentroidsMapping, int segments_count, SegmentInteropData[] segments,
            int coplanar_triangles_count, int[] coplanar_triangles, int constraints_count, int[] constraints);
            [DllImport(TetrahedralizerConstant.TETRAHEDRALIZER_LIBRARY_NAME)]
            static extern void CalculateFacetPartition(IntPtr handle);
            [DllImport(TetrahedralizerConstant.TETRAHEDRALIZER_LIBRARY_NAME)]
            static extern int GetFacetPartitionInsertedVerticesCount(IntPtr handle);
            [DllImport(TetrahedralizerConstant.TETRAHEDRALIZER_LIBRARY_NAME)]
            static extern void GetFacetPartitionInsertedVertices(IntPtr handle, [Out] int[] out_array);
            [DllImport(TetrahedralizerConstant.TETRAHEDRALIZER_LIBRARY_NAME)]
            static extern int GetFacetPartitionPolyhedronsCount(IntPtr handle);
            [DllImport(TetrahedralizerConstant.TETRAHEDRALIZER_LIBRARY_NAME)]
            static extern void GetFacetPartitionPolyhedrons(IntPtr handle, [Out] int[] out_array);
            [DllImport(TetrahedralizerConstant.TETRAHEDRALIZER_LIBRARY_NAME)]
            static extern int GetFacetPartitionFacetsCount(IntPtr handle);
            [DllImport(TetrahedralizerConstant.TETRAHEDRALIZER_LIBRARY_NAME)]
            static extern void GetFacetPartitionFacets(IntPtr handle, [Out] FacetInteropData[] facetInteropData);
            [DllImport(TetrahedralizerConstant.TETRAHEDRALIZER_LIBRARY_NAME)]
            static extern void GetFacetPartitionFacetsCentrodisMapping(IntPtr handle, [Out] int[] out_array);
            [DllImport(TetrahedralizerConstant.TETRAHEDRALIZER_LIBRARY_NAME)]
            static extern int GetFacetPartitionSegmentsCount(IntPtr handle);
            [DllImport(TetrahedralizerConstant.TETRAHEDRALIZER_LIBRARY_NAME)]
            static extern void GetFacetPartitionSegments(IntPtr handle, [Out] SegmentInteropData[] segmentInteropData);
    
            double[] explicitVerticesArray = explicitVertices.ToArray();
            TetrahedralizerUtility.SwapElementsByInterval(explicitVerticesArray, 3);
            int[] implicitVerticesArray = null == implicitVertices ? null : implicitVertices.ToArray();
            int implicitCount = null == implicitVertices ? 0 : TetrahedralizerUtility.CountFlatIListElements(implicitVerticesArray);
            FacetInteropData[] facetsInteropData = new FacetInteropData[facets.Count];
            GCHandle[] facetsInteropDataHandles = new GCHandle[facets.Count];
            for(int i=0; i<facets.Count; i++)
            {
                facetsInteropData[i] = new FacetInteropData(facets[i], out facetsInteropDataHandles[i]);
            }
            SegmentInteropData[] segmentInteropData = segments.Select(i=>new SegmentInteropData(i)).ToArray();
            int[] constraintsArray = constraints.ToArray();
            TetrahedralizerUtility.SwapElementsByInterval(constraintsArray, 3);
    
            IntPtr handle = CreateFacetPartitionHandle();
            AddFacetPartitionInput(handle, 
            explicitVerticesArray.Length/3, explicitVerticesArray, implicitCount, implicitVerticesArray, 
            TetrahedralizerUtility.CountFlatIListElements(polyhedrons), polyhedrons.ToArray(), facets.Count, facetsInteropData, facetsCentroidsMapping.ToArray(), segments.Count, segmentInteropData,
            TetrahedralizerUtility.CountFlatIListElements(coplanarTriangles), coplanarTriangles.ToArray(), constraintsArray.Length/3, constraintsArray);
            CalculateFacetPartition(handle);
   
            List<int> insertedVertices = InteropUtility.GetList<int>(handle, GetFacetPartitionInsertedVerticesCount, GetFacetPartitionInsertedVertices);
            List<int> newPolyhedrons = InteropUtility.GetList<int>(handle, GetFacetPartitionPolyhedronsCount, GetFacetPartitionPolyhedrons);
            List<Facet> newFacets = InteropUtility.GetList<FacetInteropData>(handle, GetFacetPartitionFacetsCount, GetFacetPartitionFacets).Select(i=>new Facet(i)).ToList();
            List<int> newFacetsCentroidsMapping = InteropUtility.GetList<int>(handle, GetFacetPartitionFacetsCount, GetFacetPartitionFacetsCentrodisMapping);
            List<Segment> newSegments = InteropUtility.GetList<SegmentInteropData>(handle, GetFacetPartitionSegmentsCount, GetFacetPartitionSegments).Select(i=>new Segment(i)).ToList();
            DisposeFacetPartitionHandle(handle);
            foreach(GCHandle gCHandle in facetsInteropDataHandles)
            {
                gCHandle.Free();
            }

            return (insertedVertices, newPolyhedrons, newFacets, newFacetsCentroidsMapping, newSegments);
        }
    }
    
}