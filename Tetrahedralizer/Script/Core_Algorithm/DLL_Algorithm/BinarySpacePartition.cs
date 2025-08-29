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
    }
    public class BinarySpacePartitionOutput
    {
        public List<int> m_insertedVertices; // 5/9 followed by indices of explicit vertices
        public List<int> m_polyhedrons; // # of polyhedron facets, followed by facets indexes.
        public List<int> m_polyhedronsFacets; // # of facets vertices, followed by vertices indexes ordered in cw or ccw.
    }


    public void CalculateBinarySpacePartition(BinarySpacePartitionInput input, BinarySpacePartitionOutput output)
    {
        [DllImport(TetrahedralizerLibraryConstant.TETRAHEDRALIZER_LIBRARY_NAME)]
        static extern IntPtr CreateBinarySpacePartitionHandle();
        [DllImport(TetrahedralizerLibraryConstant.TETRAHEDRALIZER_LIBRARY_NAME)]
        static extern void DisposeBinarySpacePartitionHandle(IntPtr handle);
        [DllImport(TetrahedralizerLibraryConstant.TETRAHEDRALIZER_LIBRARY_NAME)]
        static extern void AddBinarySpacePartitionInput(IntPtr handle, int explicit_count, double[] explicit_values, int tetrahedron_count, int[] tetrahedrons, int constraints_count, int[] constraints);
        [DllImport(TetrahedralizerLibraryConstant.TETRAHEDRALIZER_LIBRARY_NAME)]
        static extern int CalculateBinarySpacePartition(IntPtr handle);
        [DllImport(TetrahedralizerLibraryConstant.TETRAHEDRALIZER_LIBRARY_NAME)]
        static extern int GetOutputInsertedVerticesCount(IntPtr handle);
        [DllImport(TetrahedralizerLibraryConstant.TETRAHEDRALIZER_LIBRARY_NAME)]
        static extern IntPtr GetOutputInsertedVertices(IntPtr handle);
        [DllImport(TetrahedralizerLibraryConstant.TETRAHEDRALIZER_LIBRARY_NAME)]
        static extern int GetOutputPolyhedronsCount(IntPtr handle);
        [DllImport(TetrahedralizerLibraryConstant.TETRAHEDRALIZER_LIBRARY_NAME)]
        static extern IntPtr GetOutputPolyhedrons(IntPtr handle);
        [DllImport(TetrahedralizerLibraryConstant.TETRAHEDRALIZER_LIBRARY_NAME)]
        static extern int GetOutputPolyhedronsFacetsCount(IntPtr handle);
        [DllImport(TetrahedralizerLibraryConstant.TETRAHEDRALIZER_LIBRARY_NAME)]
        static extern IntPtr GetOutputPolyhedronsFacets(IntPtr handle);

        double[] explicitVertices = input.m_explicitVertices.ToArray();
        TetrahedralizerLibraryUtility.SwapElementsByInterval(explicitVertices, 3);
        int[] tetrahedrons = input.m_tetrahedrons.ToArray();
        TetrahedralizerLibraryUtility.SwapElementsByInterval(tetrahedrons, 4);

        IntPtr handle = CreateBinarySpacePartitionHandle();
        AddBinarySpacePartitionInput(handle, input.m_explicitVertices.Count/3, explicitVertices, input.m_tetrahedrons.Count/4, tetrahedrons, input.m_constraints.Count/3, input.m_constraints.ToArray());

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

        DisposeBinarySpacePartitionHandle(handle);
    }
}
