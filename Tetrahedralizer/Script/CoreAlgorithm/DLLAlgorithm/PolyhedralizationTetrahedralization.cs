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
        CalculatePolyhedralizationTetrahedralization(IReadOnlyList<double> explicitVertices, IReadOnlyList<int> implicitVertices, IReadOnlyList<int> polyhedrons, IReadOnlyList<int> facets, IReadOnlyList<int> facetsCentroids, IReadOnlyList<double> facetsCentroidsWeights)
        {
            [DllImport(TetrahedralizerConstant.TETRAHEDRALIZER_LIBRARY_NAME)]
            static extern IntPtr CreatePolyhedralizationTetrahedralizationHandle();
            [DllImport(TetrahedralizerConstant.TETRAHEDRALIZER_LIBRARY_NAME)]
            static extern void DisposePolyhedralizationTetrahedralizationHandle(IntPtr handle);
            [DllImport(TetrahedralizerConstant.TETRAHEDRALIZER_LIBRARY_NAME)]
            static extern void AddPolyhedralizationTetrahedralizationInput(IntPtr handle, 
            int explicit_count, double[] explicit_values, int implicit_count, int[] implicit_values, 
            int polyhedrons_count, int[] polyhedrons, int facets_count, int[] facets, int[] facets_centroids, double[] facets_centroids_weights);
            [DllImport(TetrahedralizerConstant.TETRAHEDRALIZER_LIBRARY_NAME)]
            static extern void CalculatePolyhedralizationTetrahedralization(IntPtr handle);
            [DllImport(TetrahedralizerConstant.TETRAHEDRALIZER_LIBRARY_NAME)]
            static extern int GetPolyhedralizationTetrahedralizationInsertedFacetsCentroidsCount(IntPtr handle);
            [DllImport(TetrahedralizerConstant.TETRAHEDRALIZER_LIBRARY_NAME)]
            static extern IntPtr GetPolyhedralizationTetrahedralizationInsertedFacetsCentroids(IntPtr handle);
            [DllImport(TetrahedralizerConstant.TETRAHEDRALIZER_LIBRARY_NAME)]
            static extern int GetPolyhedralizationTetrahedralizationInsertedPolyhedronsCentroidsCount(IntPtr handle);
            [DllImport(TetrahedralizerConstant.TETRAHEDRALIZER_LIBRARY_NAME)]
            static extern IntPtr GetPolyhedralizationTetrahedralizationInsertedPolyhedronsCentroids(IntPtr handle);
            [DllImport(TetrahedralizerConstant.TETRAHEDRALIZER_LIBRARY_NAME)]
            static extern int GetPolyhedralizationTetrahedralizationTetrahedronsCount(IntPtr handle);
            [DllImport(TetrahedralizerConstant.TETRAHEDRALIZER_LIBRARY_NAME)]
            static extern IntPtr GetPolyhedralizationTetrahedralizationTetrahedrons(IntPtr handle);
    
    
            double[] explicitVerticesArray = explicitVertices.ToArray();
            TetrahedralizerUtility.SwapElementsByInterval(explicitVerticesArray, 3); // Change from left to right hand coordinate.
            int[] implicitVerticesArray = null == implicitVertices ? null : implicitVertices.ToArray();
            int implicitVerticesCount = null == implicitVertices ? 0 : TetrahedralizerUtility.CountFlatIListElements(implicitVerticesArray);
    
            IntPtr handle = CreatePolyhedralizationTetrahedralizationHandle();
            AddPolyhedralizationTetrahedralizationInput(handle, explicitVerticesArray.Length/3, explicitVerticesArray, implicitVerticesCount, implicitVerticesArray,
            TetrahedralizerUtility.CountFlatIListElements(polyhedrons), polyhedrons.ToArray(), TetrahedralizerUtility.CountFlatIListElements(facets), facets.ToArray(), facetsCentroids.ToArray(), facetsCentroidsWeights.ToArray());
            CalculatePolyhedralizationTetrahedralization(handle);
    
            IntPtr ptr = GetPolyhedralizationTetrahedralizationInsertedFacetsCentroids(handle);
            List<int> insertedFacetsCentroids = ptr.ReadInt32Repeat(GetPolyhedralizationTetrahedralizationInsertedFacetsCentroidsCount(handle));
            ptr = GetPolyhedralizationTetrahedralizationInsertedPolyhedronsCentroids(handle);
            List<int> insertedPolyhedronsCentroids = ptr.ReadInt32Repeat(GetPolyhedralizationTetrahedralizationInsertedPolyhedronsCentroidsCount(handle));
            ptr = GetPolyhedralizationTetrahedralizationTetrahedrons(handle);
            List<int> tetrahedrons = ptr.ReadInt32Repeat(4*GetPolyhedralizationTetrahedralizationTetrahedronsCount(handle));

            DisposePolyhedralizationTetrahedralizationHandle(handle);
    
            TetrahedralizerUtility.SwapElementsByInterval(tetrahedrons, 4); // Change from right to left hand coordinate.
            return (insertedFacetsCentroids, insertedPolyhedronsCentroids, tetrahedrons);
        }
    }
    
}