using System;
using System.Linq;
using System.Collections.Generic;
using System.Runtime.InteropServices;


namespace Hanzzz.Tetrahedralizer
{
    public class DelaunayTetrahedralization
    {
        /// <summary>
        /// Calculates a Delaunay Tetrahedralization for a set of vertices that are unique and are not all on the same plane.
        /// </summary>
        /// <param name="explicitVertices">A list of explicit vertices (x,y,z). x is right, y is up, z is forward.</param>
        /// <param name="implicitVertices">A list of implicit vertices (n, i1...in). n is 5 or 9, i1...in are indexes of explicit vertices.</param>
        /// <returns>
        /// A list of tetrahedrons (t0,t1,t2,t3) in left-hand orientation (curl your left hand around t0,t1,t2 and your thumb points towards t3).
        /// </returns>
        public List<int> CalculateDelaunayTetrahedralization(IReadOnlyList<double> explicitVertices, IReadOnlyList<int> implicitVertices)
        {
            [DllImport(TetrahedralizerConstant.TETRAHEDRALIZER_LIBRARY_NAME)]
            static extern IntPtr CreateDelaunayTetrahedralizationHandle();
            [DllImport(TetrahedralizerConstant.TETRAHEDRALIZER_LIBRARY_NAME)]
            static extern void DisposeDelaunayTetrahedralizationHandle(IntPtr handle);
            [DllImport(TetrahedralizerConstant.TETRAHEDRALIZER_LIBRARY_NAME)]
            static extern void AddDelaunayTetrahedralizationInput(IntPtr handle, int explicit_count, double[] explicit_values, int implicit_count, int[] implicit_values);
            [DllImport(TetrahedralizerConstant.TETRAHEDRALIZER_LIBRARY_NAME)]
            static extern void CalculateDelaunayTetrahedralization(IntPtr handle);
            [DllImport(TetrahedralizerConstant.TETRAHEDRALIZER_LIBRARY_NAME)]
            static extern int GetDelaunayTetrahedralizationTetrahedronsCount(IntPtr handle);
            [DllImport(TetrahedralizerConstant.TETRAHEDRALIZER_LIBRARY_NAME)]
            static extern IntPtr GetDelaunayTetrahedralizationTetrahedrons(IntPtr handle);

    
            double[] explicitVerticesArray = explicitVertices.ToArray();
            TetrahedralizerUtility.SwapElementsByInterval(explicitVerticesArray, 3); // Change from left to right hand coordinate.
            int[] implicitVerticesArray = null == implicitVertices ? null : implicitVertices.ToArray();
            int implicitVerticesCount = null == implicitVertices ? 0 : TetrahedralizerUtility.CountFlatIListElements(implicitVerticesArray);
    
            IntPtr handle = CreateDelaunayTetrahedralizationHandle();
            AddDelaunayTetrahedralizationInput(handle, explicitVerticesArray.Length/3, explicitVerticesArray, implicitVerticesCount, implicitVerticesArray);
            CalculateDelaunayTetrahedralization(handle);
    
            IntPtr ptr = GetDelaunayTetrahedralizationTetrahedrons(handle);
            List<int> tetrahedrons = ptr.ReadInt32Repeat(4*GetDelaunayTetrahedralizationTetrahedronsCount(handle));
            DisposeDelaunayTetrahedralizationHandle(handle);
    
            TetrahedralizerUtility.SwapElementsByInterval(tetrahedrons, 4); // Change from right to left hand coordinate.
            return tetrahedrons;
        }
    }
    
}