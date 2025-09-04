using System;
using System.Linq;
using System.Collections.Generic;
using System.Runtime.InteropServices;


public class DelaunayTetrahedralization
{
    public class DelaunayTetrahedralizationInput
    {
        public IList<double> m_explicitVertices; // Every 3 doubles are x,y,z of a point. Assuming left hand coordinate.
        public IList<int> m_implicitVertices; // 5/9 followed by indexes of m_explicitVertices
    }
    public class DelaunayTetrahedralizationOutput
    {
        public List<int> m_tetrahedrons; // Every 4 ints is a tetrahedron. Curl around the first 3 points and your thumb points toward the 4th point. Assuming left hand coordinate.
    }


    public void CalculateDelaunayTetrahedralization(DelaunayTetrahedralizationInput delaunayTetrahedralizationInput, DelaunayTetrahedralizationOutput delaunayTetrahedralizationOutput)
    {
        [DllImport(TetrahedralizerConstant.TETRAHEDRALIZER_LIBRARY_NAME)]
        static extern IntPtr CreateDelaunayTetrahedralizationHandle();
        [DllImport(TetrahedralizerConstant.TETRAHEDRALIZER_LIBRARY_NAME)]
        static extern void DisposeDelaunayTetrahedralizationHandle(IntPtr handle);
        [DllImport(TetrahedralizerConstant.TETRAHEDRALIZER_LIBRARY_NAME)]
        static extern void AddDelaunayTetrahedralizationVertices(IntPtr handle, int explicit_count, double[] explicit_values, int implicit_count, int[] implicit_values);
        [DllImport(TetrahedralizerConstant.TETRAHEDRALIZER_LIBRARY_NAME)]
        static extern void CalculateDelaunayTetrahedralization(IntPtr handle);
        [DllImport(TetrahedralizerConstant.TETRAHEDRALIZER_LIBRARY_NAME)]
        static extern int GetOutputTetrahedronsCount(IntPtr handle); // technically returns UInt32, but C# hates it.
        [DllImport(TetrahedralizerConstant.TETRAHEDRALIZER_LIBRARY_NAME)]
        static extern IntPtr GetOutputTetrahedrons(IntPtr handle); // technically returns UIntPtr, but C# hates it.


        double[] explicitVertices = delaunayTetrahedralizationInput.m_explicitVertices.ToArray();
        TetrahedralizerUtility.SwapElementsByInterval(explicitVertices, 3); // Change from left to right hand coordinate.
        int[] implicitVertices = null == delaunayTetrahedralizationInput.m_implicitVertices ? null : delaunayTetrahedralizationInput.m_implicitVertices.ToArray();
        int implicitVerticesCount = null == delaunayTetrahedralizationInput.m_implicitVertices ? 0 : TetrahedralizerUtility.CountFlatIListElements(implicitVertices);

        IntPtr handle = CreateDelaunayTetrahedralizationHandle();

        AddDelaunayTetrahedralizationVertices(handle, explicitVertices.Length/3, explicitVertices, implicitVerticesCount, implicitVertices);
        CalculateDelaunayTetrahedralization(handle);

        int n = 4*GetOutputTetrahedronsCount(handle);
        delaunayTetrahedralizationOutput.m_tetrahedrons = new List<int>(n);
        IntPtr ptr = GetOutputTetrahedrons(handle);
        for(int i=0; i<n; i++)
        {
            delaunayTetrahedralizationOutput.m_tetrahedrons.Add(ptr.ReadInt32());
        }

        DisposeDelaunayTetrahedralizationHandle(handle);

        TetrahedralizerUtility.SwapElementsByInterval(delaunayTetrahedralizationOutput.m_tetrahedrons, 4); // Change from right to left hand coordinate.
    }
}
