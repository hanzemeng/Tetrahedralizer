using System;
using System.Linq;
using System.Collections.Generic;
using System.Runtime.InteropServices;


namespace Hanzzz.Tetrahedralizer
{
    public class GenericPointApproximation
    {
        /// <summary>
        /// Calculates the approximated positions for a set of vertices.
        /// </summary>
        /// <param name="explicitVertices">A list of explicit vertices (x,y,z). x is right, y is up, z is forward.</param>
        /// <param name="implicitVertices">A list of implicit vertices (n, i1...in). n is 5 or 9, i1...in are indexes of explicit vertices.</param>
        /// <returns>
        /// A list of vertices (x,y,z). x is right, y is up, z is forward. Note that explicit vertices are duplicated.
        /// </returns>
        public List<double> CalculateGenericPointApproximation(IReadOnlyList<double> explicitVertices, IReadOnlyList<int> implicitVertices)
        {
            [DllImport(TetrahedralizerConstant.TETRAHEDRALIZER_LIBRARY_NAME)]
            static extern IntPtr CreateGenericPointApproximationHandle();
            [DllImport(TetrahedralizerConstant.TETRAHEDRALIZER_LIBRARY_NAME)]
            static extern void DisposeGenericPointApproximationHandle(IntPtr handle);
            [DllImport(TetrahedralizerConstant.TETRAHEDRALIZER_LIBRARY_NAME)]
            static extern void AddGenericPointApproximationInput(IntPtr handle, int explicit_count, double[] explicit_values, int implicit_count, int[] implicit_values);
            [DllImport(TetrahedralizerConstant.TETRAHEDRALIZER_LIBRARY_NAME)]
            static extern int CalculateGenericPointApproximation(IntPtr handle);
            [DllImport(TetrahedralizerConstant.TETRAHEDRALIZER_LIBRARY_NAME)]
            static extern IntPtr GetGenericPointApproximationPositions(IntPtr handle);
    
    
            double[] explicitVerticesArray = explicitVertices.ToArray();
            TetrahedralizerUtility.SwapElementsByInterval(explicitVerticesArray, 3); // Change from left to right hand coordinate.
            int[] implicitVerticesArray = null == implicitVertices ? null : implicitVertices.ToArray();
            int implicitVerticesCount = null == implicitVertices ? 0 : TetrahedralizerUtility.CountFlatIListElements(implicitVerticesArray);
    
            IntPtr handle = CreateGenericPointApproximationHandle();
            AddGenericPointApproximationInput(handle, explicitVerticesArray.Length/3, explicitVerticesArray, implicitVerticesCount, implicitVerticesArray);
            CalculateGenericPointApproximation(handle);
    
            IntPtr ptr = GetGenericPointApproximationPositions(handle);
            List<double> approximatedPositions = ptr.ReadDoubleRepeat(explicitVerticesArray.Length+3*implicitVerticesCount);
            DisposeGenericPointApproximationHandle(handle);
    
            TetrahedralizerUtility.SwapElementsByInterval(approximatedPositions, 3);
            return approximatedPositions;
        }
    }
    
}