using System;
using System.Linq;
using System.Collections.Generic;
using System.Runtime.InteropServices;

public class InteriorCharacterization
{
    public class InteriorCharacterizationInput
    {
        public IList<double> m_explicitVertices; // Every 3 doubles are x,y,z of a point. Assuming left hand coordinate.
        public IList<int> m_implicitVertices; // 5/9 followed by indexes of m_explicitVertices.
        public IList<int> m_polyhedrons; // # of polyhedron facets, followed by facets indexes.
        public IList<int> m_polyhedronsFacets; // # of facets vertices, followed by vertices indexes ordered in cw or ccw.
        public IList<int> m_constraints; // Every 3 ints is a constraint (triangle). Curl around the points and your thumb points outward. Assuming left hand coordinate.
        public IList<double> m_polyhedronsWindingNumbers; // The winding number of a polyhedron determines if it is inside or outside the constraints.
        public double m_minCutNeighborMultiplier; // Discussed in paper but don't know how to choose a value.
    }
    public class InteriorCharacterizationOutput
    {
        public List<double> m_polyhedronsWindingNumbers; // If winding number is not provided, it will be calculated by the algorithm.
        public List<int> m_polyhedronsInteriorLabels; // 0 if outside, otherwise inside.
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
        int polyhedrons_count, int[] polyhedrons, int polyhedrons_facets_count, int[] polyhedrons_facets, int constraints_count, int[] constraints, 
        double[] polyhedrons_winding_numbers, double min_cut_neighbor_multiplier);
        [DllImport(TetrahedralizerConstant.TETRAHEDRALIZER_LIBRARY_NAME)]
        static extern int CalculateInteriorCharacterization(IntPtr handle);
        [DllImport(TetrahedralizerConstant.TETRAHEDRALIZER_LIBRARY_NAME)]
        static extern IntPtr GetOutputPolyhedronsWindingNumbers(IntPtr handle);
        [DllImport(TetrahedralizerConstant.TETRAHEDRALIZER_LIBRARY_NAME)]
        static extern IntPtr GetOutputPolyhedronsLabels(IntPtr handle);


        double[] explicitVertices = input.m_explicitVertices.ToArray();
        TetrahedralizerUtility.SwapElementsByInterval(explicitVertices, 3);
        int implicitCount = TetrahedralizerUtility.CountFlatIListElements(input.m_implicitVertices);
        int[] implicitVertices = null == input.m_implicitVertices ? null : input.m_implicitVertices.ToArray();
        int polyhedronsCount = TetrahedralizerUtility.CountFlatIListElements(input.m_polyhedrons);
        int[] constraints = input.m_constraints.ToArray();
        TetrahedralizerUtility.SwapElementsByInterval(constraints, 3);
        double[] polyhedronsWindingNumbers = null == input.m_polyhedronsWindingNumbers ? null : input.m_polyhedronsWindingNumbers.ToArray();

        IntPtr handle = CreateInteriorCharacterizationHandle();
        AddInteriorCharacterizationInput(handle, 
        input.m_explicitVertices.Count/3, explicitVertices, implicitCount, implicitVertices, 
        polyhedronsCount, input.m_polyhedrons.ToArray(), TetrahedralizerUtility.CountFlatIListElements(input.m_polyhedronsFacets), input.m_polyhedronsFacets.ToArray(), input.m_constraints.Count/3, constraints, 
        polyhedronsWindingNumbers, input.m_minCutNeighborMultiplier);

        CalculateInteriorCharacterization(handle);


        IntPtr ptr;
        if(null == input.m_polyhedronsWindingNumbers)
        {
            output.m_polyhedronsWindingNumbers = new List<double>();
            ptr = GetOutputPolyhedronsWindingNumbers(handle);
            for(int i=0; i<polyhedronsCount; i++)
            {
                output.m_polyhedronsWindingNumbers.Add(ptr.ReadDouble());
            }
        }

        output.m_polyhedronsInteriorLabels = new List<int>();
        ptr = GetOutputPolyhedronsLabels(handle);
        for(int i=0; i<polyhedronsCount; i++)
        {
            output.m_polyhedronsInteriorLabels.Add(ptr.ReadInt32());
        }

        DisposeInteriorCharacterizationHandle(handle);
    }
}
