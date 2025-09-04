using System;
using System.Linq;
using System.Collections.Generic;
using System.Runtime.InteropServices;

public class GenericPointApproximation
{
    public class GenericPointApproximationInput
    {
        public IList<double> m_explicitVertices; // Every 3 doubles are x,y,z of a point. Assuming left hand coordinate.
        public IList<int> m_implicitVertices; // 5/9 followed by indices of explicit vertices
    }
    public class GenericPointApproximationOutput
    {
        public List<double> m_approximatePositions; // postions of explicit and implicit vertices in left hand coordinate
    }

    public static List<double> CalculateGenericPointApproximation(IList<double> explicitVertices, IList<int> implicitVertices)
    {
        GenericPointApproximation genericPointApproximation = new GenericPointApproximation();
        GenericPointApproximationInput input = new GenericPointApproximationInput();
        GenericPointApproximationOutput output = new GenericPointApproximationOutput();
        input.m_explicitVertices = explicitVertices;
        input.m_implicitVertices = implicitVertices;

        genericPointApproximation.CalculateGenericPointApproximation(input, output);
        return output.m_approximatePositions;
    }


    public void CalculateGenericPointApproximation(GenericPointApproximationInput input, GenericPointApproximationOutput output)
    {
        [DllImport(TetrahedralizerConstant.TETRAHEDRALIZER_LIBRARY_NAME)]
        static extern IntPtr CreateGenericPointApproximationHandle();
        [DllImport(TetrahedralizerConstant.TETRAHEDRALIZER_LIBRARY_NAME)]
        static extern void DisposeGenericPointApproximationHandle(IntPtr handle);
        [DllImport(TetrahedralizerConstant.TETRAHEDRALIZER_LIBRARY_NAME)]
        static extern void AddGenericPointApproximationVertices(IntPtr handle, int explicit_count, double[] explicit_values, int implicit_count, int[] implicit_values);
        [DllImport(TetrahedralizerConstant.TETRAHEDRALIZER_LIBRARY_NAME)]
        static extern int ApproximateGenericPoint(IntPtr handle);
        [DllImport(TetrahedralizerConstant.TETRAHEDRALIZER_LIBRARY_NAME)]
        static extern IntPtr GetOutputApproximation(IntPtr handle);


        double[] explicitVertices = input.m_explicitVertices.ToArray();
        TetrahedralizerUtility.SwapElementsByInterval(explicitVertices, 3);
        int[] implicitVertices = input.m_implicitVertices.ToArray();
        int implicit_count = TetrahedralizerUtility.CountFlatIListElements(input.m_implicitVertices);

        IntPtr handle = CreateGenericPointApproximationHandle();
        AddGenericPointApproximationVertices(handle, input.m_explicitVertices.Count/3, explicitVertices, implicit_count, implicitVertices);

        ApproximateGenericPoint(handle);

        int n = input.m_explicitVertices.Count+3*implicit_count;
        output.m_approximatePositions = new List<double>(n);
        IntPtr ptr = GetOutputApproximation(handle);
        for(int i=0; i<n; i++)
        {
            output.m_approximatePositions.Add(BitConverter.Int64BitsToDouble(Marshal.ReadInt64(ptr)));
            ptr = IntPtr.Add(ptr, 8);
        }

        DisposeGenericPointApproximationHandle(handle);

        TetrahedralizerUtility.SwapElementsByInterval(output.m_approximatePositions, 3);
    }
}
