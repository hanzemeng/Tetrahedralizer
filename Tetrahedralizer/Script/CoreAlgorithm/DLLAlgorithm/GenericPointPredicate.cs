using System;
using System.Linq;
using System.Collections.Generic;
using System.Runtime.InteropServices;
using UnityEngine;


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
        static extern int CalculateOrient3dExplicit(IntPtr handle, double p0x, double p0y, double p0z, double p1x, double p1y, double p1z, double p2x, double p2y, double p2z, double p3x, double p3y, double p3z);
        [DllImport(TetrahedralizerConstant.TETRAHEDRALIZER_LIBRARY_NAME)]
        static extern bool CalculateIsCollinear(IntPtr handle, int p0,int p1,int p2);
    
        private IntPtr m_handle;
    
    
        public GenericPointPredicate(IReadOnlyList<double> explicitVertices, IReadOnlyList<int> implicitVertices)
        {
            m_handle = CreateGenericPointPredicateHandle();
            if(null != explicitVertices)
            {
                double[] explicitVerticesArray = explicitVertices.ToArray();
                TetrahedralizerUtility.SwapElementsByInterval(explicitVerticesArray, 3); // Change from left to right hand coordinate.
                int[] implicitVerticesArray = null == implicitVertices ? null : implicitVertices.ToArray();
                int implicitVerticesCount = null == implicitVertices ? 0 : TetrahedralizerUtility.CountFlatIListElements(implicitVerticesArray);
                AddGenericPointPredicateVertices(m_handle, explicitVerticesArray.Length/3, explicitVerticesArray, implicitVerticesCount, implicitVerticesArray);
            }
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
        public int Orient3d(Vector3 p0,Vector3 p1,Vector3 p2,Vector3 p3)
        {
            return -1*CalculateOrient3dExplicit(m_handle,
            p0.x,p0.z,p0.y,
            p1.x,p1.z,p1.y,
            p2.x,p2.z,p2.y,
            p3.x,p3.z,p3.y);
        }
        public bool IsCollinear(int p0, int p1, int p2)
        {
            return CalculateIsCollinear(m_handle,p0,p1,p2);
        }
    }
    
}