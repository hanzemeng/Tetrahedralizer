using System;
using System.Linq;
using System.Collections.Generic;
using System.Runtime.InteropServices;


namespace Hanzzz.Tetrahedralizer
{
    public class InteriorCharacterization
    {
        public class InteriorCharacterizationInput
        {
            public IList<double> m_explicitVertices; // Every 3 doubles are x,y,z of a point. Assuming left hand coordinate.
            public IList<int> m_implicitVertices; // 5/9 followed by indexes of m_explicitVertices.
            public IList<int> m_polyhedrons; // # of polyhedron facets, followed by facets indexes.
            public IList<int> m_facets; // # of facets vertices, followed by vertices indexes ordered in cw or ccw.
            public IList<int> m_facetsCentroids; // every facet centroid is defined by 3 coplanar explicit vertices
            public IList<double> m_facetsCentroidsWeights; // and the weight of the explicit vertices, note the 3rd weight is ignored
            public IList<int> m_constraints; // Every 3 ints is a constraint (triangle). Curl around the points and your thumb points outward. Assuming left hand coordinate.
            public double m_polyhedronInMultiplier; // Multiply this to the cost of labeling a polyhedron to be in, should be in (0,1)
            
        }
        public class InteriorCharacterizationOutput
        {
            public List<int> m_polyhedronsLabels; // 0 if outside, otherwise inside.
            public List<int> m_facetsVerticesMapping;
            // for every vertex in every facet, record # of triangles followed by indexes of the triangles
            // note that vertices are duplicated across facets
            public List<int> m_facetsCentroidsMapping;
            // for every facet centroid, an incident triangle will be recorded, UNDEFINED_VALUE if no such triangle
        }
    
    
        public void CalculateInteriorCharacterization(InteriorCharacterizationInput input, InteriorCharacterizationOutput output)
        {
            [DllImport(TetrahedralizerConstant.TETRAHEDRALIZER_LIBRARY_NAME)]
            static extern IntPtr CreateInteriorCharacterizationHandle();
            [DllImport(TetrahedralizerConstant.TETRAHEDRALIZER_LIBRARY_NAME)]
            static extern void DisposeInteriorCharacterizationHandle(IntPtr handle);
            [DllImport(TetrahedralizerConstant.TETRAHEDRALIZER_LIBRARY_NAME)]
            static extern void AddInteriorCharacterizationInput(IntPtr handle, 
            int explicit_count, double[] explicit_values, int implicit_count, int[] implicit_values, 
            int polyhedrons_count, int[] polyhedrons, int facets_count, int[] facets, int[] facets_centroids, double[] facets_centroids_weights,
            int constraints_count, int[] constraints, double polyhedronInMultiplier);
            [DllImport(TetrahedralizerConstant.TETRAHEDRALIZER_LIBRARY_NAME)]
            static extern void CalculateInteriorCharacterization(IntPtr handle);
            [DllImport(TetrahedralizerConstant.TETRAHEDRALIZER_LIBRARY_NAME)]
            static extern IntPtr GetInteriorCharacterizationPolyhedronsLabels(IntPtr handle);
            [DllImport(TetrahedralizerConstant.TETRAHEDRALIZER_LIBRARY_NAME)]
            static extern IntPtr GetInteriorCharacterizationFacetsVerticesMapping(IntPtr handle);
            [DllImport(TetrahedralizerConstant.TETRAHEDRALIZER_LIBRARY_NAME)]
            static extern IntPtr GetInteriorCharacterizationFacetsCentroidsMapping(IntPtr handle);
    
    
            double[] explicitVertices = input.m_explicitVertices.ToArray();
            TetrahedralizerUtility.SwapElementsByInterval(explicitVertices, 3);
            int implicitCount = TetrahedralizerUtility.CountFlatIListElements(input.m_implicitVertices);
            int[] implicitVertices = null == input.m_implicitVertices ? null : input.m_implicitVertices.ToArray();
            int polyhedronsCount = TetrahedralizerUtility.CountFlatIListElements(input.m_polyhedrons);
            int facetsCount = TetrahedralizerUtility.CountFlatIListElements(input.m_facets);
            int[] constraints = input.m_constraints.ToArray();
            TetrahedralizerUtility.SwapElementsByInterval(constraints, 3);
    
            IntPtr handle = CreateInteriorCharacterizationHandle();
            AddInteriorCharacterizationInput(handle, 
            input.m_explicitVertices.Count/3, explicitVertices, implicitCount, implicitVertices, 
            polyhedronsCount, input.m_polyhedrons.ToArray(), facetsCount, input.m_facets.ToArray(), input.m_facetsCentroids.ToArray(), input.m_facetsCentroidsWeights.ToArray(),
            input.m_constraints.Count/3, constraints, input.m_polyhedronInMultiplier);
    
            CalculateInteriorCharacterization(handle);
    
            IntPtr ptr = GetInteriorCharacterizationPolyhedronsLabels(handle);
            output.m_polyhedronsLabels = ptr.ReadInt32Repeat(polyhedronsCount);
            ptr = GetInteriorCharacterizationFacetsVerticesMapping(handle);
            output.m_facetsVerticesMapping = ptr.ReadInt32NestedRepeat(input.m_facets.Count - facetsCount);
            ptr = GetInteriorCharacterizationFacetsCentroidsMapping(handle);
            output.m_facetsCentroidsMapping = ptr.ReadInt32Repeat(facetsCount);
            DisposeInteriorCharacterizationHandle(handle);
        }
    }
    
}