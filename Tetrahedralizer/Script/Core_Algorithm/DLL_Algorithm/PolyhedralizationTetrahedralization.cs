using System;
using System.Linq;
using System.Collections.Generic;
using System.Runtime.InteropServices;


namespace Hanzzz.Tetrahedralizer
{
    public class PolyhedralizationTetrahedralization
    {
        public class PolyhedralizationTetrahedralizationInput
        {
            public IList<double> m_explicitVertices; // Every 3 doubles are x,y,z of a point. Assuming left hand coordinate.
            public IList<int> m_implicitVertices; // 5/9 followed by indexes of m_explicitVertices
            public List<int> m_polyhedrons; // # of polyhedron facets, followed by facets indexes.
            public List<int> m_facets; // # of facets vertices, followed by vertices indexes ordered in cw or ccw.
            public List<int> m_facetsCentroids; // every facet centroid is defined by 3 coplanar explicit vertices
            public List<double> m_facetsCentroidsWeights; // and the weight of the explicit vertices, note the 3rd weight is ignored
        }
        public class PolyhedralizationTetrahedralizationOutput
        {
            public List<int> m_insertedFacetsCentroids; // new points are added to the centroids of the listed facets
            public List<int> m_insertedPolyhedronsCentroids; // new points are added to the centroids of the listed polyhedrons, indexed after facets centroids
            public List<int> m_tetrahedrons; // Every 4 ints is a tetrahedron. Curl around the first 3 points and your thumb points toward the 4th point. Assuming left hand coordinate.
        }
    
    
        public void CalculatePolyhedralizationTetrahedralization(PolyhedralizationTetrahedralizationInput input, PolyhedralizationTetrahedralizationOutput output)
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
    
    
            double[] explicitVertices = input.m_explicitVertices.ToArray();
            TetrahedralizerUtility.SwapElementsByInterval(explicitVertices, 3); // Change from left to right hand coordinate.
            int[] implicitVertices = null == input.m_implicitVertices ? null : input.m_implicitVertices.ToArray();
            int implicitVerticesCount = null == input.m_implicitVertices ? 0 : TetrahedralizerUtility.CountFlatIListElements(implicitVertices);
    
            IntPtr handle = CreatePolyhedralizationTetrahedralizationHandle();
    
            AddPolyhedralizationTetrahedralizationInput(handle, explicitVertices.Length/3, explicitVertices, implicitVerticesCount, implicitVertices,
            TetrahedralizerUtility.CountFlatIListElements(input.m_polyhedrons), input.m_polyhedrons.ToArray(), TetrahedralizerUtility.CountFlatIListElements(input.m_facets), input.m_facets.ToArray(), input.m_facetsCentroids.ToArray(), input.m_facetsCentroidsWeights.ToArray());
            CalculatePolyhedralizationTetrahedralization(handle);
    
            void PopulateOutput(int n, IntPtr ptr, ref List<int> res)
            {
                res = new List<int>();
                for(int i=0; i<n; i++)
                {
                    res.Add(ptr.ReadInt32());
                }
            }
    
            PopulateOutput(GetPolyhedralizationTetrahedralizationInsertedFacetsCentroidsCount(handle), GetPolyhedralizationTetrahedralizationInsertedFacetsCentroids(handle), ref output.m_insertedFacetsCentroids);
            PopulateOutput(GetPolyhedralizationTetrahedralizationInsertedPolyhedronsCentroidsCount(handle), GetPolyhedralizationTetrahedralizationInsertedPolyhedronsCentroids(handle), ref output.m_insertedPolyhedronsCentroids);
            PopulateOutput(4*GetPolyhedralizationTetrahedralizationTetrahedronsCount(handle), GetPolyhedralizationTetrahedralizationTetrahedrons(handle), ref output.m_tetrahedrons);
    
            DisposePolyhedralizationTetrahedralizationHandle(handle);
    
            TetrahedralizerUtility.SwapElementsByInterval(output.m_tetrahedrons, 4); // Change from right to left hand coordinate.
        }
    }
    
}