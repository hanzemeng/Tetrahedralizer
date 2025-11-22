using System;
using System.Linq;
using System.Collections.Generic;
using System.Runtime.InteropServices;


namespace Hanzzz.Tetrahedralizer
{
    public class FacetAssociation
    {
        public class FacetAssociationInput
        {
            public IList<double> m_explicitVertices; // Every 3 doubles are x,y,z of a point. Assuming left hand coordinate.
            public IList<int> m_implicitVertices; // 5/9 followed by indexes of m_explicitVertices.
            public IList<int> m_facets; // # of vertices followed by indexes, vertices are ordered in cw or ccw
            public IList<int> m_constraints;
        }
        public class FacetAssociationOutput
        {
            public List<List<List<int>>> m_facetsVerticesMapping;
            // for every vertex in every facet, record # of triangles followed by indexes of the triangles
            // note that vertices are duplicated across facets
            public List<int> m_facetsCentroidsMapping;
        }
    
    
        public void CalculateFacetAssociation(FacetAssociationInput input, FacetAssociationOutput output)
        {
            [DllImport(TetrahedralizerConstant.TETRAHEDRALIZER_LIBRARY_NAME)]
            static extern IntPtr CreateFacetAssociationHandle();
            [DllImport(TetrahedralizerConstant.TETRAHEDRALIZER_LIBRARY_NAME)]
            static extern void DisposeFacetAssociationHandle(IntPtr handle);
            [DllImport(TetrahedralizerConstant.TETRAHEDRALIZER_LIBRARY_NAME)]
            static extern void AddFacetAssociationInput(IntPtr handle, int explicit_count, double[] explicit_values, int implicit_count, int[] implicit_values, int facets_count, int[] facets, int constraints_count, int[] constraints);
            [DllImport(TetrahedralizerConstant.TETRAHEDRALIZER_LIBRARY_NAME)]
            static extern int CalculateFacetAssociation(IntPtr handle);
            [DllImport(TetrahedralizerConstant.TETRAHEDRALIZER_LIBRARY_NAME)]
            static extern IntPtr GetFacetAssociationFacetsVerticesMapping(IntPtr handle);
            [DllImport(TetrahedralizerConstant.TETRAHEDRALIZER_LIBRARY_NAME)]
            static extern IntPtr GetFacetAssociationFacetsCentroidsMapping(IntPtr handle);
    
            double[] explicitVertices = input.m_explicitVertices.ToArray();
            TetrahedralizerUtility.SwapElementsByInterval(explicitVertices, 3);
            int implicitCount = TetrahedralizerUtility.CountFlatIListElements(input.m_implicitVertices);
            int[] implicitVertices = null == input.m_implicitVertices ? null : input.m_implicitVertices.ToArray();
            List<List<int>> facets = TetrahedralizerUtility.FlatIListToNestedList(input.m_facets);
    
            IntPtr handle = CreateFacetAssociationHandle();
            AddFacetAssociationInput(handle, input.m_explicitVertices.Count/3, explicitVertices, implicitCount, implicitVertices, facets.Count, input.m_facets.ToArray(), input.m_constraints.Count/3, input.m_constraints.ToArray());
            CalculateFacetAssociation(handle);
            
            IntPtr ptr = GetFacetAssociationFacetsVerticesMapping(handle);
            output.m_facetsVerticesMapping = facets.Select(i=>i.Select(j=>new List<int>()).ToList()).ToList();
            for(int i=0; i<facets.Count; i++)
            {
                for(int j=0; j<facets[i].Count; j++)
                {
                    int n = ptr.ReadInt32();
                    for(int k=0; k<n; k++)
                    {
                        output.m_facetsVerticesMapping[i][j].Add(ptr.ReadInt32());
                    }
                }
            }
            
            ptr = GetFacetAssociationFacetsCentroidsMapping(handle);
            output.m_facetsCentroidsMapping = new List<int>();
            for(int i=0; i<facets.Count; i++)
            {
                output.m_facetsCentroidsMapping.Add(ptr.ReadInt32());
            }
            DisposeFacetAssociationHandle(handle);
        }
    }
    
}