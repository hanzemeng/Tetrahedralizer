using System;
using System.Collections.Generic;
using UnityEngine;

public static class TetrahedralMeshUtility
{
    public static double CalculateTetrahedronVolume(Vector3 p0,Vector3 p1,Vector3 p2,Vector3 p3)
    {
        return CalculateTetrahedronVolume(p0.x,p0.y,p0.z, p1.x,p1.y,p1.z, p2.x,p2.y,p2.z, p3.x,p3.y,p3.z);
    }
    public static double CalculateTetrahedronVolume(Point3D p0,Point3D p1,Point3D p2,Point3D p3)
    {
        return CalculateTetrahedronVolume(p0.x,p0.y,p0.z, p1.x,p1.y,p1.z, p2.x,p2.y,p2.z, p3.x,p3.y,p3.z);
    }
    public static double CalculateTetrahedronVolume(double px, double py, double pz, double qx, double qy, double qz, double rx, double ry, double rz, double sx, double sy, double sz)
    {
        double fadx, fbdx, fcdx, fady, fbdy, fcdy, fadz, fbdz, fcdz;
	    double fbdxcdy, fcdxbdy, fcdxady, fadxcdy, fadxbdy, fbdxady, det;

	    fadx = qx - px; fbdx = rx - px; fcdx = sx - px;
	    fady = qy - py; fbdy = ry - py; fcdy = sy - py;
	    fadz = qz - pz; fbdz = rz - pz; fcdz = sz - pz;

	    fbdxcdy = fbdx * fcdy * fadz; fcdxbdy = fcdx * fbdy * fadz;
	    fcdxady = fcdx * fady * fbdz; fadxcdy = fadx * fcdy * fbdz;
	    fadxbdy = fadx * fbdy * fcdz; fbdxady = fbdx * fady * fcdz;

        det = (fbdxcdy - fcdxbdy) + (fcdxady - fadxcdy) + (fadxbdy - fbdxady);

        return Math.Abs(det) / 6d;
    }


    private static Dictionary<Vector3, int> m_mapping = new Dictionary<Vector3, int>();
    public static void RemoveDuplicateVertices(List<Vector3> vertices, List<int> indexes)
    {
        m_mapping.Clear();

        for(int i=0; i<vertices.Count; i++)
        {
            if(!m_mapping.ContainsKey(vertices[i]))
            {
                m_mapping[vertices[i]] = m_mapping.Count;
            }
        }

        for(int i = 0; i<indexes.Count; i++)
        {
            indexes[i] = m_mapping[vertices[indexes[i]]];
        }
        foreach(var kvp in m_mapping)
        {
            vertices[kvp.Value] = kvp.Key;
        }
        vertices.RemoveRange(m_mapping.Count,vertices.Count-m_mapping.Count);
    }

    private static List<double> m_volumes = new List<double>();
    public static void RemoveDegenerateTetrahedrons(List<Vector3> vertices, List<int> tetrahedrons, double ignoreRatio)
    {
        double averageVolume = 0d;
        m_volumes.Clear();
        for(int i=0; i<tetrahedrons.Count; i+=4)
        {
            Vector3 p0 = vertices[tetrahedrons[i+0]];
            Vector3 p1 = vertices[tetrahedrons[i+1]];
            Vector3 p2 = vertices[tetrahedrons[i+2]];
            Vector3 p3 = vertices[tetrahedrons[i+3]];
            double v = CalculateTetrahedronVolume(p0,p1,p2,p3);
            m_volumes.Add(v);
            averageVolume += v;
        }
        //Debug.Log(averageVolume);
        averageVolume /= (double)m_volumes.Count;

        for(int i=0; i<m_volumes.Count; i++)
        {
            if(m_volumes[i] >= averageVolume*ignoreRatio)
            {
                continue;
            }

            int n = tetrahedrons.Count;
            tetrahedrons[4*i+0] = tetrahedrons[n-4];
            tetrahedrons[4*i+1] = tetrahedrons[n-3];
            tetrahedrons[4*i+2] = tetrahedrons[n-2];
            tetrahedrons[4*i+3] = tetrahedrons[n-1];
            tetrahedrons.RemoveRange(n-4,4);

            n = m_volumes.Count;
            m_volumes[i] = m_volumes[n-1];
            m_volumes.RemoveRange(n-1,1);
            i--;
        }
    }
}
