namespace Hanzzz.Tetrahedralizer
{

using UnityEngine;
using System;
using System.Linq;
using System.Collections.Generic;
using System.Runtime.InteropServices;


public class Tetrahedralizer
{
    #if UNITY_EDITOR_OSX || UNITY_STANDALONE_OSX
    private const string TETRAHEDRALIZER_LIBRARY_NAME = "TetrahedralizerLibMacOS";
    #elif UNITY_EDITOR_WIN || UNITY_STANDALONE_WIN
    private const string TETRAHEDRALIZER_LIBRARY_NAME = "TetrahedralizerLibWindows";
    #else
    // please compile the library yourself
    #endif

    [DllImport(TETRAHEDRALIZER_LIBRARY_NAME)]
    static extern IntPtr tetrahedralize(byte[] dll_input);
    [DllImport(TETRAHEDRALIZER_LIBRARY_NAME)]
    static extern void load_tetrahedralized_mesh(byte[] dll_input);
    [DllImport(TETRAHEDRALIZER_LIBRARY_NAME)]
    static extern void unload_tetrahedralized_mesh();
    [DllImport(TETRAHEDRALIZER_LIBRARY_NAME)]
    static extern IntPtr approximate_positions();
    [DllImport(TETRAHEDRALIZER_LIBRARY_NAME)]
    static extern IntPtr associate_facets();

    private byte[] m_buffer;
    private List<byte> m_input;
    private double ReadDouble(ref IntPtr ptr)
    {
        for(int i = 0; i<8; i++)
        {
            m_buffer[i] = Marshal.ReadByte(ptr);
            ptr = IntPtr.Add(ptr,1);
        }
        return BitConverter.ToDouble(m_buffer,0);
    }
    private Int32 ReadInt32(ref IntPtr ptr)
    {
        for(int i = 0; i<4; i++)
        {
            m_buffer[i] = Marshal.ReadByte(ptr);
            ptr = IntPtr.Add(ptr,1);
        }
        return BitConverter.ToInt32(m_buffer,0);
    }
    private void WriteDouble(double v)
    {
        UInt64 iv = (UInt64)BitConverter.DoubleToInt64Bits(v);

        for(int i=0; i<8; i++)
        {
            byte b = (byte)(iv & 0xff);
            m_buffer[i] = b;
            iv >>= 8;
        }

        for(int i=0; i<8; i++)
        {
            m_input.Add(m_buffer[i]);
        }
    }
    private void WriteInt32(int v)
    {
        UInt32 iv = (UInt32)v;

        for(int i=0; i<4; i++)
        {
            byte b = (byte)(iv & 0xff);
            m_buffer[i] = b;
            iv >>= 8;
        }

        for(int i=0; i<4; i++)
        {
            m_input.Add(m_buffer[i]);
        }
    }


    public struct Settings
    {
        public const bool DEFAULT_remapVertexData = false;
        public const double DEFAULT_degenerateTetrahedronRatio = 0.0d;


        public bool remapVertexData;
        public double degenerateTetrahedronRatio;

        public Settings(bool remapVertexData=DEFAULT_remapVertexData, double degenerateTetrahedronRatio=DEFAULT_degenerateTetrahedronRatio)
        {
            this.remapVertexData = remapVertexData;
            this.degenerateTetrahedronRatio = degenerateTetrahedronRatio;
        }
    }

    private Settings m_settings;

    private MeshVertexDataMapper m_meshVertexDataMapper;
    private List<Vector3> m_originalVertices;
    private List<Int32> m_originalTriangles;
    private List<Int32> m_originalTrianglesSubmeshes;
    private List<Int32> m_resultTrianglesSubmeshes;
    private List<Int32> m_list_i_0;

    public Tetrahedralizer()
    {
        m_buffer = new byte[8];
        m_input = new List<byte>();

        m_settings = new Settings();

        m_meshVertexDataMapper = new MeshVertexDataMapper();
        m_originalVertices = new List<Vector3>();
        m_originalTriangles = new List<Int32>();
        m_originalTrianglesSubmeshes = new List<Int32>();
        m_resultTrianglesSubmeshes = new List<Int32>();
        m_list_i_0 = new List<Int32>();
    }
    public void SetSettings(Settings settings)
    {
        m_settings = settings;
    }

    public void MeshToTetrahedralizedMesh(Mesh mesh, TetrahedralizedMesh tetrahedralizedMesh)
    {
        if(!BitConverter.IsLittleEndian)
        {
            throw new Exception("CPU is not in little endian.");
        }

        m_input.Clear();

        tetrahedralizedMesh.originalMesh = mesh;
        tetrahedralizedMesh.originalVerticesMappings.Clear();
        tetrahedralizedMesh.newVerticesMappings.Clear();
        tetrahedralizedMesh.tetrahedrons.Clear();

        mesh.GetVertices(m_originalVertices);
        m_originalTriangles = mesh.triangles.ToList();
        Dictionary<Vector3, Int32> mapping = new Dictionary<Vector3, Int32>();

        for(Int32 i=0; i<m_originalVertices.Count; i++)
        {
            Int32 ni;
            if(!mapping.ContainsKey(m_originalVertices[i]))
            {
                mapping[m_originalVertices[i]] = (Int32)mapping.Count;
                tetrahedralizedMesh.originalVerticesMappings.Add(new ListInt32());
            }
            ni = mapping[m_originalVertices[i]];
            tetrahedralizedMesh.originalVerticesMappings[ni].Add(i);
        }

        for(Int32 i = 0; i<m_originalTriangles.Count; i++)
        {
            m_originalTriangles[i] = mapping[m_originalVertices[m_originalTriangles[i]]];
        }
        foreach(var kvp in mapping)
        {
            m_originalVertices[kvp.Value] = kvp.Key;
        }
        m_originalVertices.RemoveRange(mapping.Count,m_originalVertices.Count-mapping.Count);

        WriteInt32(mapping.Count);
        WriteInt32(m_originalTriangles.Count/3);

        for(Int32 i = 0; i<mapping.Count; i++)
        {
            WriteDouble(m_originalVertices[i].x);
            WriteDouble(m_originalVertices[i].z); // convert from left hand to right hand coordinate system
            WriteDouble(m_originalVertices[i].y); // convert from left hand to right hand coordinate system
        }
        for(Int32 i = 0; i<m_originalTriangles.Count; i+=3)
        {
            //if(triangles[i+0] == triangles[i+2] || triangles[i+0] == triangles[i+1] || triangles[i+2] == triangles[i+1])
            //{
            //    throw new Exception("Triangle contains duplicate vertices.");
            //}
            WriteInt32(m_originalTriangles[i+0]);
            WriteInt32(m_originalTriangles[i+2]); // convert from left hand to right hand coordinate system
            WriteInt32(m_originalTriangles[i+1]); // convert from left hand to right hand coordinate system
        }


        IntPtr ptr = tetrahedralize(m_input.ToArray());

        Int32 newVerticesCount = ReadInt32(ref ptr);
        for(Int32 i = 0; i<newVerticesCount; i++)
        {
            NineInt32 nineInt32 = new NineInt32();
            for(Int32 j=0; j<9; j++)
            {
                nineInt32[j] = ReadInt32(ref ptr);
            }
            tetrahedralizedMesh.newVerticesMappings.Add(nineInt32);
        }

        Int32 tetrahedronsCount = ReadInt32(ref ptr);
        for(Int32 i = 0; i<tetrahedronsCount; i++)
        {
            Int32 p0 = ReadInt32(ref ptr);
            Int32 p1 = ReadInt32(ref ptr);
            Int32 p2 = ReadInt32(ref ptr);
            Int32 p3 = ReadInt32(ref ptr); // in right hand coordinate system, p3 is above the plane formed by p0,p1,p2

            tetrahedralizedMesh.tetrahedrons.Add(p0);
            tetrahedralizedMesh.tetrahedrons.Add(p1);
            tetrahedralizedMesh.tetrahedrons.Add(p3); // convert from right hand to left hand coordinate system
            tetrahedralizedMesh.tetrahedrons.Add(p2);
        }
    }

    public void TetrahedralizedMeshToTetrahedralMesh(TetrahedralizedMesh tetrahedralizedMesh, TetrahedralMesh tetrahedralMesh)
    {
        if(!BitConverter.IsLittleEndian)
        {
            throw new Exception("CPU is not in little endian.");
        }

        List<Vector3> vertices = new List<Vector3>();
        List<int> tetrahedrons = tetrahedralizedMesh.tetrahedrons.Select(i=>i).ToList();

        tetrahedralizedMesh.originalMesh.GetVertices(m_originalVertices);

        m_originalTriangles.Clear();
        m_originalTrianglesSubmeshes.Clear();
        Int32 originalSubmeshesCount = tetrahedralizedMesh.originalMesh.subMeshCount;
        for(Int32 i=0; i<originalSubmeshesCount; i++)
        {
            tetrahedralizedMesh.originalMesh.GetTriangles(m_list_i_0,i);
            for(Int32 j=0; j<m_list_i_0.Count/3; j++)
            {
                m_originalTrianglesSubmeshes.Add(i);
            }
            m_originalTriangles.AddRange(m_list_i_0);
        }

        Int32[] originalTrianglesMappings = new Int32[m_originalTriangles.Count];
        for(Int32 i=0; i<tetrahedralizedMesh.originalVerticesMappings.Count; i++)
        {
            foreach(Int32 v in tetrahedralizedMesh.originalVerticesMappings[i].list)
            {
                originalTrianglesMappings[v] = i;
            }
        }
        m_input.Clear();
        WriteInt32(tetrahedralizedMesh.originalVerticesMappings.Count);
        WriteInt32(tetrahedralizedMesh.newVerticesMappings.Count);
        WriteInt32(tetrahedralizedMesh.tetrahedrons.Count/4);
        WriteInt32(m_originalTriangles.Count/3);

        for(Int32 i=0; i<tetrahedralizedMesh.originalVerticesMappings.Count; i++)
        {
            Vector3 p = m_originalVertices[tetrahedralizedMesh.originalVerticesMappings[i][0]];
            WriteDouble(p.x);
            WriteDouble(p.z);  // convert from left hand to right hand coordinate system
            WriteDouble(p.y);  // convert from left hand to right hand coordinate system
        }
        for(Int32 i=0; i<tetrahedralizedMesh.newVerticesMappings.Count; i++)
        {
            for(Int32 j=0; j<9; j++)
            {
                WriteInt32(tetrahedralizedMesh.newVerticesMappings[i][j]);
            }
        }
        for(Int32 i=0; i<tetrahedralizedMesh.tetrahedrons.Count; i+=4)
        {
            WriteInt32(tetrahedralizedMesh.tetrahedrons[i+0]);
            WriteInt32(tetrahedralizedMesh.tetrahedrons[i+1]);
            WriteInt32(tetrahedralizedMesh.tetrahedrons[i+3]); // convert from left hand to right hand coordinate system
            WriteInt32(tetrahedralizedMesh.tetrahedrons[i+2]); // convert from left hand to right hand coordinate system
        }
        for(Int32 i=0; i<m_originalTriangles.Count; i+=3)
        {
            WriteInt32(originalTrianglesMappings[m_originalTriangles[i+0]]);
            WriteInt32(originalTrianglesMappings[m_originalTriangles[i+2]]); // convert from left hand to right hand coordinate system
            WriteInt32(originalTrianglesMappings[m_originalTriangles[i+1]]); // convert from left hand to right hand coordinate system
        }

        load_tetrahedralized_mesh(m_input.ToArray());
        IntPtr ptr = approximate_positions();

        for(Int32 i=0; i<tetrahedralizedMesh.originalVerticesMappings.Count+tetrahedralizedMesh.newVerticesMappings.Count; i++)
        {
            double x = ReadDouble(ref ptr);
            double z = ReadDouble(ref ptr); // convert from right hand to left hand coordinate system
            double y = ReadDouble(ref ptr); // convert from right hand to left hand coordinate system
            vertices.Add(new Vector3((float)x,(float)y,(float)z));
        }

        if(!m_settings.remapVertexData)
        {
            unload_tetrahedralized_mesh();
            TetrahedralMeshUtility.RemoveDuplicateVertices(vertices, tetrahedrons);
            TetrahedralMeshUtility.RemoveDegenerateTetrahedrons(vertices, tetrahedrons, m_settings.degenerateTetrahedronRatio);

            tetrahedralMesh.Clear();
            tetrahedralMesh.SetVertices(vertices);
            tetrahedralMesh.tetrahedrons = tetrahedrons;
            return;
        }
        
        ptr = associate_facets();
        Int32 UNDEFINED_VALUE = -1;

        m_meshVertexDataMapper.AssignSourceMesh(tetrahedralizedMesh.originalMesh);
        m_resultTrianglesSubmeshes.Clear();
        m_list_i_0.Clear();
        int[] ps = new int[3];
        double[] ts = new double[3];
        void ProcessFacet(Int32 p0, Int32 p1, Int32 p2)
        {
            Int32 c0 = ReadInt32(ref ptr);
            Int32 c1 = ReadInt32(ref ptr);
            Int32 c2 = ReadInt32(ref ptr);

            if(UNDEFINED_VALUE == c0 || UNDEFINED_VALUE == c1 || UNDEFINED_VALUE == c2 ||
               m_originalTrianglesSubmeshes[c0] != m_originalTrianglesSubmeshes[c1] ||
               m_originalTrianglesSubmeshes[c1] != m_originalTrianglesSubmeshes[c2])
            {
                m_meshVertexDataMapper.AddDefaultValue(vertices[p0]);
                m_meshVertexDataMapper.AddDefaultValue(vertices[p1]);
                m_meshVertexDataMapper.AddDefaultValue(vertices[p2]);
                m_resultTrianglesSubmeshes.Add(originalSubmeshesCount);
                return;
            }

            void BarycentricWeight(int t0,int t1,int t2,int p)
            {
                double triArea = Vector3.Cross(m_originalVertices[t1] - m_originalVertices[t0], m_originalVertices[t2] - m_originalVertices[t0]).magnitude;
                Vector3 ep0 = m_originalVertices[t0] - vertices[p];
                Vector3 ep1 = m_originalVertices[t1] - vertices[p];
                Vector3 ep2 = m_originalVertices[t2] - vertices[p];
                ps[0] = t0;
                ps[1] = t1;
                ps[2] = t2;
                ts[0] = Vector3.Cross(ep1,ep2).magnitude / triArea;
                ts[1] = Vector3.Cross(ep2,ep0).magnitude / triArea;
                ts[2] = Vector3.Cross(ep0,ep1).magnitude / triArea;
                m_meshVertexDataMapper.InterpolateVertexData(3,vertices[p],ps,ts);
            }

            BarycentricWeight(m_originalTriangles[3*c0+0],m_originalTriangles[3*c0+1],m_originalTriangles[3*c0+2], p0);
            BarycentricWeight(m_originalTriangles[3*c1+0],m_originalTriangles[3*c1+1],m_originalTriangles[3*c1+2], p1);
            BarycentricWeight(m_originalTriangles[3*c2+0],m_originalTriangles[3*c2+1],m_originalTriangles[3*c2+2], p2);
            m_resultTrianglesSubmeshes.Add(m_originalTrianglesSubmeshes[c0]);
        }

        double averageVolume = TetrahedralMeshUtility.CalculateAverageVolume(vertices,tetrahedrons);
        for(Int32 i=0; i<tetrahedralizedMesh.tetrahedrons.Count; i+=4)
        {
            Int32 p0 = tetrahedralizedMesh.tetrahedrons[i+0];
            Int32 p1 = tetrahedralizedMesh.tetrahedrons[i+1];
            Int32 p2 = tetrahedralizedMesh.tetrahedrons[i+2];
            Int32 p3 = tetrahedralizedMesh.tetrahedrons[i+3];

            double volume = TetrahedralMeshUtility.CalculateTetrahedronVolume(vertices[p0],vertices[p1],vertices[p2],vertices[p3]);
            if(volume < averageVolume*m_settings.degenerateTetrahedronRatio)
            {
                for(Int32 j=0; j<12; j++)
                {
                    ReadInt32(ref ptr);
                }
                continue;
            }
            ProcessFacet(p0,p1,p3);
            ProcessFacet(p1,p0,p2);
            ProcessFacet(p0,p3,p2);
            ProcessFacet(p3,p1,p2);
        }

        m_meshVertexDataMapper.MakeTetrahedralMesh(tetrahedralMesh);
        tetrahedralMesh.SetFacetsSubmeshes(m_resultTrianglesSubmeshes);

        unload_tetrahedralized_mesh();
    }
}

}
