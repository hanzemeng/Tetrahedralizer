using System;
using System.Linq;
using System.Collections.Generic;
using System.Runtime.InteropServices;

public class BinarySpacePartition
{
    public class BinarySpacePartitionInput
    {
        public IList<double> m_explicitVertices; // Every 3 doubles are x,y,z of a point. Assuming left hand coordinate.
        public IList<int> m_tetrahedrons;
        public IList<int> m_constraints;
        public bool m_aggressivelyAddVirtualConstraints; // If true, every constraint can be represented as a union of polyhedrons facets. Otherwise, every polygon formed by coplanar constraints can be represented as a union of polyhedrons facets.
        public bool m_removeCollinearSegments;
    }
    public class BinarySpacePartitionOutput
    {
        public List<int> m_insertedVertices; // 5/9 followed by indices of explicit vertices
        public List<int> m_polyhedrons; // # of polyhedron facets, followed by facets indexes.
        public List<int> m_polyhedronsFacets; // # of facets vertices, followed by vertices indexes ordered in cw or ccw.
    }


    public void CalculateBinarySpacePartition(BinarySpacePartitionInput input, BinarySpacePartitionOutput output)
    {
        [DllImport(TetrahedralizerConstant.TETRAHEDRALIZER_LIBRARY_NAME)]
        static extern IntPtr CreateBinarySpacePartitionHandle();
        [DllImport(TetrahedralizerConstant.TETRAHEDRALIZER_LIBRARY_NAME)]
        static extern void DisposeBinarySpacePartitionHandle(IntPtr handle);
        [DllImport(TetrahedralizerConstant.TETRAHEDRALIZER_LIBRARY_NAME)]
        static extern void AddBinarySpacePartitionInput(IntPtr handle, int explicit_count, double[] explicit_values, int tetrahedron_count, int[] tetrahedrons, int constraints_count, int[] constraints, bool aggressivelyAddVirtualConstraints);
        [DllImport(TetrahedralizerConstant.TETRAHEDRALIZER_LIBRARY_NAME)]
        static extern int CalculateBinarySpacePartition(IntPtr handle);
        [DllImport(TetrahedralizerConstant.TETRAHEDRALIZER_LIBRARY_NAME)]
        static extern int GetOutputInsertedVerticesCount(IntPtr handle);
        [DllImport(TetrahedralizerConstant.TETRAHEDRALIZER_LIBRARY_NAME)]
        static extern IntPtr GetOutputInsertedVertices(IntPtr handle);
        [DllImport(TetrahedralizerConstant.TETRAHEDRALIZER_LIBRARY_NAME)]
        static extern int GetOutputPolyhedronsCount(IntPtr handle);
        [DllImport(TetrahedralizerConstant.TETRAHEDRALIZER_LIBRARY_NAME)]
        static extern IntPtr GetOutputPolyhedrons(IntPtr handle);
        [DllImport(TetrahedralizerConstant.TETRAHEDRALIZER_LIBRARY_NAME)]
        static extern int GetOutputPolyhedronsFacetsCount(IntPtr handle);
        [DllImport(TetrahedralizerConstant.TETRAHEDRALIZER_LIBRARY_NAME)]
        static extern IntPtr GetOutputPolyhedronsFacets(IntPtr handle);

        double[] explicitVertices = input.m_explicitVertices.ToArray();
        TetrahedralizerUtility.SwapElementsByInterval(explicitVertices, 3);
        int[] tetrahedrons = input.m_tetrahedrons.ToArray();
        TetrahedralizerUtility.SwapElementsByInterval(tetrahedrons, 4);

        IntPtr handle = CreateBinarySpacePartitionHandle();
        AddBinarySpacePartitionInput(handle, input.m_explicitVertices.Count/3, explicitVertices, input.m_tetrahedrons.Count/4, tetrahedrons, input.m_constraints.Count/3, input.m_constraints.ToArray(), input.m_aggressivelyAddVirtualConstraints);

        CalculateBinarySpacePartition(handle);

        void PopulateOutput(int n, IntPtr ptr, ref List<int> res)
        {
            res = new List<int>();
            for(int i=0; i<n; i++)
            {
                int j = ptr.ReadInt32();
                res.Add(j);
                for(;j>0;j--)
                {
                    res.Add(ptr.ReadInt32());
                }
            }
        }

        PopulateOutput(GetOutputInsertedVerticesCount(handle), GetOutputInsertedVertices(handle), ref output.m_insertedVertices);
        PopulateOutput(GetOutputPolyhedronsCount(handle), GetOutputPolyhedrons(handle), ref output.m_polyhedrons);
        PopulateOutput(GetOutputPolyhedronsFacetsCount(handle), GetOutputPolyhedronsFacets(handle), ref output.m_polyhedronsFacets);

        if(input.m_removeCollinearSegments)
        {
            List<List<int>> facets = TetrahedralizerUtility.FlatIListToNestedList(output.m_polyhedronsFacets);
            List<List<int>> newFacets = new List<List<int>>();
            using GenericPointPredicate genericPointPredicate = new GenericPointPredicate(input.m_explicitVertices, output.m_insertedVertices);

            foreach(List<int> facet in facets)
            {
                List<int> newFacet = new List<int>();
                for(int i=0; i<facet.Count; i++)
                {
                    int p = 0==i ? facet.Count-1:i-1;
                    int n = facet.Count-1==i ? 0:i+1;

                    if(!genericPointPredicate.IsCollinear(facet[p],facet[i],facet[n]))
                    {
                        newFacet.Add(facet[i]);
                    }
                }
                newFacets.Add(newFacet);
            }

            output.m_polyhedronsFacets = TetrahedralizerUtility.NestedListToFlatList(newFacets);
        }

        DisposeBinarySpacePartitionHandle(handle);

        // sanity checks: every facet should not be a line, and every polyhedron should not be a plane
        //{
        //    using GenericPointPredicate genericPointPredicate = new GenericPointPredicate(input.m_explicitVertices, output.m_insertedVertices);
        //    List<List<int>> polyhedrons = TetrahedralizerUtility.FlatIListToNestedList(output.m_polyhedrons);
        //    List<List<int>> facets = TetrahedralizerUtility.FlatIListToNestedList(output.m_polyhedronsFacets);

        //    foreach(List<int> facet in facets)
        //    {
        //        int p0 = facet[0];
        //        int p1 = facet[1];
        //        bool facetIsDegenerate = true;
        //        for(int i=2; i<facet.Count; i++)
        //        {
        //            if(!genericPointPredicate.IsCollinear(p0,p1,facet[i]))
        //            {
        //                facetIsDegenerate = false;
        //                break;
        //            }
        //        }
        //        if(facetIsDegenerate)
        //        {
        //            throw new Exception();
        //        }
        //    }

        //    foreach(List<int> polyhedron in polyhedrons)
        //    {
        //        int p0,p1,p2;
        //        p0=p1=p2=-1; // find non collinear points
        //        List<int> facetVertices = facets[polyhedron[0]];
        //        for(int i=0; i<facetVertices.Count; i++)
        //        {
        //            int p = 0==i ? facetVertices.Count-1:i-1;
        //            int n = facetVertices.Count-1==i ? 0:i+1;
        //            if(!genericPointPredicate.IsCollinear(facetVertices[p],facetVertices[i],facetVertices[n]))
        //            {
        //                p0=facetVertices[p];
        //                p1=facetVertices[i];
        //                p2=facetVertices[n];
        //                break;
        //            }
        //        }

        //        bool polyhedronIsDegenerate = true;
        //        for(int i=1; i<polyhedron.Count; i++)
        //        {
        //            facetVertices = facets[polyhedron[i]];
        //            for(int j=0; j<facetVertices.Count; j++)
        //            {
        //                if(0 != genericPointPredicate.Orient3d(p0,p1,p2,facetVertices[j]))
        //                {
        //                    polyhedronIsDegenerate = false;
        //                    break;
        //                }
        //            }
        //        }

        //        if(polyhedronIsDegenerate)
        //        {
        //            throw new Exception();
        //        }
        //    }
        //}
    }
}
