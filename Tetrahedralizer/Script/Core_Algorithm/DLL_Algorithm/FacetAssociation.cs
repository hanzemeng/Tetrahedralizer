using System;
using System.Linq;
using System.Collections.Generic;
using System.Runtime.InteropServices;

public class FacetAssociation
{
    public class FacetAssociationInput
    {
        public IList<double> m_explicitVertices; // Every 3 doubles are x,y,z of a point. Assuming left hand coordinate.
        public IList<int> m_implicitVertices; // 5/9 followed by indexes of m_explicitVertices.
        public IList<int> m_tetrahedrons;
        public IList<int> m_constraints;
    }
    public class FacetAssociationOutput
    {
        public List<int> m_tetrahedronsFacetsMapping;
        // for every tetrahedron (t), for every facet (f) of t, for every vertex (v) of f:
        // v is contained in the m_tetrahedrons_facets_mapping[v]th constraint. v is UNDEFINED_VALUE if it is not contained in any constraint.
        // internally, facets are checked in: 0,1,2  1,0,3  0,2,3, 2,1,3
        // but output order is              : 0,1,3  1,0,2  0,3,2, 3,1,2
    }


    public void CalculateFacetAssociation(FacetAssociationInput input, FacetAssociationOutput output)
    {
        [DllImport(TetrahedralizerLibraryConstant.TETRAHEDRALIZER_LIBRARY_NAME)]
        static extern IntPtr CreateFacetAssociationHandle();
        [DllImport(TetrahedralizerLibraryConstant.TETRAHEDRALIZER_LIBRARY_NAME)]
        static extern void DisposeFacetAssociationHandle(IntPtr handle);
        [DllImport(TetrahedralizerLibraryConstant.TETRAHEDRALIZER_LIBRARY_NAME)]
        static extern void AddFacetAssociationInput(IntPtr handle, int explicit_count, double[] explicit_values, int implicit_count, int[] implicit_values, int tetrahedron_count, int[] tetrahedrons, int constraints_count, int[] constraints);
        [DllImport(TetrahedralizerLibraryConstant.TETRAHEDRALIZER_LIBRARY_NAME)]
        static extern int CalculateFacetAssociation(IntPtr handle);
        [DllImport(TetrahedralizerLibraryConstant.TETRAHEDRALIZER_LIBRARY_NAME)]
        static extern IntPtr GetOutputFacetAssociation(IntPtr handle);

        double[] explicitVertices = input.m_explicitVertices.ToArray();
        TetrahedralizerLibraryUtility.SwapElementsByInterval(explicitVertices, 3);
        int implicit_count = TetrahedralizerLibraryUtility.CountFlatIListElements(input.m_implicitVertices);
        int[] implicitVertices = null == input.m_implicitVertices ? null : input.m_implicitVertices.ToArray();
        int[] tetrahedrons = input.m_tetrahedrons.ToArray();
        TetrahedralizerLibraryUtility.SwapElementsByInterval(tetrahedrons, 4);

        IntPtr handle = CreateFacetAssociationHandle();
        AddFacetAssociationInput(handle, input.m_explicitVertices.Count/3, explicitVertices, implicit_count, implicitVertices, input.m_tetrahedrons.Count/4, tetrahedrons, input.m_constraints.Count/3, input.m_constraints.ToArray());

        CalculateFacetAssociation(handle);

        IntPtr ptr = GetOutputFacetAssociation(handle);

        output.m_tetrahedronsFacetsMapping = new List<int>(3*input.m_tetrahedrons.Count);
        for(int i=0; i<3*input.m_tetrahedrons.Count; i++)
        {
            output.m_tetrahedronsFacetsMapping.Add(ptr.ReadInt32());
        }

        DisposeFacetAssociationHandle(handle);
    }
}
