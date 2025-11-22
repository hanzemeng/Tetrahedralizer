using System;
using System.Linq;
using System.Collections.Generic;
using System.Runtime.InteropServices;


namespace Hanzzz.Tetrahedralizer
{
    public class GenericPointPredicate : IDisposable
    {
        [DllImport(TetrahedralizerConstant.TETRAHEDRALIZER_LIBRARY_NAME)]
        static extern IntPtr CreateGenericPointPredicateHandle();
        [DllImport(TetrahedralizerConstant.TETRAHEDRALIZER_LIBRARY_NAME)]
        static extern void DisposeGenericPointPredicateHandle(IntPtr handle);
        [DllImport(TetrahedralizerConstant.TETRAHEDRALIZER_LIBRARY_NAME)]
        static extern void AddGenericPointPredicateVertices(IntPtr handle, int explicit_count, double[] explicit_values, int implict_count, int[] implicit_values);
        [DllImport(TetrahedralizerConstant.TETRAHEDRALIZER_LIBRARY_NAME)]
        static extern int CalculateOrient3d(IntPtr handle, int p0,int p1,int p2,int p3);
        [DllImport(TetrahedralizerConstant.TETRAHEDRALIZER_LIBRARY_NAME)]
        static extern bool CalculateIsCollinear(IntPtr handle, int p0,int p1,int p2);
    
        private IntPtr m_handle;
    
    
        public GenericPointPredicate(IList<double> explicitVertices, IList<int> implicitVertices)
        {
            m_handle = CreateGenericPointPredicateHandle();
            double[] temp = explicitVertices.ToArray();
            TetrahedralizerUtility.SwapElementsByInterval(temp,3);
            AddGenericPointPredicateVertices(m_handle, explicitVertices.Count/3, temp, TetrahedralizerUtility.CountFlatIListElements(implicitVertices), implicitVertices.ToArray());
        }
        public void Dispose()
        {
            DisposeGenericPointPredicateHandle(m_handle);
        }
    
        // dll side returns the right hand version, Unity side returns the left hand version
        public int Orient3d(int p0, int p1, int p2, int p3)
        {
            return -1*CalculateOrient3d(m_handle,p0,p1,p2,p3);
        }
        public bool IsCollinear(int p0, int p1, int p2)
        {
            return CalculateIsCollinear(m_handle,p0,p1,p2);
        }
    }
    
}