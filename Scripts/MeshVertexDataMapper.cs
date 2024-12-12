using System.Linq;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.Rendering;

public class MeshVertexDataMapper
{
    private VertexAttributeDescriptor[] m_sourceVertexAttributeDescriptors;

    private List<Vector3> m_sourcePositions;
    private List<Color32> m_sourceColors;
    private List<Vector4>[] m_sourceUVs;
    private bool m_hasColor;
    private bool[] m_hasUV;

    private List<Vector3> m_targetPositions;
    private List<Color32> m_targetColors;
    private List<List<Vector4>> m_targetUVs;
    
    public MeshVertexDataMapper()
    {
        m_sourcePositions = new List<Vector3>();
        m_sourceColors = new List<Color32>();
        m_sourceUVs = new List<Vector4>[8]{new(), new(), new(), new(), new(), new(), new(), new()};
        m_hasColor = false;
        m_hasUV = new bool[8];

        m_targetPositions = new List<Vector3>();
        m_targetColors = new List<Color32>();
        m_targetUVs = Enumerable.Range(0,8).Select(i=>new List<Vector4>()).ToList();
    }

    public void AssignSourceMesh(Mesh sourceMesh)
    {
        m_sourceVertexAttributeDescriptors = sourceMesh.GetVertexAttributes();

        sourceMesh.GetVertices(m_sourcePositions);
        m_targetPositions.Clear();

        if(m_hasColor = sourceMesh.HasVertexAttribute(VertexAttribute.Color))
        {
            sourceMesh.GetColors(m_sourceColors);
            m_targetColors.Clear();
        }

        for(int i=0; i<8; i++)
        {
            if(m_hasUV[i] = sourceMesh.HasVertexAttribute(VertexAttribute.TexCoord0+i))
            {
                sourceMesh.GetUVs(i, m_sourceUVs[i]);
                m_targetUVs[i].Clear();
            }
        }
    }
    public void AssignSourceMesh(MeshVertexDataMapper other)
    {
        m_sourceVertexAttributeDescriptors = other.m_sourceVertexAttributeDescriptors;

        m_sourcePositions = other.m_sourcePositions;
        m_targetPositions.Clear();

        if(m_hasColor = other.m_hasColor)
        {
            m_sourceColors = other.m_sourceColors;
            m_targetColors.Clear();
        }

        for(int i=0; i<8; i++)
        {
            if(m_hasUV[i] = other.m_hasUV[i])
            {
                m_sourceUVs[i] = other.m_sourceUVs[i];
                m_targetUVs[i].Clear();
            }
        }
    }

    public int GetTargetVertexCount()
    {
        return m_targetPositions.Count;
    }

    public int CopyVertexData(int s)
    {
        int res = m_targetPositions.Count;

        m_targetPositions.Add(m_sourcePositions[s]);
        if(m_hasColor)
        {
            m_targetColors.Add(m_sourceColors[s]);
        }
        for(int i=0; i<8; i++)
        {
            if(m_hasUV[i])
            {
                m_targetUVs[i].Add(m_sourceUVs[i][s]);
            }
        }
        return res;
    }

    public int InterpolateVertexData(int n, Vector3 newPosition, int[] ps, double[] ts)
    {
        int res = m_targetPositions.Count;

        m_targetPositions.Add(newPosition);

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
                m_targetColors.Add(newColor);
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
                    m_targetUVs[i].Add(newUV);
                }
            }
        }

        return res;
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
        m_targetPositions.Add(position);
        if(m_hasColor)
        {
            m_targetColors.Add(Color.clear);
        }
        for(int i=0; i<8; i++)
        {
            if(m_hasUV[i])
            {
                m_targetUVs[i].Add(Vector4.zero);
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
        res.SetVertexBufferParams(m_targetPositions.Count, m_sourceVertexAttributeDescriptors);
        return res;
    }
}
