using System;
using System.Linq;
using System.Collections.Generic;
using System.Runtime.InteropServices;


namespace Hanzzz.Tetrahedralizer
{
    public class InteriorCharacterization
    {
        // todo
        /// <summary>
        /// Determines polyhedrons inside the constraints and assosiate facets vertices with constraints.
        /// </summary>
        /// <param name="explicitVertices">A list of explicit vertices (x,y,z). x is right, y is up, z is forward.</param>
        /// <param name="implicitVertices">A list of implicit vertices (n, i1...in). n is 5 or 9, i1...in are indexes of explicit vertices.</param>
        /// <param name="polyhedrons">A list of polyhedrons (n, f1...fn). n is the number of facets, f1...fn are indexes to facets.</param>
        /// <param name="facets">A list of facets (n, v1...vn). n is the number of vertices, v1...vn are indexes to vertices. Vertices are either in cw or ccw.</param>
        /// <param name="facetsCentroids">A list of facets centroids (v0,v1,v2). v0,v1,v2 are indexes to 3 explicit vertices.</param>
        /// <param name="facetsCentroidsWeights">A list of facets centroids weights (w0,w1). w0,w1,1-w0-w1 are weights of the 3 explicit vertices defined above.</param>
        /// <param name="constraints">A list of constraints (c0,c1,c2). Left-hand orientation.</param>
        /// <param name="polyhedronInMultiplier">A multiplier (0,1] to mark a polyhedron as in. Smaller means more likely to be in.</param>
        /// <returns>
        /// A tuple containing:
        /// <para><c>polyhedronsLabels</c>0 if the polyhedron is outside, otherwise inside.</para>
        /// <para><c>facetsVerticesMapping</c>for every vertex in every facet: (n, c1...cn). n is the number of constraints, c1...cn are indexes to the coplanar constraints that intersect the vertex.</para>
        /// <para><c>facetsCentroidsMapping</c>for every facet centroid: record a coplanar constraint that touches it. UNDEFINED_VALUE if no such constraint.</para>
        /// </returns>
        public List<int> CalculateInteriorCharacterization(IReadOnlyList<double> explicitVertices, IReadOnlyList<int> implicitVertices, IReadOnlyList<int> polyhedrons, IReadOnlyList<Facet> facets, IReadOnlyList<Segment> segments, IReadOnlyList<int> coplanarTriangles, IReadOnlyList<int> constraints, double polyhedronInMultiplier)
        {
            [DllImport(TetrahedralizerConstant.TETRAHEDRALIZER_LIBRARY_NAME)]
            static extern IntPtr CreateInteriorCharacterizationHandle();
            [DllImport(TetrahedralizerConstant.TETRAHEDRALIZER_LIBRARY_NAME)]
            static extern void DisposeInteriorCharacterizationHandle(IntPtr handle);
            [DllImport(TetrahedralizerConstant.TETRAHEDRALIZER_LIBRARY_NAME)]
            static extern void AddInteriorCharacterizationInput(IntPtr handle, 
            int explicit_count, double[] explicit_values, int implicit_count, int[] implicit_values, 
            int polyhedrons_count, int[] polyhedrons, int facets_count, FacetInteropData[] facets, int segments_count, SegmentInteropData[] segments,
            int coplanar_triangles_count, int[] coplanar_triangles, int constraints_count, int[] constraints, double polyhedronInMultiplier);
            [DllImport(TetrahedralizerConstant.TETRAHEDRALIZER_LIBRARY_NAME)]
            static extern void CalculateInteriorCharacterization(IntPtr handle);
            [DllImport(TetrahedralizerConstant.TETRAHEDRALIZER_LIBRARY_NAME)]
            static extern void GetInteriorCharacterizationPolyhedronsLabels(IntPtr handle, [Out] int[] labels);
            int GetPolyhedronsCount(IntPtr _)
            {
                return TetrahedralizerUtility.CountFlatIListElements(polyhedrons);
            }

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
    
            IntPtr handle = CreateInteriorCharacterizationHandle();
            AddInteriorCharacterizationInput(handle, 
            explicitVerticesArray.Length/3, explicitVerticesArray, implicitCount, implicitVerticesArray, 
            GetPolyhedronsCount(handle), polyhedrons.ToArray(), facets.Count, facetsInteropData, segments.Count, segmentInteropData,
            TetrahedralizerUtility.CountFlatIListElements(coplanarTriangles), coplanarTriangles.ToArray(), constraintsArray.Length/3, constraintsArray, polyhedronInMultiplier);
            CalculateInteriorCharacterization(handle);
   
            List<int> polyhedronsLabels = InteropUtility.GetList<int>(handle, GetPolyhedronsCount, GetInteriorCharacterizationPolyhedronsLabels);
            DisposeInteriorCharacterizationHandle(handle);
            foreach(GCHandle gCHandle in facetsInteropDataHandles)
            {
                gCHandle.Free();
            }

            return polyhedronsLabels;
        }
    }
    
}