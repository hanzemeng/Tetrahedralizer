using System;
using System.Linq;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.Rendering;
using Unity.Collections;


namespace Hanzzz.Tetrahedralizer
{
    public class MeshVertexDataMapper : IDisposable
    {
        private VertexAttributeDescriptor[] m_sourceVertexAttributeDescriptors;
    
        private NativeArray<Vector3> m_sourcePositions;
        private NativeArray<Color32> m_sourceColors;
        private NativeArray<Vector4>[] m_sourceUVs;
        private bool m_hasColor;
        private bool[] m_hasUV;
    
        private int m_targetArrayIndex;
        private NativeArray<Vector3> m_targetPositions;
        private NativeArray<Color32> m_targetColors;
        private NativeArray<Vector4>[] m_targetUVs;
        

        public void Dispose()
        {
            m_sourcePositions.Dispose();
            m_sourceColors.Dispose();
            foreach(NativeArray<Vector4> m_sourceUV in m_sourceUVs)
            {
                m_sourceUV.Dispose();
            }

            m_targetPositions.Dispose();
            m_targetColors.Dispose();
            foreach(NativeArray<Vector4> m_targetUV in m_targetUVs)
            {
                m_targetUV.Dispose();
            }
        }
        public MeshVertexDataMapper(int capacity=1000)
        {
            m_sourcePositions = new NativeArray<Vector3>(capacity, Allocator.Persistent);
            m_sourceColors = new NativeArray<Color32>(capacity, Allocator.Persistent);
            m_sourceUVs = Enumerable.Range(0,8).Select(i=>new NativeArray<Vector4>(capacity, Allocator.Persistent)).ToArray();
            m_hasColor = false;
            m_hasUV = new bool[8];
    
            m_targetPositions = new NativeArray<Vector3>(capacity, Allocator.Persistent);
            m_targetColors = new NativeArray<Color32>(capacity, Allocator.Persistent);
            m_targetUVs = Enumerable.Range(0,8).Select(i=>new NativeArray<Vector4>(capacity, Allocator.Persistent)).ToArray();
        }
    
        public void AssignSourceMesh(Mesh sourceMesh)
        {
            m_sourceVertexAttributeDescriptors = sourceMesh.GetVertexAttributes();
            
            Mesh.MeshData meshData = Mesh.AcquireReadOnlyMeshData(sourceMesh)[0];
            m_sourcePositions.Resize(meshData.vertexCount, Allocator.Persistent);
            meshData.GetVertices(m_sourcePositions);
            if(m_hasColor = sourceMesh.HasVertexAttribute(VertexAttribute.Color))
            {
                m_sourceColors.Resize(meshData.vertexCount, Allocator.Persistent);
                meshData.GetColors(m_sourceColors);
            }
            for(int i=0; i<8; i++)
            {
                if(m_hasUV[i] = sourceMesh.HasVertexAttribute(VertexAttribute.TexCoord0+i))
                {
                    m_sourceUVs[i].Resize(meshData.vertexCount, Allocator.Persistent);
                    meshData.GetUVs(i,m_sourceUVs[i]);
                }
            }

            m_targetArrayIndex = 0;
        }
        public void AssignSourceMesh(MeshVertexDataMapper other)
        {
            m_sourceVertexAttributeDescriptors = other.m_sourceVertexAttributeDescriptors;
    
            m_sourcePositions = other.m_sourcePositions;
            if(m_hasColor = other.m_hasColor)
            {
                m_sourceColors = other.m_sourceColors;
            }
            for(int i=0; i<8; i++)
            {
                if(m_hasUV[i] = other.m_hasUV[i])
                {
                    m_sourceUVs[i] = other.m_sourceUVs[i];
                }
            }

            m_targetArrayIndex = 0;
        }
    
        public void AssignSourceTetrahedralMesh(TetrahedralMesh sourceMesh)
        {
            m_sourceVertexAttributeDescriptors = sourceMesh.GetVertexAttributeDescriptors();

            m_sourcePositions.Assign(sourceMesh.vertices, Allocator.Persistent);
            if(m_hasColor = sourceMesh.HasVertexAttribute(VertexAttribute.Color))
            {
                m_sourceColors.Assign(sourceMesh.colors, Allocator.Persistent);
            }
            for(int i=0; i<8; i++)
            {
                if(m_hasUV[i] = sourceMesh.HasVertexAttribute(VertexAttribute.TexCoord0+i))
                {
                    m_sourceUVs[i].Assign(sourceMesh.GetUVs(i), Allocator.Persistent);
                }
            }

            m_targetArrayIndex = 0;
        }
    
        public int GetTargetVertexCount()
        {
            return m_targetArrayIndex;
        }
    
        public void CopyVertexData(int s)
        {
            ResizeTargetIfNeeded();
            m_targetPositions[m_targetArrayIndex] = m_sourcePositions[s];
            if(m_hasColor)
            {
                m_targetColors[m_targetArrayIndex] = m_sourceColors[s];
            }
            for(int i=0; i<8; i++)
            {
                if(m_hasUV[i])
                {
                    m_targetUVs[i][m_targetArrayIndex] = m_sourceUVs[i][s];
                }
            }
            m_targetArrayIndex++;
        }
        public void InterpolateVertexData(int n, Vector3 newPosition, int[] ps, double[] ts)
        {
            ResizeTargetIfNeeded();
            m_targetPositions[m_targetArrayIndex] = newPosition;
            {
                if(m_hasColor)
                {
                    Color32 newColor = new Color32(0,0,0,0);
                    for(int i=0; i<n; i++)
                    {
                        newColor.r += (byte)(ts[i]*m_sourceColors[ps[i]].r);
                        newColor.g += (byte)(ts[i]*m_sourceColors[ps[i]].g);
                        newColor.b += (byte)(ts[i]*m_sourceColors[ps[i]].b);
                        newColor.a += (byte)(ts[i]*m_sourceColors[ps[i]].a);
                    }
                    m_targetColors[m_targetArrayIndex] = newColor;
                }
            }
            {
                for(int i=0; i<8; i++)
                {
                    if(m_hasUV[i])
                    {
                        Vector4 newUV = Vector4.zero;
                        for(int j=0; j<n; j++)
                        {
                            newUV += (float)ts[j]*m_sourceUVs[i][ps[j]];
                        }
                        m_targetUVs[i][m_targetArrayIndex] = newUV;
                    }
                }
            }

            m_targetArrayIndex++;
        }
        public void AddDefaultValues(IReadOnlyList<Vector3> positions)
        {
            for(int i=0; i<positions.Count; i++)
            {
                AddDefaultValue(positions[i]);
            }
        }
        public void AddDefaultValue(Vector3 position)
        {
            ResizeTargetIfNeeded();
            m_targetPositions[m_targetArrayIndex] = position;
            if(m_hasColor)
            {
                m_targetColors[m_targetArrayIndex] = Color.clear;
            }
            for(int i=0; i<8; i++)
            {
                if(m_hasUV[i])
                {
                    m_targetUVs[i][m_targetArrayIndex] = Vector4.zero;
                }
            }

            m_targetArrayIndex++;
        }

        private void ResizeTargetIfNeeded(int resizeFactor=2)
        {
            int arraySize = m_targetPositions.Length;
            if(m_targetArrayIndex < arraySize)
            {
                return;
            }

            m_targetPositions.Resize(arraySize*resizeFactor, Allocator.Persistent, true);
            if(m_hasColor)
            {
                m_targetColors.Resize(arraySize*resizeFactor, Allocator.Persistent, true);
            }
            for(int i=0; i<8; i++)
            {
                if(m_hasUV[i])
                {
                    m_targetUVs[i].Resize(arraySize*resizeFactor, Allocator.Persistent, true);
                }
            }
        }
    
        public Mesh MakeMesh()
        {
            Mesh res = new Mesh();
            res.indexFormat = IndexFormat.UInt32;
            res.SetVertices(m_targetPositions);
            if(m_hasColor)
            {
                res.SetColors(m_targetColors);
            }
            for(int i=0; i<8; i++)
            {
                if(m_hasUV[i])
                {
                    res.SetUVs(i,m_targetUVs[i]);
                }
            }
            res.SetVertexBufferParams(m_targetArrayIndex, m_sourceVertexAttributeDescriptors);
            return res;
        }
        public void MakeTetrahedralMesh(TetrahedralMesh tetrahedralMesh)
        {
            tetrahedralMesh.Clear();
            tetrahedralMesh.vertices.Capacity = m_targetArrayIndex;
            for(int i=0; i<m_targetArrayIndex; i++)
            {
                tetrahedralMesh.vertices.Add(m_targetPositions[i]);
            }
            if(m_hasColor)
            {
                tetrahedralMesh.colors.Capacity = m_targetArrayIndex;
                for(int i=0; i<m_targetArrayIndex; i++)
                {
                    tetrahedralMesh.colors.Add(m_targetColors[i]);
                }
            }
            for(int i=0; i<8; i++)
            {
                if(m_hasUV[i])
                {
                    List<Vector4> uvs = tetrahedralMesh.GetUVs(i);
                    uvs.Capacity = m_targetArrayIndex;
                    for(int j=0; j<m_targetArrayIndex; j++)
                    {
                        uvs.Add(m_targetUVs[i][j]);
                    }
                }
            }
    
            //tetrahedralMesh.SetVertices(m_targetPositions.Take(m_targetArrayIndex).ToList());
            //if(m_hasColor)
            //{
            //    tetrahedralMesh.SetColors(m_targetColors.Take(m_targetArrayIndex).ToList());
            //}
            //for(int i=0; i<8; i++)
            //{
            //    if(m_hasUV[i])
            //    {
            //        tetrahedralMesh.SetUVs(i,m_targetUVs[i].Take(m_targetArrayIndex).ToList());
            //    }
            //}
            tetrahedralMesh.SetVertexAttributeDescriptors(m_sourceVertexAttributeDescriptors);
        }
    
        public void ClearTarget()
        {
            m_targetArrayIndex = 0;
        }
    }
    
}