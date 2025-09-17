using UnityEngine;

public static class MeshExtension
{
    public static int GetIndexCount(this Mesh mesh)
    {
        int res = 0;
        for(int i=mesh.subMeshCount-1; i>=0; i--)
        {
            res += (int)mesh.GetIndexCount(i); // now we use uint?
        }
        return res;
    }

    
}

public static class MeshUtility
{
    private static Mesh m_emptyMesh = new Mesh();
    public static Mesh GetEmptyMesh()
    {
        if(null == m_emptyMesh)
        {
            m_emptyMesh = new Mesh();
        }
        return m_emptyMesh;
    }
}
