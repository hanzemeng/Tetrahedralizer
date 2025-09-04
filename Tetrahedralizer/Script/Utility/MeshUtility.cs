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
