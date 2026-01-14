using System;
using System.Linq;
using System.Collections.Generic;
using System.Runtime.InteropServices;


namespace Hanzzz.Tetrahedralizer
{
    public class PolyhedralizationTetrahedralization
    {
        /// <summary>
        /// Determines polyhedrons inside the constraints and assosiate facets vertices with constraints.
        /// </summary>
        /// <param name="explicitVertices">A list of explicit vertices (x,y,z). x is right, y is up, z is forward.</param>
        /// <param name="implicitVertices">A list of implicit vertices (n, i1...in). n is 5 or 9, i1...in are indexes of explicit vertices.</param>
        /// <param name="polyhedrons">A list of polyhedrons (n, f1...fn). n is the number of facets, f1...fn are indexes to facets.</param>
        /// <param name="facets">A list of facets (n, v1...vn). n is the number of vertices, v1...vn are indexes to vertices. Vertices are either in cw or ccw.</param>
        /// <param name="facetsCentroids">A list of facets centroids (v0,v1,v2). v0,v1,v2 are indexes to 3 explicit vertices.</param>
        /// <param name="facetsCentroidsWeights">A list of facets centroids weights (w0,w1). w0,w1,1-w0-w1 are weights of the 3 explicit vertices defined above.</param>
        /// <returns>
        /// A tuple containing:
        /// <para><c>insertedFacetsCentroids</c>New points are added to the centroids of the listed facets.</para>
        /// <para><c>insertedPolyhedronsCentroids</c>New points are added to the centroids of the listed polyhedrons. These points are indexed after facets centroids.</para>
        /// <para><c>tetrahedrons</c>A list of tetrahedrons (t0,t1,t2,t3) in left-hand orientation.</para>
        /// </returns>
        public (List<int> insertedFacetsCentroids, List<int> insertedPolyhedronsCentroids, List<int> tetrahedrons)
        CalculatePolyhedralizationTetrahedralization(IReadOnlyList<double> explicitVertices, IReadOnlyList<int> implicitVertices, IReadOnlyList<int> polyhedrons, IReadOnlyList<Facet> facets, IReadOnlyList<Segment> segments)
        {
            [DllImport(TetrahedralizerConstant.TETRAHEDRALIZER_LIBRARY_NAME)]
            static extern IntPtr CreatePolyhedralizationTetrahedralizationHandle();
            [DllImport(TetrahedralizerConstant.TETRAHEDRALIZER_LIBRARY_NAME)]
            static extern void DisposePolyhedralizationTetrahedralizationHandle(IntPtr handle);
            [DllImport(TetrahedralizerConstant.TETRAHEDRALIZER_LIBRARY_NAME)]
            static extern void AddPolyhedralizationTetrahedralizationInput(IntPtr handle, 
            int explicit_count, double[] explicit_values, int implicit_count, int[] implicit_values,
            int polyhedrons_count, int[] polyhedrons, int facets_count, FacetInteropData[] facets, int segments_count, SegmentInteropData[] segments);
            [DllImport(TetrahedralizerConstant.TETRAHEDRALIZER_LIBRARY_NAME)]
            static extern void CalculatePolyhedralizationTetrahedralization(IntPtr handle);
            [DllImport(TetrahedralizerConstant.TETRAHEDRALIZER_LIBRARY_NAME)]
            static extern int GetPolyhedralizationTetrahedralizationInsertedFacetsCentroidsCount(IntPtr handle);
            [DllImport(TetrahedralizerConstant.TETRAHEDRALIZER_LIBRARY_NAME)]
            static extern void GetPolyhedralizationTetrahedralizationInsertedFacetsCentroids(IntPtr handle, [Out] int[] out_array);
            [DllImport(TetrahedralizerConstant.TETRAHEDRALIZER_LIBRARY_NAME)]
            static extern int GetPolyhedralizationTetrahedralizationInsertedPolyhedronsCentroidsCount(IntPtr handle);
            [DllImport(TetrahedralizerConstant.TETRAHEDRALIZER_LIBRARY_NAME)]
            static extern void GetPolyhedralizationTetrahedralizationInsertedPolyhedronsCentroids(IntPtr handle, [Out] int[] out_array);
            [DllImport(TetrahedralizerConstant.TETRAHEDRALIZER_LIBRARY_NAME)]
            static extern int GetPolyhedralizationTetrahedralizationTetrahedronsCount(IntPtr handle);
            [DllImport(TetrahedralizerConstant.TETRAHEDRALIZER_LIBRARY_NAME)]
            static extern void GetPolyhedralizationTetrahedralizationTetrahedrons(IntPtr handle, [Out] int[] out_array);
    
    
            double[] explicitVerticesArray = explicitVertices.ToArray();
            TetrahedralizerUtility.SwapElementsByInterval(explicitVerticesArray, 3); // Change from left to right hand coordinate.
            int[] implicitVerticesArray = null == implicitVertices ? null : implicitVertices.ToArray();
            int implicitVerticesCount = null == implicitVertices ? 0 : TetrahedralizerUtility.CountFlatIListElements(implicitVerticesArray);
            
            FacetInteropData[] facetsInteropData = new FacetInteropData[facets.Count];
            GCHandle[] facetsInteropDataHandles = new GCHandle[facets.Count];
            for(int i=0; i<facets.Count; i++)
            {
                facetsInteropData[i] = new FacetInteropData(facets[i], out facetsInteropDataHandles[i]);
            }
            SegmentInteropData[] segmentInteropData = segments.Select(i=>new SegmentInteropData(i)).ToArray();

            IntPtr handle = CreatePolyhedralizationTetrahedralizationHandle();
            AddPolyhedralizationTetrahedralizationInput(handle, explicitVerticesArray.Length/3, explicitVerticesArray, implicitVerticesCount, implicitVerticesArray,
            TetrahedralizerUtility.CountFlatIListElements(polyhedrons), polyhedrons.ToArray(), facets.Count, facetsInteropData.ToArray(), segments.Count, segmentInteropData.ToArray());
            CalculatePolyhedralizationTetrahedralization(handle);
    
            List<int> insertedFacetsCentroids = InteropUtility.GetList<int>(handle, GetPolyhedralizationTetrahedralizationInsertedFacetsCentroidsCount, GetPolyhedralizationTetrahedralizationInsertedFacetsCentroids);
            List<int> insertedPolyhedronsCentroids = InteropUtility.GetList<int>(handle, GetPolyhedralizationTetrahedralizationInsertedPolyhedronsCentroidsCount, GetPolyhedralizationTetrahedralizationInsertedPolyhedronsCentroids);
            List<int> tetrahedrons = InteropUtility.GetList<int>(handle, GetPolyhedralizationTetrahedralizationTetrahedronsCount, GetPolyhedralizationTetrahedralizationTetrahedrons);

            DisposePolyhedralizationTetrahedralizationHandle(handle);
            foreach(GCHandle gCHandle in facetsInteropDataHandles)
            {
                gCHandle.Free();
            }
    
            TetrahedralizerUtility.SwapElementsByInterval(tetrahedrons, 4); // Change from right to left hand coordinate.
            return (insertedFacetsCentroids, insertedPolyhedronsCentroids, tetrahedrons);
        }
    }
    
}