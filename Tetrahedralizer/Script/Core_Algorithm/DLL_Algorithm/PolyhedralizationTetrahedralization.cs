using System;
using System.Linq;
using System.Collections.Generic;
using System.Runtime.InteropServices;


public class PolyhedralizationTetrahedralization
{
    public class PolyhedralizationTetrahedralizationInput
    {
        public IList<double> m_explicitVertices; // Every 3 doubles are x,y,z of a point. Assuming left hand coordinate.
        public IList<int> m_implicitVertices; // 5/9 followed by indexes of m_explicitVertices
        public List<int> m_polyhedrons; // # of polyhedron facets, followed by facets indexes.
        public List<int> m_polyhedronsFacets; // # of facets vertices, followed by vertices indexes ordered in cw or ccw.
    }
    public class PolyhedralizationTetrahedralizationOutput
    {
        public List<int> m_tetrahedrons; // Every 4 ints is a tetrahedron. Curl around the first 3 points and your thumb points toward the 4th point. Assuming left hand coordinate.
    }


    public void CalculatePolyhedralizationTetrahedralization(PolyhedralizationTetrahedralizationInput input, PolyhedralizationTetrahedralizationOutput output)
    {
        [DllImport(TetrahedralizerLibraryConstant.TETRAHEDRALIZER_LIBRARY_NAME)]
        static extern IntPtr CreatePolyhedralizationTetrahedralizationHandle();
        [DllImport(TetrahedralizerLibraryConstant.TETRAHEDRALIZER_LIBRARY_NAME)]
        static extern void DisposePolyhedralizationTetrahedralizationHandle(IntPtr handle);
        [DllImport(TetrahedralizerLibraryConstant.TETRAHEDRALIZER_LIBRARY_NAME)]
        static extern void AddPolyhedralizationTetrahedralizationInput(IntPtr handle, 
        int explicit_count, double[] explicit_values, int implicit_count, int[] implicit_values, 
        int polyhedrons_count, int[] polyhedrons, int polyhedrons_facets_count, int[] polyhedrons_facets);
        [DllImport(TetrahedralizerLibraryConstant.TETRAHEDRALIZER_LIBRARY_NAME)]
        static extern void CalculatePolyhedralizationTetrahedralization(IntPtr handle);
        [DllImport(TetrahedralizerLibraryConstant.TETRAHEDRALIZER_LIBRARY_NAME)]
        static extern int GetPolyhedralizationTetrahedralizationTetrahedronsCount(IntPtr handle);
        [DllImport(TetrahedralizerLibraryConstant.TETRAHEDRALIZER_LIBRARY_NAME)]
        static extern IntPtr GetPolyhedralizationTetrahedralizationTetrahedrons(IntPtr handle);


        double[] explicitVertices = input.m_explicitVertices.ToArray();
        TetrahedralizerLibraryUtility.SwapElementsByInterval(explicitVertices, 3); // Change from left to right hand coordinate.
        int[] implicitVertices = null == input.m_implicitVertices ? null : input.m_implicitVertices.ToArray();
        int implicitVerticesCount = null == input.m_implicitVertices ? 0 : TetrahedralizerLibraryUtility.CountFlatIListElements(implicitVertices);

        IntPtr handle = CreatePolyhedralizationTetrahedralizationHandle();

        AddPolyhedralizationTetrahedralizationInput(handle, explicitVertices.Length/3, explicitVertices, implicitVerticesCount, implicitVertices,
        TetrahedralizerLibraryUtility.CountFlatIListElements(input.m_polyhedrons), input.m_polyhedrons.ToArray(), TetrahedralizerLibraryUtility.CountFlatIListElements(input.m_polyhedronsFacets), input.m_polyhedronsFacets.ToArray());
        CalculatePolyhedralizationTetrahedralization(handle);

        int n = 4*GetPolyhedralizationTetrahedralizationTetrahedronsCount(handle);
        output.m_tetrahedrons = new List<int>(n);
        IntPtr ptr = GetPolyhedralizationTetrahedralizationTetrahedrons(handle);
        for(int i=0; i<n; i++)
        {
            output.m_tetrahedrons.Add(ptr.ReadInt32());
        }

        DisposePolyhedralizationTetrahedralizationHandle(handle);

        TetrahedralizerLibraryUtility.SwapElementsByInterval(output.m_tetrahedrons, 4); // Change from right to left hand coordinate.
    }
}
