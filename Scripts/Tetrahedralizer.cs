using UnityEngine;
using System;
using System.Linq;
using System.Collections.Generic;
using System.Runtime.InteropServices;


public class Tetrahedralizer
{
    private byte[] buffer;
    private MeshVertexDataMapper meshVertexDataMapper;

    public Tetrahedralizer()
    {
        buffer = new byte[8];
        meshVertexDataMapper = new MeshVertexDataMapper();
    }

    public void MeshToTetrahedralizedMesh(Mesh mesh, TetrahedralizedMesh tetrahedralizedMesh)
    {
        if(!BitConverter.IsLittleEndian)
        {
            throw new Exception("CPU is not in little endian.");
        }

        List<byte> input = new List<byte>();

        void WriteDouble(double v)
        {
            UInt64 iv = (UInt64)BitConverter.DoubleToInt64Bits(v);

            for(int i=0; i<8; i++)
            {
                byte b = (byte)(iv & 0xff);
                buffer[i] = b;
                iv >>= 8;
            }

            for(int i=0; i<8; i++)
            {
                input.Add(buffer[i]);
            }
        }
        void WriteInt32(int v)
        {
            UInt32 iv = (UInt32)v;

            for(int i=0; i<4; i++)
            {
                byte b = (byte)(iv & 0xff);
                buffer[i] = b;
                iv >>= 8;
            }

            for(int i=0; i<4; i++)
            {
                input.Add(buffer[i]);
            }
        }

        tetrahedralizedMesh.originalMesh = mesh;
        tetrahedralizedMesh.originalVerticesMappings.Clear();
        tetrahedralizedMesh.newVerticesMappings.Clear();
        tetrahedralizedMesh.tetrahedrons.Clear();


        List<Vector3> vertices = mesh.vertices.ToList();
        List<int> triangles = mesh.triangles.ToList();
        Dictionary<Vector3, UInt32> mapping = new Dictionary<Vector3, UInt32>();

        for(UInt32 i=0; i<vertices.Count; i++)
        {
            UInt32 ni;
            if(!mapping.ContainsKey(vertices[(int)i]))
            {
                mapping[vertices[(int)i]] = (UInt32)mapping.Count;
                tetrahedralizedMesh.originalVerticesMappings.Add(new ListUInt32());
            }
            ni = mapping[vertices[(int)i]];
            tetrahedralizedMesh.originalVerticesMappings[(int)ni].Add(i);
        }

        for(UInt32 i = 0; i<triangles.Count; i++)
        {
            triangles[(int)i] = (int)mapping[vertices[triangles[(int)i]]];
        }
        foreach(var kvp in mapping)
        {
            vertices[(int)kvp.Value] = kvp.Key;
        }
        vertices.RemoveRange(mapping.Count,vertices.Count-mapping.Count);

        WriteInt32(mapping.Count);
        WriteInt32(triangles.Count/3);

        for(UInt32 i = 0; i<mapping.Count; i++)
        {
            WriteDouble(vertices[(int)i].x);
            WriteDouble(vertices[(int)i].z); // convert from left hand to right hand coordinate system
            WriteDouble(vertices[(int)i].y); // convert from left hand to right hand coordinate system
        }
        for(UInt32 i = 0; i<triangles.Count; i+=3)
        {
            if(triangles[(int)i+0] == triangles[(int)i+2] || triangles[(int)i+0] == triangles[(int)i+1] || triangles[(int)i+2] == triangles[(int)i+1])
            {
                throw new Exception("Triangle contains duplicate vertices.");
            }
            WriteInt32(triangles[(int)i+0]);
            WriteInt32(triangles[(int)i+2]); // convert from left hand to right hand coordinate system
            WriteInt32(triangles[(int)i+1]); // convert from left hand to right hand coordinate system
        }


        [DllImport ("libTetrahedralizerLib")]
        static extern IntPtr tetrahedralize(byte[] dll_input);
        IntPtr ptr = tetrahedralize(input.ToArray());


        //double ReadDouble()
        //{
        //    for(int i = 0; i<8; i++)
        //    {
        //        buffer[i] = Marshal.ReadByte(ptr);
        //        ptr = IntPtr.Add(ptr,1);
        //    }
        //    return BitConverter.ToDouble(buffer,0);
        //}
        UInt32 ReadUInt32()
        {
            for(int i = 0; i<4; i++)
            {
                buffer[i] = Marshal.ReadByte(ptr);
                ptr = IntPtr.Add(ptr,1);
            }
            return BitConverter.ToUInt32(buffer,0);
        }


        UInt32 newVerticesCount = ReadUInt32();
        for(UInt32 i = 0; i<newVerticesCount; i++)
        {
            NineUInt32 nineUInt32;
            for(UInt32 j=0; j<9; j++)
            {
                nineUInt32[j] = ReadUInt32();
            }
            tetrahedralizedMesh.newVerticesMappings.Add(nineUInt32);
        }

        UInt32 tetrahedronsCount = ReadUInt32();
        for(UInt32 i = 0; i<tetrahedronsCount; i++)
        {
            UInt32 p0 = ReadUInt32();
            UInt32 p1 = ReadUInt32();
            UInt32 p2 = ReadUInt32();
            UInt32 p3 = ReadUInt32(); // in right hand coordinate system, p3 is above the plane formed by p0,p1,p2

            tetrahedralizedMesh.tetrahedrons.Add(p0);
            tetrahedralizedMesh.tetrahedrons.Add(p1);
            tetrahedralizedMesh.tetrahedrons.Add(p3); // convert from right hand to left hand coordinate system
            tetrahedralizedMesh.tetrahedrons.Add(p2);
        }
    }

    public void TetrahedralizedMeshToTetrahedralMesh(TetrahedralizedMesh tetrahedralizedMesh, TetrahedralMesh tetrahedralMesh, bool remapVertexData)
    {
        if(!BitConverter.IsLittleEndian)
        {
            throw new Exception("CPU is not in little endian.");
        }

        Vector3[] originalVertices = tetrahedralizedMesh.originalMesh.vertices;

        List<Vector3> vertices = new List<Vector3>();
        for(UInt32 i=0; i<tetrahedralizedMesh.originalVerticesMappings.Count; i++)
        {
            UInt32 oi = tetrahedralizedMesh.originalVerticesMappings[(int)i][0];
            vertices.Add(originalVertices[(int)oi]);
        }

        UInt32 UNDEFINED_VALUE = 0xffffffff;
        double[] input_positions = new double[27];
        IntPtr ptr;
        byte[] bs = new byte[8];


        [DllImport ("libTetrahedralizerLib")]
        static extern IntPtr approximate_position(UInt32 n, double[] dll_input);

        double ReadDouble()
        {
            for(int i = 0; i<8; i++)
            {
                bs[i] = Marshal.ReadByte(ptr);
                ptr = IntPtr.Add(ptr,1);
            }
            return BitConverter.ToDouble(bs,0);
        }
        

        for(UInt32 i=0; i<tetrahedralizedMesh.newVerticesMappings.Count; i++)
        {
            UInt32 n = (UNDEFINED_VALUE == tetrahedralizedMesh.newVerticesMappings[(int)i][5]) ? (UInt32)5:(UInt32)9;
            for(UInt32 j=0; j<n; j++)
            {
                UInt32 k = tetrahedralizedMesh.newVerticesMappings[(int)i][j];
                input_positions[3*j+0] = vertices[(int)k].x;
                input_positions[3*j+1] = vertices[(int)k].z; // convert from left hand to right hand coordinate system
                input_positions[3*j+2] = vertices[(int)k].y; // convert from left hand to right hand coordinate system
            }

            ptr = approximate_position(n, input_positions);

            double x = ReadDouble();
            double z = ReadDouble(); // convert from right hand to left hand coordinate system
            double y = ReadDouble(); // convert from right hand to left hand coordinate system

            vertices.Add(new Vector3((float)x,(float)y,(float)z));
        }

        if(!remapVertexData)
        {
            tetrahedralMesh.vertices = vertices;
            tetrahedralMesh.tetrahedrons = tetrahedralizedMesh.tetrahedrons.Select(i=>i).ToList();
            tetrahedralMesh.mesh = null;
            return;
        }


        Point3D CalculateNormal(int p0, int p1, int p2, IReadOnlyList<Vector3> positions)
        {
            Point3D pp0 = new Point3D(positions[p0]);
            Point3D pp1 = new Point3D(positions[p1]);
            Point3D pp2 = new Point3D(positions[p2]);

            Point3D pp01 = pp1-pp0;
            pp01.Normalize();
            Point3D pp12 = pp2-pp1;
            pp12.Normalize();
            Point3D res = Point3D.Cross(pp01, pp12);
            //if(Double.IsNaN(res.x))
            //{
            //    throw new Exception();
            //}
            return res;
        }

        Point3D[] originalVerticesNormal = new Point3D[originalVertices.Length];
        {
            int[] originalTriangles = tetrahedralizedMesh.originalMesh.triangles;
            for(int i=0; i<originalTriangles.Length; i+=3)
            {
                Point3D pn = CalculateNormal(originalTriangles[i+0],originalTriangles[i+1],originalTriangles[i+2], originalVertices);
                originalVerticesNormal[originalTriangles[i+0]] = pn;
                originalVerticesNormal[originalTriangles[i+1]] = pn;
                originalVerticesNormal[originalTriangles[i+2]] = pn;
            }
        }

        meshVertexDataMapper.AssignSourceMesh(tetrahedralizedMesh.originalMesh);
        int uniqueOriginalVerticesCount = tetrahedralizedMesh.originalVerticesMappings.Count;

        (double, int)[] vertexMatchData = new (double, int)[9];
        int[] ps = new int[3];
        double[] ts = new double[3];
        void ProcessFacet(int f0, int f1, int f2)
        {
            Point3D pn = CalculateNormal(f0,f1,f2, vertices);

            void ProcessVertex(int v)
            {
                if(v < uniqueOriginalVerticesCount)
                {
                    double d = -10;
                    int cv = -1;
                    foreach(UInt32 ov in tetrahedralizedMesh.originalVerticesMappings[v].list)
                    {
                        double newD = Point3D.Dot(pn, originalVerticesNormal[(int)ov]);

                        if(newD > d)
                        {
                            d = newD;
                            cv = (int)ov;
                        }
                    }

                    if(cv == -1)
                    {
                        cv = (int)tetrahedralizedMesh.originalVerticesMappings[v].list[0];
                    }
                    meshVertexDataMapper.CopyVertexData(cv);
                }
                else
                {
                    NineUInt32 vs = tetrahedralizedMesh.newVerticesMappings[v-uniqueOriginalVerticesCount];
                    int n = UNDEFINED_VALUE==vs[5] ? 5:9;

                    for(int i=0; i<n; i++)
                    {
                        vertexMatchData[i] = (-10,-1);
                        foreach(UInt32 ov in tetrahedralizedMesh.originalVerticesMappings[(int)vs[(UInt32)i]].list)
                        {
                            double newD = Point3D.Dot(pn, originalVerticesNormal[(int)ov]);

                            if(newD > vertexMatchData[i].Item1)
                            {
                                vertexMatchData[i] = (newD, (int)ov);
                            }
                        }

                        if(vertexMatchData[i].Item2 == -1)
                        {
                            vertexMatchData[i] = (-10,(int)tetrahedralizedMesh.originalVerticesMappings[(int)vs[(UInt32)i]].list[0]);
                        }
                    }

                    void BarycentricWeight(int t0, int t1, int t2, int p0)
                    {
                        double triArea = Vector3.Cross(originalVertices[t1] - originalVertices[t0], originalVertices[t2] - originalVertices[t0]).magnitude;
                        Vector3 ep0 = originalVertices[t0] - vertices[p0];
                        Vector3 ep1 = originalVertices[t1] - vertices[p0];
                        Vector3 ep2 = originalVertices[t2] - vertices[p0];
                        ps[0] = t0;
                        ps[1] = t1;
                        ps[2] = t2;
                        ts[0] = Vector3.Cross(ep1,ep2).magnitude / triArea;
                        ts[1] = Vector3.Cross(ep2,ep0).magnitude / triArea;
                        ts[2] = Vector3.Cross(ep0,ep1).magnitude / triArea;
                        meshVertexDataMapper.InterpolateVertexData(3,vertices[v],ps,ts);
                    }

                    if(5 == n)
                    {
                        double d0 = (vertexMatchData[0].Item1 + vertexMatchData[1].Item1) / 2d;
                        double d1 = (vertexMatchData[2].Item1 + vertexMatchData[3].Item1 + vertexMatchData[4].Item1) / 3d;

                        if(d0 > d1)
                        {
                            double edgeMag = (originalVertices[vertexMatchData[0].Item2] - originalVertices[vertexMatchData[1].Item2]).magnitude;
                            double vertexMag = (vertices[v] - originalVertices[vertexMatchData[0].Item2]).magnitude;
                            double t = vertexMag / edgeMag;

                            ps[0] = vertexMatchData[0].Item2;
                            ps[1] = vertexMatchData[1].Item2;
                            ts[0] = t;
                            ts[1] = 1d-t;
                            meshVertexDataMapper.InterpolateVertexData(2,vertices[v],ps,ts);
                        }
                        else
                        {
                            BarycentricWeight(vertexMatchData[2].Item2, vertexMatchData[3].Item2, vertexMatchData[4].Item2, v);
                        }
                    }
                    else
                    {
                        double d0 = (vertexMatchData[0].Item1 + vertexMatchData[1].Item1 + vertexMatchData[2].Item1) / 3d;
                        double d1 = (vertexMatchData[3].Item1 + vertexMatchData[4].Item1 + vertexMatchData[5].Item1) / 3d;
                        double d2 = (vertexMatchData[6].Item1 + vertexMatchData[7].Item1 + vertexMatchData[8].Item1) / 3d;
                        if(d0 >= d1 && d0 >= d2)
                        {
                            BarycentricWeight(vertexMatchData[0].Item2, vertexMatchData[1].Item2, vertexMatchData[2].Item2, v);
                        }
                        else if(d1 >= d0 && d1 >= d2)
                        {
                            BarycentricWeight(vertexMatchData[3].Item2, vertexMatchData[4].Item2, vertexMatchData[5].Item2, v);
                        }
                        else
                        {
                            BarycentricWeight(vertexMatchData[6].Item2, vertexMatchData[7].Item2, vertexMatchData[8].Item2, v);
                        }
                    }
                }
            }

            ProcessVertex(f0);
            ProcessVertex(f1);
            ProcessVertex(f2);
        }

        for(int i=0; i<tetrahedralizedMesh.tetrahedrons.Count; i+=4)
        {
            int p0 = (int)tetrahedralizedMesh.tetrahedrons[(int)i+0];
            int p1 = (int)tetrahedralizedMesh.tetrahedrons[(int)i+1];
            int p2 = (int)tetrahedralizedMesh.tetrahedrons[(int)i+2];
            int p3 = (int)tetrahedralizedMesh.tetrahedrons[(int)i+3];
            ProcessFacet(p0,p1,p3);
            ProcessFacet(p1,p2,p3);
            ProcessFacet(p2,p0,p3);
            ProcessFacet(p0,p2,p1);
        }

        Mesh mesh = meshVertexDataMapper.MakeMesh();
        int[] temp = Enumerable.Range(0,3*tetrahedralizedMesh.tetrahedrons.Count).Select(i=>i).ToArray();
        mesh.triangles = Enumerable.Range(0,3*tetrahedralizedMesh.tetrahedrons.Count).Select(i=>i).ToArray();
        mesh.RecalculateBounds();
        mesh.RecalculateNormals();
        mesh.RecalculateTangents();
        tetrahedralMesh.mesh = mesh;

    }

}
